#pragma once

#include <chladni/plate.h>

#include <deal.II/base/symmetric_tensor.h>
#include <deal.II/base/tensor.h>

namespace Chladni
{
    using namespace dealii;

    /**
     * Cauchy stress of a linear isotropic material (Hooke's law):
     *
     *   sigma = lambda tr(eps) I + 2 mu eps,   eps = (grad u + grad u^T) / 2.
     */
    SymmetricTensor<2, dim> stress(const Tensor<2, dim> &displacement_gradient,
                                   double                lambda,
                                   double                mu);

    /**
     * Von Mises stress sqrt(3/2 s:s), where s = sigma - tr(sigma)/3 I is the deviatoric stress.
     *
     * This scalar summarizes the stress tensor. It equals |sigma_xx| for uniaxial tension and
     * is the standard measure for where a metal is loaded most.
     */
    double von_mises_stress(const Tensor<2, dim> &displacement_gradient, double lambda, double mu);
} // namespace Chladni
