// The forced response behaves as the physics says:
// - for f -> 0 the whole plate follows the bolt rigidly, so G = RMS(u_z) / A = 1,
// - approaching an eigenfrequency the response grows without bound (resonance),
// - the sweep returns the requested frequencies.

#include "test_utilities.h"

#include <chladni/eigenmode_solver.h>
#include <chladni/forced_response_solver.h>
#include <chladni/plate.h>

#include <deal.II/base/mpi.h>

using namespace Chladni;

int main(int argc, char *argv[])
{
    Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);

    const Parameters           parameters = small_plate_parameters();
    const Plate                plate(parameters);
    const ForcedResponseSolver forced_solver(plate, parameters.drive_amplitude);

    // Static limit.
    check_relative(forced_solver.response_amplitude(forced_solver.solve(1e-3)),
                   1.,
                   1e-6,
                   "G(f -> 0) = 1 (rigid motion)");

    // Resonance. A shaker at the center cannot excite every mode (e.g. modes that are
    // antisymmetric in x have no net push from the center), so we look for the first mode that
    // responds and check that the response keeps growing as we approach its frequency.
    const auto modes = EigenmodeSolver(plate).solve(0., 6);

    bool found_resonance = false;
    for (const auto &mode : modes) {
        const double G_far    = forced_solver.response_amplitude(forced_solver.solve(0.9 * mode.frequency));
        const double G_near   = forced_solver.response_amplitude(forced_solver.solve(0.99 * mode.frequency));
        const double G_closer = forced_solver.response_amplitude(forced_solver.solve(0.9999 * mode.frequency));

        std::cout << "  mode at " << mode.frequency << " Hz: G = " << G_far << ", " << G_near << ", "
                  << G_closer << std::endl;

        if (G_closer > 100. && G_closer > 10. * G_near && G_near > G_far) {
            found_resonance = true;
            break;
        }
    }
    check(found_resonance, "response grows without bound near an excited eigenfrequency");

    // Sweep bookkeeping.
    const auto curve = forced_solver.sweep(10., 20., 11);
    check(curve.size() == 11, "sweep has the requested number of points");
    check_relative(curve.front().frequency, 10., 1e-14, "sweep starts at f_min");
    check_relative(curve.back().frequency, 20., 1e-14, "sweep ends at f_max");

    return 0;
}
