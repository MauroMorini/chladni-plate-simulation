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
        double response_amplitude; // G(f) = RMS(u_z) / A [-]  or  RMS(u_z) / F [m/N]
    };

    /**
     * The plate driven by a shaker at the bolt, as in the real Chladni experiment.
     *
     * The shaker oscillates as cos(omega t). After transients have died out, the whole plate
     * oscillates with the same frequency, u(x, t) = u_hat(x) cos(omega t). How the amplitude field
     * u_hat is computed depends on the parameter 'Center support':
     *
     * - clamped (displacement drive): the bolt moves up and down with amplitude A,
     *
     *     (K - omega^2 M) u_hat = 0   for all DoFs outside the bolt,
     *     u_hat = (0, 0, A)           on the bolt.
     *
     *   Resonances are at the eigenfrequencies of the plate clamped at the bolt.
     *
     * - free (force drive): the bolt is pushed up and down with total force amplitude F,
     *
     *     (K - omega^2 M) u_hat = f,  f = F spread over the vertical bolt DoFs.
     *
     *   Resonances are at the eigenfrequencies of the free plate.
     *
     * Close to a resonance the response becomes large and looks like the corresponding
     * eigenmode: that is the Chladni pattern.
     */
    class ForcedResponseSolver
    {
    public:
        explicit ForcedResponseSolver(const Plate &plate);

        /** Amplitude field u_hat for the drive frequency @p frequency [Hz]. */
        Vector<double> solve(double frequency) const;

        /**
         * Response G = RMS(u_z) / drive, where RMS(u_z) = ||u_z||_L2 / |Omega|^(1/2) is the root
         * mean square of the vertical displacement and the drive is A (clamped, G is
         * dimensionless) or F (free, G in m/N). For the clamped plate, G = 1 means the plate
         * moves, on average, as much as the bolt.
         */
        double response_amplitude(const Vector<double> &displacement) const;

        /** Solve for @p n_steps equally spaced frequencies in [f_min, f_max]. */
        std::vector<SweepPoint> sweep(double f_min, double f_max, unsigned int n_steps) const;

    private:
        /** The drive amplitude A [m] (clamped) or drive force F [N] (free). */
        double drive_magnitude() const;

        const Plate &plate;
    };
} // namespace Chladni
