#pragma once

#include <chladni/plate.h>

#include <deal.II/lac/vector.h>

#include <vector>

namespace Chladni
{
    using namespace dealii;

    /** A free vibration of the plate with the bolt held fixed. */
    struct Eigenmode
    {
        double         frequency;    // f = omega / (2 pi) [Hz]
        Vector<double> displacement; // mode shape, scaled such that max |u_z| = 1
    };

    /**
     * Computes vibration modes of the plate clamped at the bolt (u = 0 there):
     *
     *   K u = omega^2 M u.
     *
     * We are interested in the modes near a chosen frequency f, not only in the lowest ones.
     * SLEPc's shift-and-invert transformation does exactly that: with the shift
     * sigma = (2 pi f)^2 it iterates with (K - sigma M)^{-1} M, whose largest eigenvalues
     * 1 / (omega_k^2 - sigma) belong to the modes with omega_k^2 closest to sigma.
     */
    class EigenmodeSolver
    {
    public:
        explicit EigenmodeSolver(const Plate &plate);

        /** The @p n_modes eigenmodes closest to @p target_frequency [Hz], sorted by frequency. */
        std::vector<Eigenmode> solve(double target_frequency, unsigned int n_modes) const;

    private:
        const Plate &plate;
    };
} // namespace Chladni
