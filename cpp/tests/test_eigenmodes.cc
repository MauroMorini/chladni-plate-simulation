// The SLEPc shift-and-invert eigenfrequencies agree with a dense LAPACK solve of the same
// problem, both for the lowest modes and for modes around a target frequency. Because the
// plate is square, some modes come in pairs with equal frequency (rotated by 90 degrees).

#include "test_utilities.h"

#include <chladni/eigenmode_solver.h>
#include <chladni/plate.h>

#include <deal.II/base/mpi.h>
#include <deal.II/base/numbers.h>
#include <deal.II/lac/lapack_full_matrix.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

using namespace Chladni;

/** All eigenfrequencies of the clamped plate from a dense generalized eigenvalue solve. */
std::vector<double> reference_frequencies(const Plate &plate)
{
    const auto &bolt = plate.get_bolt_dofs();
    const std::set<types::global_dof_index> bolt_set(bolt.begin(), bolt.end());

    std::vector<types::global_dof_index> free_dofs;
    for (types::global_dof_index i = 0; i < plate.get_dof_handler().n_dofs(); ++i)
        if (bolt_set.count(i) == 0)
            free_dofs.push_back(i);

    const unsigned int         n = free_dofs.size();
    LAPACKFullMatrix<double>   stiffness(n, n), mass(n, n);
    for (unsigned int i = 0; i < n; ++i)
        for (unsigned int j = 0; j < n; ++j) {
            stiffness(i, j) = plate.get_stiffness_matrix().el(free_dofs[i], free_dofs[j]);
            mass(i, j)      = plate.get_mass_matrix().el(free_dofs[i], free_dofs[j]);
        }

    Vector<double>              eigenvalues;
    std::vector<Vector<double>> eigenvectors;
    stiffness.compute_generalized_eigenvalues_symmetric(
        mass, 0., std::numeric_limits<double>::max(), 0., eigenvalues, eigenvectors);

    std::vector<double> frequencies;
    for (const double lambda : eigenvalues)
        frequencies.push_back(std::sqrt(lambda) / (2. * numbers::PI));
    std::sort(frequencies.begin(), frequencies.end());
    return frequencies;
}

int main(int argc, char *argv[])
{
    Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);

    const Plate plate(small_plate_parameters());
    const auto  reference = reference_frequencies(plate);
    std::cout << "  " << reference.size() << " reference eigenfrequencies, lowest "
              << reference[0] << " Hz" << std::endl;

    const EigenmodeSolver solver(plate);
    const unsigned int    n_modes = 6;

    // Lowest modes: target frequency 0.
    const auto lowest = solver.solve(0., n_modes);
    for (unsigned int k = 0; k < n_modes; ++k)
        check_relative(lowest[k].frequency, reference[k], 1e-6, "lowest mode " + std::to_string(k));

    // Modes around a target inside the spectrum: they must be the n_modes reference frequencies
    // closest to the target.
    const double target = reference[20];
    std::vector<double> closest = reference;
    std::sort(closest.begin(), closest.end(), [&](const double a, const double b) {
        return std::abs(a * a - target * target) < std::abs(b * b - target * target);
    });
    closest.resize(n_modes);
    std::sort(closest.begin(), closest.end());

    const auto around_target = solver.solve(target, n_modes);
    for (unsigned int k = 0; k < n_modes; ++k)
        check_relative(around_target[k].frequency,
                       closest[k],
                       1e-6,
                       "mode " + std::to_string(k) + " around " + std::to_string(target) + " Hz");

    // Square symmetry: at least one pair of (numerically) equal frequencies among the lowest.
    bool found_pair = false;
    for (unsigned int k = 0; k + 1 < n_modes; ++k)
        if (std::abs(lowest[k + 1].frequency - lowest[k].frequency) < 1e-6 * lowest[k].frequency)
            found_pair = true;
    check(found_pair, "square symmetry gives a pair of equal eigenfrequencies");

    // Modes are scaled to max |u_z| = 1 and vanish on the bolt.
    double max_vertical = 0.;
    for (const auto dof : plate.get_z_dofs())
        max_vertical = std::max(max_vertical, std::abs(lowest[0].displacement[dof]));
    check_relative(max_vertical, 1., 1e-12, "mode 0 is scaled to max |u_z| = 1");

    double max_on_bolt = 0.;
    for (const auto dof : plate.get_bolt_dofs())
        max_on_bolt = std::max(max_on_bolt, std::abs(lowest[0].displacement[dof]));
    check(max_on_bolt < 1e-10, "mode 0 vanishes on the bolt");

    return 0;
}
