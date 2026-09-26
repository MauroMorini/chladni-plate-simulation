#include <chladni/eigenmode_solver.h>
#include <chladni/forced_response_solver.h>
#include <chladni/output.h>
#include <chladni/parameters.h>
#include <chladni/plate.h>

#include <deal.II/base/mpi.h>
#include <deal.II/base/timer.h>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace Chladni;

namespace
{
    /** e.g. frequency_tag(512.345) == "512.3Hz" */
    std::string frequency_tag(const double frequency)
    {
        std::ostringstream tag;
        tag << std::fixed << std::setprecision(1) << frequency << "Hz";
        return tag.str();
    }

    void run_eigenmodes(const Parameters &parameters, const Plate &plate)
    {
        std::cout << "Computing " << parameters.number_of_eigenmodes
                  << " eigenmodes closest to " << parameters.frequency << " Hz ..." << std::endl;

        const EigenmodeSolver eigenmode_solver(plate);
        const auto modes = eigenmode_solver.solve(parameters.frequency,
                                                  parameters.number_of_eigenmodes);

        for (unsigned int k = 0; k < modes.size(); ++k) {
            std::ostringstream filename;
            filename << parameters.output_directory << "/mode_" << std::setw(2)
                     << std::setfill('0') << k << '_' << frequency_tag(modes[k].frequency)
                     << ".vtu";
            write_vtu(plate, modes[k].displacement, filename.str());

            std::cout << "  mode " << k << ": " << std::setprecision(6) << modes[k].frequency
                      << " Hz  -> " << filename.str() << std::endl;
        }
        write_eigenfrequencies_csv(modes, parameters.output_directory + "/eigenfrequencies.csv");
    }

    void run_forced(const Parameters &parameters, const Plate &plate)
    {
        std::cout << "Shaking the bolt at " << parameters.frequency << " Hz ..." << std::endl;

        const ForcedResponseSolver forced_solver(plate, parameters.drive_amplitude);
        const auto displacement = forced_solver.solve(parameters.frequency);

        const std::string filename =
            parameters.output_directory + "/forced_" + frequency_tag(parameters.frequency) + ".vtu";
        write_vtu(plate, displacement, filename);

        std::cout << "  response amplitude G = RMS(u_z) / A = "
                  << forced_solver.response_amplitude(displacement) << "  -> " << filename
                  << std::endl;
    }

    void run_sweep(const Parameters &parameters, const Plate &plate)
    {
        std::cout << "Sweeping " << parameters.sweep_steps << " frequencies from "
                  << parameters.sweep_min_frequency << " Hz to " << parameters.sweep_max_frequency
                  << " Hz ..." << std::endl;

        const ForcedResponseSolver forced_solver(plate, parameters.drive_amplitude);
        const auto curve = forced_solver.sweep(parameters.sweep_min_frequency,
                                               parameters.sweep_max_frequency,
                                               parameters.sweep_steps);

        const std::string filename = parameters.output_directory + "/sweep.csv";
        write_sweep_csv(curve, filename);
        std::cout << "  resonance curve -> " << filename << std::endl;
    }
} // namespace

int main(int argc, char *argv[])
{
    try {
        // PETSc and SLEPc need MPI to be initialized, even though we run on a single process.
        Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);
        AssertThrow(Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD) == 1,
                    ExcMessage("This program runs on a single MPI process only."));

        const std::string parameter_file = (argc > 1) ? argv[1] : "parameters/default.prm";
        const Parameters  parameters     = Parameters::from_file(parameter_file);

        // Keep a copy of the parameters next to the results, so every run can be reproduced.
        std::filesystem::create_directories(parameters.output_directory);
        std::filesystem::copy_file(parameter_file,
                                   parameters.output_directory + "/parameters.prm",
                                   std::filesystem::copy_options::overwrite_existing);

        Timer timer;

        std::cout << "Setting up the plate ..." << std::endl;
        const Plate plate(parameters);
        std::cout << "  " << plate.get_dof_handler().n_dofs() << " degrees of freedom, "
                  << plate.get_bolt_dofs().size() << " of them in the bolt" << std::endl;

        switch (parameters.mode) {
            case Mode::eigenmodes:
                run_eigenmodes(parameters, plate);
                break;
            case Mode::forced:
                run_forced(parameters, plate);
                break;
            case Mode::sweep:
                run_sweep(parameters, plate);
                break;
        }

        std::cout << "Done in " << timer.wall_time() << " s." << std::endl;
    }
    catch (std::exception &exc) {
        std::cerr << std::endl
                  << std::endl
                  << "----------------------------------------------------" << std::endl;
        std::cerr << "Exception on processing: " << std::endl
                  << exc.what() << std::endl
                  << "Aborting!" << std::endl
                  << "----------------------------------------------------" << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << std::endl
                  << std::endl
                  << "----------------------------------------------------" << std::endl;
        std::cerr << "Unknown exception!" << std::endl
                  << "Aborting!" << std::endl
                  << "----------------------------------------------------" << std::endl;
        return 1;
    }

    return 0;
}
