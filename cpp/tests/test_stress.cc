// The von Mises stress gives the textbook values for two simple stress states.

#include "test_utilities.h"

#include <chladni/stress.h>

#include <cmath>

using namespace Chladni;

int main()
{
    const double E = 2., nu = 0.3;
    Parameters   material;
    material.youngs_modulus = E;
    material.poisson_ratio  = nu;
    const double lambda = material.lame_lambda(), mu = material.lame_mu();

    // Uniaxial tension sigma_xx = s: eps_xx = s/E, eps_yy = eps_zz = -nu s/E. Von Mises = |s|.
    const double   s = 3.;
    Tensor<2, dim> uniaxial;
    uniaxial[0][0] = s / E;
    uniaxial[1][1] = -nu * s / E;
    uniaxial[2][2] = -nu * s / E;
    check_relative(stress(uniaxial, lambda, mu)[0][0], s, 1e-12, "uniaxial: sigma_xx = s");
    check(std::abs(stress(uniaxial, lambda, mu)[1][1]) < 1e-12, "uniaxial: sigma_yy = 0");
    check_relative(von_mises_stress(uniaxial, lambda, mu), s, 1e-12, "uniaxial: von Mises = s");

    // Pure shear sigma_xy = t: eps_xy = t / (2 mu). Von Mises = sqrt(3) t.
    const double   t = 5.;
    Tensor<2, dim> shear;
    shear[0][1] = t / (2. * mu);
    shear[1][0] = t / (2. * mu);
    check_relative(von_mises_stress(shear, lambda, mu), std::sqrt(3.) * t, 1e-12,
                   "pure shear: von Mises = sqrt(3) t");

    // A rigid rotation (antisymmetric gradient) causes no stress.
    Tensor<2, dim> rotation;
    rotation[0][1] = 1.;
    rotation[1][0] = -1.;
    check(von_mises_stress(rotation, lambda, mu) < 1e-12, "rotation: no stress");

    return 0;
}
