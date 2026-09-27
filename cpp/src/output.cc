#include <chladni/output.h>

#include <chladni/stress.h>

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_values_extractors.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/data_postprocessor.h>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <utility>

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

        /** Computes the von Mises stress from the displacement gradient at every output point. */
        class VonMisesStress : public DataPostprocessorScalar<dim>
        {
        public:
            VonMisesStress(const double lambda, const double mu)
                : DataPostprocessorScalar<dim>("von_mises", update_gradients)
                , lambda(lambda)
                , mu(mu)
            {}

            void evaluate_vector_field(const DataPostprocessorInputs::Vector<dim> &inputs,
                                       std::vector<Vector<double>> &computed_quantities) const override
            {
                for (unsigned int p = 0; p < inputs.solution_gradients.size(); ++p) {
                    Tensor<2, dim> gradient;
                    for (unsigned int i = 0; i < dim; ++i)
                        gradient[i] = inputs.solution_gradients[p][i];
                    computed_quantities[p](0) = von_mises_stress(gradient, lambda, mu);
                }
            }

        private:
            const double lambda, mu;
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
        const VonMisesStress von_mises(plate.get_parameters().lame_lambda(),
                                       plate.get_parameters().lame_mu());

        DataOut<dim> data_out;
        data_out.attach_dof_handler(plate.get_dof_handler());
        data_out.add_data_vector(displacement,
                                 names,
                                 DataOut<dim>::type_dof_data,
                                 interpretation);
        data_out.add_data_vector(displacement, abs_uz);
        data_out.add_data_vector(displacement, von_mises);

        // Subdivide each cell so that the higher-order solution is shown accurately.
        const unsigned int degree = plate.get_dof_handler().get_fe().degree;
        data_out.build_patches(plate.get_mapping(), degree);

        std::ofstream file(filename);
        data_out.write_vtu(file);
    }

    void write_top_surface_csv(const Plate          &plate,
                               const Vector<double> &displacement,
                               const std::string    &filename)
    {
        const Parameters &parameters  = plate.get_parameters();
        const auto       &dof_handler = plate.get_dof_handler();
        const auto       &fe          = dof_handler.get_fe();

        // On every face of the top surface we evaluate the solution at the (p+1)^2 Gauss-Lobatto
        // points. These are exactly the nodes of the Q_p elements, so all points together form
        // a regular grid over the plate.
        const QGaussLobatto<dim - 1> face_points(fe.degree + 1);
        FEFaceValues<dim>            fe_face_values(plate.get_mapping(),
                                         fe,
                                         face_points,
                                         update_values | update_gradients |
                                             update_quadrature_points);
        const FEValuesExtractors::Vector displacement_extractor(0);

        std::vector<Tensor<1, dim>> values(face_points.size());
        std::vector<Tensor<2, dim>> gradients(face_points.size());

        // A node is shared by up to 4 top faces. u_z is continuous, so every face gives the same
        // value. The stress is computed from gradients, which jump between cells, so we average
        // it over the faces that share the node.
        struct TopPoint
        {
            double       x = 0., y = 0., uz = 0.;
            double       von_mises_sum = 0.;
            unsigned int n_faces       = 0;
        };
        // Key: (y, x) rounded to integers. std::map keeps its keys sorted, so the points come
        // out sorted by y, then by x.
        std::map<std::pair<long, long>, TopPoint> points;
        const double                              resolution = 1e-9;

        const double tolerance = 1e-10 * parameters.side_length;
        for (const auto &cell : dof_handler.active_cell_iterators())
            for (const auto &face : cell->face_iterators()) {
                const bool is_top_face = face->at_boundary() &&
                                         std::abs(face->center()[2] - parameters.thickness) <
                                             tolerance;
                if (!is_top_face)
                    continue;

                fe_face_values.reinit(cell, face);
                fe_face_values[displacement_extractor].get_function_values(displacement, values);
                fe_face_values[displacement_extractor].get_function_gradients(displacement,
                                                                              gradients);

                for (const unsigned int q : fe_face_values.quadrature_point_indices()) {
                    const Point<dim> &p   = fe_face_values.quadrature_point(q);
                    TopPoint         &top = points[{std::lround(p[1] / resolution),
                                                    std::lround(p[0] / resolution)}];
                    top.x  = p[0];
                    top.y  = p[1];
                    top.uz = values[q][2];
                    top.von_mises_sum += von_mises_stress(gradients[q],
                                                          parameters.lame_lambda(),
                                                          parameters.lame_mu());
                    ++top.n_faces;
                }
            }

        std::ofstream file(filename);
        file << "x,y,uz,von_mises\n" << std::setprecision(10);
        for (const auto &[key, top] : points)
            file << top.x << ',' << top.y << ',' << top.uz << ','
                 << top.von_mises_sum / top.n_faces << '\n';
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
