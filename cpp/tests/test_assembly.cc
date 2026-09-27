// Sanity checks of the stiffness and mass matrices:
// - rigid body motions (3 translations, 3 rotations) cost no elastic energy: K r = 0,
// - the mass matrix integrates the density: 1_x^T M 1_x = rho * L^2 * h.

#include "test_utilities.h"

#include <chladni/plate.h>

#include <deal.II/dofs/dof_tools.h>
#include <deal.II/fe/fe_values_extractors.h>
#include <deal.II/lac/vector.h>

using namespace Chladni;

int main()
{
    const Parameters parameters = small_plate_parameters();
    const Plate      plate(parameters);

    const auto &dof_handler      = plate.get_dof_handler();
    const auto &stiffness_matrix = plate.get_stiffness_matrix();
    const auto &mass_matrix      = plate.get_mass_matrix();
    const auto  support_points =
        DoFTools::map_dofs_to_support_points(plate.get_mapping(), dof_handler);

    // Which vector component (0 = x, 1 = y, 2 = z) each DoF belongs to.
    std::vector<unsigned int> component_of_dof(dof_handler.n_dofs());
    for (unsigned int c = 0; c < dim; ++c) {
        const FEValuesExtractors::Scalar component(c);
        for (const auto dof :
             DoFTools::extract_dofs(dof_handler, dof_handler.get_fe().component_mask(component)))
            component_of_dof[dof] = c;
    }

    // Build a nodal vector from a displacement field u(x).
    auto interpolate = [&](const auto &displacement_field) {
        Vector<double> result(dof_handler.n_dofs());
        for (const auto &[dof, point] : support_points)
            result[dof] = displacement_field(point)[component_of_dof[dof]];
        return result;
    };

    std::vector<Tensor<1, dim>> unit_vectors(dim);
    for (unsigned int d = 0; d < dim; ++d)
        unit_vectors[d][d] = 1.;

    const double stiffness_scale = stiffness_matrix.linfty_norm();

    for (unsigned int d = 0; d < dim; ++d) {
        const auto translation = interpolate([&](const Point<dim> &) { return unit_vectors[d]; });
        const auto rotation    = interpolate(
            [&](const Point<dim> &x) { return cross_product_3d(unit_vectors[d], Tensor<1, dim>(x)); });

        Vector<double> k_times_r(dof_handler.n_dofs());

        stiffness_matrix.vmult(k_times_r, translation);
        check(k_times_r.linfty_norm() < 1e-10 * stiffness_scale * translation.linfty_norm(),
              "K * translation " + std::to_string(d) + " = 0");

        stiffness_matrix.vmult(k_times_r, rotation);
        check(k_times_r.linfty_norm() < 1e-10 * stiffness_scale * rotation.linfty_norm(),
              "K * rotation " + std::to_string(d) + " = 0");
    }

    const auto unit_x_motion = interpolate([&](const Point<dim> &) { return unit_vectors[0]; });
    check_relative(mass_matrix.matrix_norm_square(unit_x_motion),
                   parameters.density * plate.volume(),
                   1e-12,
                   "total mass");

    return 0;
}
