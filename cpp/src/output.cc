#include <chladni/output.h>

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/data_postprocessor.h>

#include <fstream>
#include <iomanip>

namespace Chladni
{
    namespace
    {
        /** Computes |u_z| from the displacement at every output point. */
        class AbsoluteVerticalDisplacement : public DataPostprocessorScalar<dim>
        {
        public:
            AbsoluteVerticalDisplacement()
                : DataPostprocessorScalar<dim>("abs_uz", update_values)
            {}

            void evaluate_vector_field(const DataPostprocessorInputs::Vector<dim> &inputs,
                                       std::vector<Vector<double>> &computed_quantities) const override
            {
                for (unsigned int p = 0; p < inputs.solution_values.size(); ++p)
                    computed_quantities[p](0) = std::abs(inputs.solution_values[p](2));
            }
        };
    } // namespace

    void write_vtu(const Plate          &plate,
                   const Vector<double> &displacement,
                   const std::string    &filename)
    {
        const std::vector<std::string> names(dim, "displacement");
        const std::vector<DataComponentInterpretation::DataComponentInterpretation>
            interpretation(dim, DataComponentInterpretation::component_is_part_of_vector);

        const AbsoluteVerticalDisplacement abs_uz;

        DataOut<dim> data_out;
        data_out.attach_dof_handler(plate.get_dof_handler());
        data_out.add_data_vector(displacement,
                                 names,
                                 DataOut<dim>::type_dof_data,
                                 interpretation);
        data_out.add_data_vector(displacement, abs_uz);

        // Subdivide each cell so that the higher-order solution is shown accurately.
        const unsigned int degree = plate.get_dof_handler().get_fe().degree;
        data_out.build_patches(plate.get_mapping(), degree);

        std::ofstream file(filename);
        data_out.write_vtu(file);
    }

    void write_eigenfrequencies_csv(const std::vector<Eigenmode> &modes,
                                    const std::string            &filename)
    {
        std::ofstream file(filename);
        file << "mode,frequency_hz\n" << std::setprecision(10);
        for (unsigned int k = 0; k < modes.size(); ++k)
            file << k << ',' << modes[k].frequency << '\n';
    }

    void write_sweep_csv(const std::vector<SweepPoint> &curve, const std::string &filename)
    {
        std::ofstream file(filename);
        file << "frequency_hz,response_amplitude\n" << std::setprecision(10);
        for (const auto &point : curve)
            file << point.frequency << ',' << point.response_amplitude << '\n';
    }
} // namespace Chladni
