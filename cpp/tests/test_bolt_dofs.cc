// The bolt consists of all DoFs within the bolt radius of the vertical center axis, for all
// three components and through the whole thickness.

#include "test_utilities.h"

#include <chladni/plate.h>

#include <deal.II/dofs/dof_tools.h>

#include <algorithm>
#include <cmath>

using namespace Chladni;

int main()
{
    Parameters parameters = small_plate_parameters();
    const Plate plate(parameters);

    const auto &bolt_dofs = plate.get_bolt_dofs();
    const auto  support_points =
        DoFTools::map_dofs_to_support_points(plate.get_mapping(), plate.get_dof_handler());

    check(!bolt_dofs.empty(), "the bolt is not empty");
    check(bolt_dofs.size() % dim == 0, "every bolt point has all three components");

    const bool all_inside = std::all_of(bolt_dofs.begin(), bolt_dofs.end(), [&](const auto dof) {
        const auto &p = support_points.at(dof);
        return std::hypot(p[0], p[1]) <= parameters.bolt_radius * (1. + 1e-8);
    });
    check(all_inside, "all bolt DoFs lie inside the bolt radius");

    // 4 cells per side, degree 2: node spacing 0.03 = bolt radius. The bolt therefore contains the
    // center axis and its 4 direct neighbors. Each has 3 nodes through the thickness (1 cell,
    // degree 2), so the bolt has 5 axes * 3 nodes * 3 components = 45 DoFs.
    check(bolt_dofs.size() == 5 * 3 * dim, "bolt of radius 0.03 has 45 DoFs");

    // A tiny bolt still holds the center axis (with a warning).
    parameters.bolt_radius = 1e-6;
    const Plate plate_with_tiny_bolt(parameters);
    check(plate_with_tiny_bolt.get_bolt_dofs().size() == 3 * dim,
          "tiny bolt contains only the center axis");

    return 0;
}
