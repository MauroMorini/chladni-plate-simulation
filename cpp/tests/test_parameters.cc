// Parameters are read from .prm text, unspecified entries keep their defaults and the Lamé
// constants follow from E and nu.

#include "test_utilities.h"

#include <chladni/parameters.h>

using namespace Chladni;

int main()
{
    const Parameters parameters = Parameters::from_string(R"(
        subsection Material
          set Young's modulus = 1
          set Poisson ratio   = 0.25
        end
        subsection Simulation
          set Mode      = sweep
          set Frequency = 123.5
        end
    )");

    check_relative(parameters.youngs_modulus, 1., 1e-14, "Young's modulus is read");
    check_relative(parameters.frequency, 123.5, 1e-14, "frequency is read");
    check(parameters.mode == Mode::sweep, "mode is read");
    check_relative(parameters.density, Parameters().density, 1e-14, "density keeps its default");

    // E = 1, nu = 1/4:  lambda = 0.25 / (1.25 * 0.5) = 0.4,  mu = 1 / 2.5 = 0.4
    check_relative(parameters.lame_lambda(), 0.4, 1e-14, "Lamé lambda");
    check_relative(parameters.lame_mu(), 0.4, 1e-14, "Lamé mu");

    bool odd_cells_rejected = false;
    try {
        Parameters::from_string("subsection Discretization\n set Cells per side = 5\nend\n");
    }
    catch (const std::exception &) {
        odd_cells_rejected = true;
    }
    check(odd_cells_rejected, "an odd number of cells per side is rejected");

    return 0;
}
