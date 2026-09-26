#pragma once

#include <chladni/plate.h>

#include <deal.II/lac/vector.h>

#include <vector>

namespace Chladni
{
    using namespace dealii;

    /** One point of a resonance curve. */
    struct SweepPoint
    {
        double frequency;          // drive frequency f [Hz]
        double response_amplitude; // G(f) = RMS(u_z) / A [-]
    };

    /**
     * The plate driven by a shaker at the bolt, as in the real Chladni experiment.
     *
     * The bolt moves up and down as A cos(omega t). After transients have died out, the whole
     * plate oscillates with the same frequency, u(x, t) = u_hat(x) cos(omega t), where the
     * amplitude field u_hat solves
     *
     *   (K - omega^2 M) u_hat = 0   for all DoFs outside the bolt,
     *   u_hat = (0, 0, A)           on the bolt.
     *
     * This has a unique solution for every frequency that is not an eigenfrequency of the
     * clamped plate. Close to an eigenfrequency the response becomes large and looks like the
     * corresponding eigenmode: that is the Chladni pattern.
     */
    class ForcedResponseSolver
    {
    public:
        ForcedResponseSolver(const Plate &plate, double drive_amplitude);

        /** Amplitude field u_hat for the drive frequency @p frequency [Hz]. */
        Vector<double> solve(double frequency) const;

        /**
         * Dimensionless response G = ||u_z||_L2 / (A |Omega|^(1/2)), i.e. the root mean square
         * of the vertical displacement divided by the drive amplitude. G = 1 means the plate
         * moves, on average, as much as the bolt.
         */
        double response_amplitude(const Vector<double> &displacement) const;

        /** Solve for @p n_steps equally spaced frequencies in [f_min, f_max]. */
        std::vector<SweepPoint> sweep(double f_min, double f_max, unsigned int n_steps) const;

    private:
        const Plate &plate;
        const double drive_amplitude;
    };
} // namespace Chladni
