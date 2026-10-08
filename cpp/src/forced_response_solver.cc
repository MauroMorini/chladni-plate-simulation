#include <chladni/forced_response_solver.h>

#include <deal.II/base/numbers.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/numerics/matrix_tools.h>

#include <cmath>
#include <map>

namespace Chladni
{
    ForcedResponseSolver::ForcedResponseSolver(const Plate &plate)
        : plate(plate)
    {}

    double ForcedResponseSolver::drive_magnitude() const
    {
        const Parameters &parameters = plate.get_parameters();
        return (parameters.center_support == CenterSupport::clamped) ? parameters.drive_amplitude :
                                                                       parameters.drive_force;
    }

    Vector<double> ForcedResponseSolver::solve(const double frequency) const
    {
        const double omega = 2. * numbers::PI * frequency;

        // System matrix A = K - omega^2 M.
        SparseMatrix<double> system_matrix(plate.get_sparsity_pattern());
        system_matrix.copy_from(plate.get_stiffness_matrix());
        system_matrix.add(-omega * omega, plate.get_mass_matrix());

        const unsigned int n_dofs = plate.get_dof_handler().n_dofs();
        Vector<double>     displacement(n_dofs);
        Vector<double>     right_hand_side(n_dofs);

        if (plate.get_parameters().center_support == CenterSupport::clamped) {
            // Prescribe the motion of the bolt: vertical amplitude A, no horizontal motion.
            std::map<types::global_dof_index, double> bolt_values;
            for (const auto dof : plate.get_bolt_dofs())
                bolt_values[dof] = plate.get_z_dofs().is_element(dof) ? drive_magnitude() : 0.;

            MatrixTools::apply_boundary_values(bolt_values,
                                               system_matrix,
                                               displacement,
                                               right_hand_side);
        }
        else {
            // Push the bolt upwards with the total force F. Each vertical bolt DoF gets a share
            // proportional to the mass it carries, (M 1_z)_i, as if a uniform force density
            // acted on the bolt material. The rest of the plate is free.
            Vector<double> unit_vertical_motion(n_dofs);
            for (const auto dof : plate.get_z_dofs())
                unit_vertical_motion[dof] = 1.;
            Vector<double> nodal_mass(n_dofs);
            plate.get_mass_matrix().vmult(nodal_mass, unit_vertical_motion);

            double bolt_mass = 0.;
            for (const auto dof : plate.get_bolt_dofs())
                if (plate.get_z_dofs().is_element(dof))
                    bolt_mass += nodal_mass[dof];

            for (const auto dof : plate.get_bolt_dofs())
                if (plate.get_z_dofs().is_element(dof))
                    right_hand_side[dof] = drive_magnitude() * nodal_mass[dof] / bolt_mass;
        }

        // The matrix is indefinite (above the first eigenfrequency), so we use a direct solver.
        // For the free plate it is singular at f = 0 (rigid body motions), so f must be > 0.
        SparseDirectUMFPACK direct_solver;
        direct_solver.initialize(system_matrix);
        direct_solver.vmult(displacement, right_hand_side);

        return displacement;
    }

    double ForcedResponseSolver::response_amplitude(const Vector<double> &displacement) const
    {
        // Keep only the vertical components u_z. Because M only couples equal components,
        //   u_z^T M u_z = rho * int u_z^2 dx   and   1_z^T M 1_z = rho * |Omega|,
        // where 1_z is a rigid vertical motion with amplitude 1. Their ratio is the mean of u_z^2.
        Vector<double> vertical_displacement(displacement.size());
        for (const auto dof : plate.get_z_dofs())
            vertical_displacement[dof] = displacement[dof];

        Vector<double> unit_vertical_motion(displacement.size());
        for (const auto dof : plate.get_z_dofs())
            unit_vertical_motion[dof] = 1.;

        const SparseMatrix<double> &mass_matrix = plate.get_mass_matrix();

        const double mean_square = mass_matrix.matrix_norm_square(vertical_displacement) /
                                   mass_matrix.matrix_norm_square(unit_vertical_motion);

        return std::sqrt(mean_square) / drive_magnitude();
    }

    std::vector<SweepPoint> ForcedResponseSolver::sweep(const double       f_min,
                                                        const double       f_max,
                                                        const unsigned int n_steps) const
    {
        std::vector<SweepPoint> curve;
        for (unsigned int step = 0; step < n_steps; ++step) {
            const double frequency = f_min + (f_max - f_min) * step / (n_steps - 1);
            const auto   displacement = solve(frequency);
            curve.push_back({frequency, response_amplitude(displacement)});
        }
        return curve;
    }
} // namespace Chladni
