#include <chladni/plate.h>

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_values_extractors.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/full_matrix.h>

#include <cmath>
#include <iostream>

namespace Chladni
{
    Plate::Plate(const Parameters &parameters)
        : parameters(parameters)
        , fe(FE_Q<dim>(parameters.polynomial_degree) ^ dim)
        , mapping(1)
    {
        make_mesh();
        setup_dofs();
        assemble_matrices();
        find_bolt_dofs();
    }

    void Plate::make_mesh()
    {
        const double half_side = parameters.side_length / 2.;

        const Point<dim> lower_corner(-half_side, -half_side, 0.);
        const Point<dim> upper_corner(half_side, half_side, parameters.thickness);

        const std::vector<unsigned int> cells_per_direction = {parameters.cells_per_side,
                                                               parameters.cells_per_side,
                                                               parameters.cells_through_thickness};

        GridGenerator::subdivided_hyper_rectangle(triangulation,
                                                  cells_per_direction,
                                                  lower_corner,
                                                  upper_corner);
    }

    void Plate::setup_dofs()
    {
        dof_handler.reinit(triangulation);
        dof_handler.distribute_dofs(fe);

        DynamicSparsityPattern dsp(dof_handler.n_dofs());
        DoFTools::make_sparsity_pattern(dof_handler, dsp);
        sparsity_pattern.copy_from(dsp);

        stiffness_matrix.reinit(sparsity_pattern);
        mass_matrix.reinit(sparsity_pattern);

        const FEValuesExtractors::Scalar z_displacement(2);
        z_dofs = DoFTools::extract_dofs(dof_handler, fe.component_mask(z_displacement));
    }

    void Plate::assemble_matrices()
    {
        const double lambda = parameters.lame_lambda();
        const double mu     = parameters.lame_mu();
        const double rho    = parameters.density;

        const QGauss<dim> quadrature(fe.degree + 1);
        FEValues<dim>     fe_values(mapping,
                                fe,
                                quadrature,
                                update_values | update_gradients | update_JxW_values);

        const FEValuesExtractors::Vector displacement(0);

        const unsigned int dofs_per_cell = fe.n_dofs_per_cell();
        FullMatrix<double> cell_stiffness(dofs_per_cell, dofs_per_cell);
        FullMatrix<double> cell_mass(dofs_per_cell, dofs_per_cell);
        std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);

        for (const auto &cell : dof_handler.active_cell_iterators()) {
            fe_values.reinit(cell);
            cell_stiffness = 0.;
            cell_mass      = 0.;

            for (const unsigned int q : fe_values.quadrature_point_indices()) {
                const double dx = fe_values.JxW(q);

                for (const unsigned int i : fe_values.dof_indices()) {
                    const auto eps_i = fe_values[displacement].symmetric_gradient(i, q);
                    const auto div_i = fe_values[displacement].divergence(i, q);
                    const auto phi_i = fe_values[displacement].value(i, q);

                    for (const unsigned int j : fe_values.dof_indices()) {
                        const auto eps_j = fe_values[displacement].symmetric_gradient(j, q);
                        const auto div_j = fe_values[displacement].divergence(j, q);
                        const auto phi_j = fe_values[displacement].value(j, q);

                        cell_stiffness(i, j) +=
                            (2. * mu * scalar_product(eps_i, eps_j) + lambda * div_i * div_j) * dx;
                        cell_mass(i, j) += rho * (phi_i * phi_j) * dx;
                    }
                }
            }

            cell->get_dof_indices(local_dof_indices);
            stiffness_matrix.add(local_dof_indices, cell_stiffness);
            mass_matrix.add(local_dof_indices, cell_mass);
        }
    }

    void Plate::find_bolt_dofs()
    {
        const auto support_points = DoFTools::map_dofs_to_support_points(mapping, dof_handler);

        // Small tolerance so that DoFs lying exactly on the bolt radius are included.
        const double tolerance = 1e-10 * parameters.side_length;

        bolt_dofs.clear();
        for (const auto &[dof, point] : support_points) {
            const double distance_to_axis = std::hypot(point[0], point[1]);
            if (distance_to_axis <= parameters.bolt_radius + tolerance)
                bolt_dofs.push_back(dof);
        }

        // The mesh has an even number of cells per side, so the center axis x = y = 0 always
        // consists of mesh vertices and the bolt is never empty.
        AssertThrow(!bolt_dofs.empty(), ExcMessage("No degrees of freedom found in the bolt."));

        const double cell_size = parameters.side_length / parameters.cells_per_side;
        if (parameters.bolt_radius < cell_size / fe.degree)
            std::cout << "Warning: the bolt radius (" << parameters.bolt_radius
                      << " m) is smaller than the node spacing (" << cell_size / fe.degree
                      << " m). Only the center axis is fixed and the results depend on the mesh."
                      << std::endl;
    }

    const DoFHandler<dim> &Plate::get_dof_handler() const
    {
        return dof_handler;
    }

    const MappingQ<dim> &Plate::get_mapping() const
    {
        return mapping;
    }

    const SparsityPattern &Plate::get_sparsity_pattern() const
    {
        return sparsity_pattern;
    }

    const SparseMatrix<double> &Plate::get_stiffness_matrix() const
    {
        return stiffness_matrix;
    }

    const SparseMatrix<double> &Plate::get_mass_matrix() const
    {
        return mass_matrix;
    }

    const std::vector<types::global_dof_index> &Plate::get_bolt_dofs() const
    {
        return bolt_dofs;
    }

    const IndexSet &Plate::get_z_dofs() const
    {
        return z_dofs;
    }

    double Plate::volume() const
    {
        return parameters.side_length * parameters.side_length * parameters.thickness;
    }
} // namespace Chladni
