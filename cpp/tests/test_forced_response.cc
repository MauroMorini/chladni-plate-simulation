// The forced response behaves as the physics says, for both kinds of center support:
// - clamped: for f -> 0 the whole plate follows the bolt rigidly, so G = RMS(u_z) / A = 1,
// - free: for f -> 0 the force just accelerates the whole plate, F = m omega^2 u_z, so
//   G = RMS(u_z) / F = 1 / (m omega^2),
// - approaching an eigenfrequency (of the clamped or of the free plate) the response grows
//   without bound (resonance),
// - the sweep returns the requested frequencies.

#include "test_utilities.h"

#include <chladni/eigenmode_solver.h>
#include <chladni/forced_response_solver.h>
#include <chladni/plate.h>

#include <deal.II/base/mpi.h>
#include <deal.II/base/numbers.h>

using namespace Chladni;

/** A shaker at the center cannot excite every mode (e.g. modes that are antisymmetric in x have
 * no net push from the center), so we look for the first mode that responds and check that the
 * response keeps growing as we approach its frequency. */
void check_resonance(const Plate &plate, const std::string &name)
{
    const ForcedResponseSolver forced_solver(plate);
    const auto                 modes = EigenmodeSolver(plate).solve(0., 6);

    bool found_resonance = false;
    for (const auto &mode : modes) {
        const auto G = [&](const double factor) {
            return forced_solver.response_amplitude(forced_solver.solve(factor * mode.frequency));
        };
        const double G_far = G(0.9), G_near = G(0.99), G_closer = G(0.9999);

        std::cout << "  mode at " << mode.frequency << " Hz: G = " << G_far << ", " << G_near
                  << ", " << G_closer << std::endl;

        if (G_closer > 10. * G_near && G_near > G_far) {
            found_resonance = true;
            break;
        }
    }
    check(found_resonance,
          name + ": response grows without bound near an excited eigenfrequency");
}

int main(int argc, char *argv[])
{
    Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);

    // Clamped center (displacement drive).
    {
        const Parameters           parameters = small_plate_parameters();
        const Plate                plate(parameters);
        const ForcedResponseSolver forced_solver(plate);

        check_relative(forced_solver.response_amplitude(forced_solver.solve(1e-3)),
                       1.,
                       1e-6,
                       "clamped: G(f -> 0) = 1 (rigid motion)");

        check_resonance(plate, "clamped");

        // Sweep bookkeeping.
        const auto curve = forced_solver.sweep(10., 20., 11);
        check(curve.size() == 11, "sweep has the requested number of points");
        check_relative(curve.front().frequency, 10., 1e-14, "sweep starts at f_min");
        check_relative(curve.back().frequency, 20., 1e-14, "sweep ends at f_max");
    }

    // Free center (force drive).
    {
        Parameters parameters     = small_plate_parameters();
        parameters.center_support = CenterSupport::free;
        const Plate                plate(parameters);
        const ForcedResponseSolver forced_solver(plate);

        const double frequency = 1.; // far below the first elastic mode
        const double omega     = 2. * numbers::PI * frequency;
        const double mass      = parameters.density * plate.volume();
        check_relative(forced_solver.response_amplitude(forced_solver.solve(frequency)),
                       1. / (mass * omega * omega),
                       1e-4,
                       "free: G(f -> 0) = 1 / (m omega^2) (rigid acceleration)");

        check_resonance(plate, "free");
    }

    return 0;
}
