#pragma once

#include <chladni/plate.h>

#include <deal.II/lac/vector.h>

#include <vector>

namespace Chladni
{
    using namespace dealii;

    /** A free vibration of the plate (no shaker force acts). */
    struct Eigenmode
    {
        double         frequency;    // f = omega / (2 pi) [Hz]
        Vector<double> displacement; // mode shape, scaled such that max |u_z| = 1
    };

    /**
     * Computes vibration modes of the plate,
     *
     *   K u = omega^2 M u,
     *
     * either clamped at the bolt (u = 0 there) or completely free, depending on the parameter
     * 'Center support'. The free plate also has 6 rigid body motions (omega = 0). They are not
     * vibrations and are left out.
     *
     * We are interested in the modes near a chosen frequency f, not only in the lowest ones.
     * SLEPc's shift-and-invert transformation does exactly that: with the shift
     * sigma = (2 pi f)^2 it iterates with (K - sigma M)^{-1} M, whose largest eigenvalues
     * 1 / (omega_k^2 - sigma) belong to the modes with omega_k^2 closest to sigma.
     */
    class EigenmodeSolver
    {
    public:
        /** Modes below this frequency [Hz] count as rigid body motions. No elastic mode of a
         * real Chladni plate is that slow. */
        static constexpr double rigid_body_frequency = 1.;

        explicit EigenmodeSolver(const Plate &plate);

        /** The @p n_modes eigenmodes closest to @p target_frequency [Hz], sorted by frequency. */
        std::vector<Eigenmode> solve(double target_frequency, unsigned int n_modes) const;

    private:
        const Plate &plate;
    };
} // namespace Chladni
