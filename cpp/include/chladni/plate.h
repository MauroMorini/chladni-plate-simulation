#pragma once

#include <chladni/parameters.h>

#include <deal.II/base/index_set.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/mapping_q.h>
#include <deal.II/grid/tria.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparsity_pattern.h>

#include <vector>

namespace Chladni
{
    using namespace dealii;

    /** The plate is a genuine 3D elastic body. */
    constexpr unsigned int dim = 3;

    /**
     * The discretized plate: mesh, finite element space, stiffness matrix K, mass matrix M and the
     * degrees of freedom that belong to the bolt at the center.
     *
     * The plate occupies [-L/2, L/2]^2 x [0, h]. Its displacement u is approximated with
     * continuous Q_p elements in each of the three components. The matrices are assembled
     * without any boundary conditions: all outer faces are traction free (zero Neumann) and the
     * bolt is handled by the solvers, which know whether it is clamped or shaken.
     *
     *   K_ij = int_Omega 2 mu eps(phi_i) : eps(phi_j) + lambda div(phi_i) div(phi_j) dx
     *   M_ij = int_Omega rho phi_i . phi_j dx
     */
    class Plate
    {
      public:
        explicit Plate(const Parameters &parameters);

        const Parameters           &get_parameters() const;
        const DoFHandler<dim>      &get_dof_handler() const;
        const MappingQ<dim>        &get_mapping() const;
        const SparsityPattern      &get_sparsity_pattern() const;
        const SparseMatrix<double> &get_stiffness_matrix() const;
        const SparseMatrix<double> &get_mass_matrix() const;

        /** DoFs whose support point lies inside the bolt (a cylinder of radius r around the
         * vertical center axis, through the whole thickness). */
        const std::vector<types::global_dof_index> &get_bolt_dofs() const;

        /** DoFs that describe the vertical (z) displacement. */
        const IndexSet &get_z_dofs() const;

        /** Volume L^2 h of the plate. */
        double volume() const;

      private:
        void make_mesh();
        void setup_dofs();
        void assemble_matrices();
        void find_bolt_dofs();

        const Parameters parameters;

        Triangulation<dim> triangulation;
        FESystem<dim>      fe;
        MappingQ<dim>      mapping;
        DoFHandler<dim>    dof_handler;

        SparsityPattern      sparsity_pattern;
        SparseMatrix<double> stiffness_matrix;
        SparseMatrix<double> mass_matrix;

        std::vector<types::global_dof_index> bolt_dofs;
        IndexSet                             z_dofs;
    };
} // namespace Chladni
