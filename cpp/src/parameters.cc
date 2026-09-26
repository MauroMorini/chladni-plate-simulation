#include <chladni/parameters.h>

#include <sstream>

namespace Chladni
{
    namespace
    {
        /** Short string representation of a number (e.g. "0.24", "7e+10") for default values. */
        template <typename Number>
        std::string str(const Number value)
        {
            std::ostringstream stream;
            stream << value;
            return stream.str();
        }
    } // namespace

    double Parameters::lame_lambda() const
    {
        const double E  = youngs_modulus;
        const double nu = poisson_ratio;
        return E * nu / ((1. + nu) * (1. - 2. * nu));
    }

    double Parameters::lame_mu() const
    {
        return youngs_modulus / (2. * (1. + poisson_ratio));
    }

    void Parameters::declare(ParameterHandler &prm)
    {
        // A default-constructed object holds the default values.
        const Parameters defaults;

        prm.enter_subsection("Geometry");
        {
            prm.declare_entry("Side length",
                              str(defaults.side_length),
                              Patterns::Double(0.),
                              "Edge length L of the square plate [m].");
            prm.declare_entry("Thickness",
                              str(defaults.thickness),
                              Patterns::Double(0.),
                              "Thickness h of the plate [m].");
            prm.declare_entry("Bolt radius",
                              str(defaults.bolt_radius),
                              Patterns::Double(0.),
                              "Radius of the bolt that holds the plate at its center [m]. "
                              "All points closer than this to the center axis are fixed/driven.");
        }
        prm.leave_subsection();

        prm.enter_subsection("Material");
        {
            prm.declare_entry("Young's modulus",
                              str(defaults.youngs_modulus),
                              Patterns::Double(0.),
                              "Young's modulus E [Pa].");
            prm.declare_entry("Poisson ratio",
                              str(defaults.poisson_ratio),
                              Patterns::Double(-1., 0.5),
                              "Poisson ratio nu [-].");
            prm.declare_entry("Density",
                              str(defaults.density),
                              Patterns::Double(0.),
                              "Mass density rho [kg/m^3].");
        }
        prm.leave_subsection();

        prm.enter_subsection("Discretization");
        {
            prm.declare_entry("Cells per side",
                              str(defaults.cells_per_side),
                              Patterns::Integer(2),
                              "Number of cells along x and along y. Must be even, so that the "
                              "center of the plate is a mesh vertex.");
            prm.declare_entry("Cells through thickness",
                              str(defaults.cells_through_thickness),
                              Patterns::Integer(1),
                              "Number of cells along z.");
            prm.declare_entry("Polynomial degree",
                              str(defaults.polynomial_degree),
                              Patterns::Integer(1),
                              "Degree of the Lagrange elements. Degree 1 locks badly for thin "
                              "plates, use at least 2.");
        }
        prm.leave_subsection();

        prm.enter_subsection("Simulation");
        {
            prm.declare_entry("Mode",
                              to_string(defaults.mode),
                              Patterns::Selection("eigenmodes|forced|sweep"),
                              "eigenmodes: vibration modes closest to 'Frequency'. "
                              "forced: response to shaking the bolt at 'Frequency'. "
                              "sweep: forced response for a range of frequencies.");
            prm.declare_entry("Frequency",
                              str(defaults.frequency),
                              Patterns::Double(0.),
                              "Target frequency (eigenmodes) or drive frequency (forced) [Hz].");
            prm.declare_entry("Number of eigenmodes",
                              str(defaults.number_of_eigenmodes),
                              Patterns::Integer(1),
                              "How many eigenmodes around 'Frequency' to compute.");
            prm.declare_entry("Drive amplitude",
                              str(defaults.drive_amplitude),
                              Patterns::Double(0.),
                              "Vertical amplitude A with which the bolt is shaken [m].");
            prm.declare_entry("Sweep min frequency",
                              str(defaults.sweep_min_frequency),
                              Patterns::Double(0.),
                              "Lowest frequency of the sweep [Hz].");
            prm.declare_entry("Sweep max frequency",
                              str(defaults.sweep_max_frequency),
                              Patterns::Double(0.),
                              "Highest frequency of the sweep [Hz].");
            prm.declare_entry("Sweep steps",
                              str(defaults.sweep_steps),
                              Patterns::Integer(2),
                              "Number of equally spaced frequencies in the sweep.");
        }
        prm.leave_subsection();

        prm.enter_subsection("Output");
        {
            prm.declare_entry("Output directory",
                              defaults.output_directory,
                              Patterns::DirectoryName(),
                              "Where .vtu and .csv files are written.");
        }
        prm.leave_subsection();
    }

    void Parameters::parse(ParameterHandler &prm)
    {
        prm.enter_subsection("Geometry");
        {
            side_length = prm.get_double("Side length");
            thickness   = prm.get_double("Thickness");
            bolt_radius = prm.get_double("Bolt radius");
        }
        prm.leave_subsection();

        prm.enter_subsection("Material");
        {
            youngs_modulus = prm.get_double("Young's modulus");
            poisson_ratio  = prm.get_double("Poisson ratio");
            density        = prm.get_double("Density");
        }
        prm.leave_subsection();

        prm.enter_subsection("Discretization");
        {
            cells_per_side          = prm.get_integer("Cells per side");
            cells_through_thickness = prm.get_integer("Cells through thickness");
            polynomial_degree       = prm.get_integer("Polynomial degree");
        }
        prm.leave_subsection();

        prm.enter_subsection("Simulation");
        {
            const std::string mode_name = prm.get("Mode");
            if (mode_name == "eigenmodes")
                mode = Mode::eigenmodes;
            else if (mode_name == "forced")
                mode = Mode::forced;
            else
                mode = Mode::sweep;

            frequency            = prm.get_double("Frequency");
            number_of_eigenmodes = prm.get_integer("Number of eigenmodes");
            drive_amplitude      = prm.get_double("Drive amplitude");
            sweep_min_frequency  = prm.get_double("Sweep min frequency");
            sweep_max_frequency  = prm.get_double("Sweep max frequency");
            sweep_steps          = prm.get_integer("Sweep steps");
        }
        prm.leave_subsection();

        prm.enter_subsection("Output");
        {
            output_directory = prm.get("Output directory");
        }
        prm.leave_subsection();

        AssertThrow(cells_per_side % 2 == 0,
                    ExcMessage("'Cells per side' must be even so that the plate center is a "
                               "mesh vertex."));
        AssertThrow(sweep_min_frequency < sweep_max_frequency,
                    ExcMessage("'Sweep min frequency' must be smaller than 'Sweep max frequency'."));
    }

    Parameters Parameters::from_file(const std::string &filename)
    {
        ParameterHandler prm;
        declare(prm);
        prm.parse_input(filename);

        Parameters parameters;
        parameters.parse(prm);
        return parameters;
    }

    Parameters Parameters::from_string(const std::string &prm_content)
    {
        ParameterHandler prm;
        declare(prm);
        prm.parse_input_from_string(prm_content);

        Parameters parameters;
        parameters.parse(prm);
        return parameters;
    }

    std::string to_string(const Mode mode)
    {
        switch (mode) {
            case Mode::eigenmodes:
                return "eigenmodes";
            case Mode::forced:
                return "forced";
            case Mode::sweep:
                return "sweep";
        }
        return "";
    }
} // namespace Chladni
