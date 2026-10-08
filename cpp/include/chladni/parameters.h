#pragma once

#include <deal.II/base/parameter_handler.h>

#include <string>

namespace Chladni
{
    using namespace dealii;

    /**
     * What the program should compute.
     *
     * - eigenmodes: the free vibration modes whose frequencies are closest to a target frequency.
     * - forced:     the steady vibration of the plate when the bolt is shaken at one frequency.
     * - sweep:      the forced response for many frequencies, giving a resonance curve.
     */
    enum class Mode
    {
        eigenmodes,
        forced,
        sweep
    };

    /**
     * How the shaker holds the center of the plate.
     *
     * - clamped: the shaker dictates the motion of the bolt (displacement drive). Eigenmodes have
     *            u = 0 on the bolt, the forced bolt moves with amplitude A.
     * - free:    the shaker only pushes on the bolt (force drive). Eigenmodes are those of the
     *            completely free plate, the forced bolt is pushed with force amplitude F.
     *
     * A real shaker lies somewhere in between: a heavy, stiff shaker is close to "clamped", a
     * light one (compared to the plate) is close to "free".
     */
    enum class CenterSupport
    {
        clamped,
        free
    };

    /**
     * All user-facing parameters of the simulation, read from a deal.II .prm file.
     *
     * All quantities are in SI units (m, Pa, kg/m^3, Hz).
     */
    struct Parameters
    {
        // Geometry: square plate [-L/2, L/2]^2 x [0, h], bolted at the center.
        double side_length = 0.24;
        double thickness   = 0.001;
        double bolt_radius = 0.005;

        // Material (defaults: aluminium).
        double youngs_modulus = 70e9;
        double poisson_ratio  = 0.33;
        double density        = 2700.;

        // Discretization.
        unsigned int cells_per_side          = 32;
        unsigned int cells_through_thickness = 1;
        unsigned int polynomial_degree       = 2;

        // Simulation.
        Mode          mode                 = Mode::eigenmodes;
        CenterSupport center_support       = CenterSupport::clamped;
        double        frequency            = 500.;
        unsigned int  number_of_eigenmodes = 6;
        double        drive_amplitude      = 1e-6;
        double        drive_force          = 0.01;
        double        sweep_min_frequency  = 10.;
        double        sweep_max_frequency  = 1000.;
        unsigned int  sweep_steps          = 200;

        // Output.
        std::string output_directory = "output";

        /** First Lamé parameter lambda = E nu / ((1 + nu)(1 - 2 nu)). */
        double lame_lambda() const;

        /** Second Lamé parameter (shear modulus) mu = E / (2 (1 + nu)). */
        double lame_mu() const;

        /** Declare all entries (with their default values) in @p prm. */
        static void declare(ParameterHandler &prm);

        /** Copy the values stored in @p prm into this object. */
        void parse(ParameterHandler &prm);

        /** Read a .prm file. */
        static Parameters from_file(const std::string &filename);

        /** Read parameters from a string in .prm format (handy for tests). */
        static Parameters from_string(const std::string &prm_content);
    };

    /** Convert a Mode into its name as used in the .prm file. */
    std::string to_string(Mode mode);

    /** Convert a CenterSupport into its name as used in the .prm file. */
    std::string to_string(CenterSupport center_support);
} // namespace Chladni
