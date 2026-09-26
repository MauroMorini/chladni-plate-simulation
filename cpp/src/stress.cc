#include <chladni/stress.h>

#include <cmath>

namespace Chladni
{
    SymmetricTensor<2, dim> stress(const Tensor<2, dim> &displacement_gradient,
                                   const double          lambda,
                                   const double          mu)
    {
        const SymmetricTensor<2, dim> strain = symmetrize(displacement_gradient);
        return lambda * trace(strain) * unit_symmetric_tensor<dim>() + 2. * mu * strain;
    }

    double von_mises_stress(const Tensor<2, dim> &displacement_gradient,
                            const double          lambda,
                            const double          mu)
    {
        const SymmetricTensor<2, dim> deviatoric_stress =
            deviator(stress(displacement_gradient, lambda, mu));
        return std::sqrt(1.5 * scalar_product(deviatoric_stress, deviatoric_stress));
    }
} // namespace Chladni
