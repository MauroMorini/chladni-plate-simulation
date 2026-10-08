// The SLEPc shift-and-invert eigenfrequencies agree with a dense LAPACK solve of the same
// problem, both for the lowest modes and for modes around a target frequency. Because the
// plate is square, some modes come in pairs with equal frequency (rotated by 90 degrees).
// The free plate gives the same frequencies as LAPACK without its 6 rigid body motions.

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

/** All eigenfrequencies of the plate from a dense generalized eigenvalue solve. The bolt is
 * clamped unless the plate's center support is free. */
std::vector<double> reference_frequencies(const Plate &plate)
{
    std::set<types::global_dof_index> bolt_set;
    if (plate.get_parameters().center_support == CenterSupport::clamped)
        bolt_set.insert(plate.get_bolt_dofs().begin(), plate.get_bolt_dofs().end());

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
    // The lower bound is slightly below 0: round-off makes some rigid body motions of the free
    // plate (omega^2 = 0) come out as tiny negative numbers.
    stiffness.compute_generalized_eigenvalues_symmetric(
        mass, -1., std::numeric_limits<double>::max(), 0., eigenvalues, eigenvectors);

    std::vector<double> frequencies;
    for (const double lambda : eigenvalues)
        frequencies.push_back(std::sqrt(std::max(lambda, 0.)) / (2. * numbers::PI));
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

    // Free plate: the 6 lowest reference frequencies are the rigid body motions (f = 0). The
    // solver leaves them out, so its modes start with reference[6].
    Parameters free_parameters     = small_plate_parameters();
    free_parameters.center_support = CenterSupport::free;
    const Plate free_plate(free_parameters);
    const auto  free_reference = reference_frequencies(free_plate);
    check(free_reference[5] < EigenmodeSolver::rigid_body_frequency &&
              free_reference[6] > EigenmodeSolver::rigid_body_frequency,
          "free plate has exactly 6 rigid body motions");

    const EigenmodeSolver free_solver(free_plate);
    const auto            free_lowest = free_solver.solve(0., n_modes);
    check(free_lowest.size() == n_modes, "free: the requested number of modes is returned");
    for (unsigned int k = 0; k < n_modes; ++k)
        check_relative(free_lowest[k].frequency,
                       free_reference[6 + k],
                       1e-6,
                       "free: lowest elastic mode " + std::to_string(k));

    // Around a target far above the rigid body motions.
    const double free_target = free_reference[26];
    std::vector<double> free_closest(free_reference.begin() + 6, free_reference.end());
    std::sort(free_closest.begin(), free_closest.end(), [&](const double a, const double b) {
        return std::abs(a * a - free_target * free_target) <
               std::abs(b * b - free_target * free_target);
    });
    free_closest.resize(n_modes);
    std::sort(free_closest.begin(), free_closest.end());

    const auto free_around_target = free_solver.solve(free_target, n_modes);
    for (unsigned int k = 0; k < n_modes; ++k)
        check_relative(free_around_target[k].frequency,
                       free_closest[k],
                       1e-6,
                       "free: mode " + std::to_string(k) + " around " +
                           std::to_string(free_target) + " Hz");

    return 0;
}
