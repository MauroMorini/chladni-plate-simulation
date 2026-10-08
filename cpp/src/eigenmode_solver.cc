#include <chladni/eigenmode_solver.h>

#include <deal.II/base/index_set.h>
#include <deal.II/base/numbers.h>
#include <deal.II/lac/petsc_solver.h>
#include <deal.II/lac/petsc_sparse_matrix.h>
#include <deal.II/lac/petsc_vector.h>
#include <deal.II/lac/slepc_solver.h>
#include <deal.II/lac/slepc_spectral_transformation.h>
#include <deal.II/numerics/matrix_tools.h>

#include <algorithm>
#include <cmath>
#include <map>

namespace Chladni
{
    namespace
    {
        /** Copy a deal.II sparse matrix into a (serial) PETSc matrix, as needed by SLEPc. */
        void copy_to_petsc(const SparseMatrix<double> &matrix, PETScWrappers::SparseMatrix &result)
        {
            result.reinit(matrix.get_sparsity_pattern());
            for (const auto &entry : matrix)
                result.set(entry.row(), entry.column(), entry.value());
            result.compress(VectorOperation::insert);
        }
    } // namespace

    EigenmodeSolver::EigenmodeSolver(const Plate &plate)
        : plate(plate)
    {}

    std::vector<Eigenmode> EigenmodeSolver::solve(const double       target_frequency,
                                                  const unsigned int n_modes) const
    {
        const unsigned int n_dofs  = plate.get_dof_handler().n_dofs();
        const bool         clamped = plate.get_parameters().center_support == CenterSupport::clamped;

        SparseMatrix<double> stiffness_matrix(plate.get_sparsity_pattern());
        SparseMatrix<double> mass_matrix(plate.get_sparsity_pattern());
        stiffness_matrix.copy_from(plate.get_stiffness_matrix());
        mass_matrix.copy_from(plate.get_mass_matrix());

        // 1. Clamp the bolt (u = 0). We remove the bolt rows and columns from K and M but keep a
        //    nonzero diagonal entry in K. Setting the diagonal of M to zero there gives each bolt
        //    DoF the eigenvalue "K_ii / 0 = infinity", far away from every frequency we look for.
        //    The free plate is not held anywhere, so it keeps K and M as they are.
        if (clamped) {
            std::map<types::global_dof_index, double> clamped_values;
            for (const auto dof : plate.get_bolt_dofs())
                clamped_values[dof] = 0.;

            Vector<double> unused_solution(n_dofs), unused_rhs(n_dofs);
            MatrixTools::apply_boundary_values(clamped_values,
                                               stiffness_matrix,
                                               unused_solution,
                                               unused_rhs);
            MatrixTools::apply_boundary_values(clamped_values,
                                               mass_matrix,
                                               unused_solution,
                                               unused_rhs);
            for (const auto dof : plate.get_bolt_dofs())
                mass_matrix.set(dof, dof, 0.);
        }

        PETScWrappers::SparseMatrix petsc_stiffness_matrix, petsc_mass_matrix;
        copy_to_petsc(stiffness_matrix, petsc_stiffness_matrix);
        copy_to_petsc(mass_matrix, petsc_mass_matrix);

        // 2. Shift-and-invert around sigma = omega_target^2. Every application of the inverse
        //    (K - sigma M)^{-1} is done with a sparse LU factorization (MUMPS).
        //
        //    The free plate has 6 rigid body motions (3 translations, 3 rotations) with
        //    omega = 0: K is singular. Then sigma = 0 is not allowed, so we shift slightly below
        //    zero instead. We also ask for 6 more modes, because some of them may be rigid
        //    motions, which we throw away below.
        const double omega_target = 2. * numbers::PI * target_frequency;
        double       sigma        = omega_target * omega_target;
        if (!clamped && target_frequency < rigid_body_frequency)
            sigma = -std::pow(2. * numbers::PI * 10. * rigid_body_frequency, 2);

        const unsigned int n_rigid_body_modes = clamped ? 0 : 6;
        const unsigned int n_computed         = n_modes + n_rigid_body_modes;

        SolverControl                    linear_solver_control(1, 0.);
        PETScWrappers::SparseDirectMUMPS linear_solver(linear_solver_control);
        SLEPcWrappers::TransformationShiftInvert shift_invert(
            MPI_COMM_SELF, SLEPcWrappers::TransformationShiftInvert::AdditionalData(sigma));
        shift_invert.set_solver(linear_solver);

        SolverControl                    eigen_solver_control(1000, 1e-10);
        SLEPcWrappers::SolverKrylovSchur eigen_solver(eigen_solver_control, MPI_COMM_SELF);
        eigen_solver.set_problem_type(EPS_GHEP);
        eigen_solver.set_transformation(shift_invert);
        eigen_solver.set_target_eigenvalue(sigma);
        eigen_solver.set_which_eigenpairs(EPS_TARGET_MAGNITUDE);

        std::vector<double>                     eigenvalues(n_computed);
        std::vector<PETScWrappers::MPI::Vector> eigenvectors(n_computed);
        for (auto &eigenvector : eigenvectors)
            eigenvector.reinit(complete_index_set(n_dofs), MPI_COMM_SELF);

        eigen_solver.solve(
            petsc_stiffness_matrix, petsc_mass_matrix, eigenvalues, eigenvectors, n_computed);

        // 3. Convert omega^2 to frequencies, drop rigid body motions and scale each mode such
        //    that max |u_z| = 1.
        std::vector<Eigenmode> modes;
        for (unsigned int k = 0; k < n_computed; ++k) {
            Eigenmode mode;
            mode.frequency = std::sqrt(std::max(eigenvalues[k], 0.)) / (2. * numbers::PI);
            if (mode.frequency < rigid_body_frequency)
                continue;
            mode.displacement = Vector<double>(eigenvectors[k]);

            double max_vertical = 0.;
            for (const auto dof : plate.get_z_dofs())
                max_vertical = std::max(max_vertical, std::abs(mode.displacement[dof]));
            if (max_vertical > 0.)
                mode.displacement /= max_vertical;

            modes.push_back(mode);
        }

        // Keep the n_modes modes closest to the target, sorted by frequency.
        const auto distance_to_target = [&](const Eigenmode &mode) {
            const double omega = 2. * numbers::PI * mode.frequency;
            return std::abs(omega * omega - omega_target * omega_target);
        };
        std::sort(modes.begin(), modes.end(), [&](const Eigenmode &a, const Eigenmode &b) {
            return distance_to_target(a) < distance_to_target(b);
        });
        if (modes.size() > n_modes)
            modes.resize(n_modes);

        std::sort(modes.begin(), modes.end(), [](const Eigenmode &a, const Eigenmode &b) {
            return a.frequency < b.frequency;
        });

        return modes;
    }
} // namespace Chladni
