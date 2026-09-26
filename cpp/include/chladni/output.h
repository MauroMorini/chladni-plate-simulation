#pragma once

#include <chladni/eigenmode_solver.h>
#include <chladni/forced_response_solver.h>
#include <chladni/plate.h>

#include <deal.II/lac/vector.h>

#include <string>
#include <vector>

namespace Chladni
{
    using namespace dealii;

    /**
     * Write a displacement field to a .vtu file (for ParaView/VisIt). Besides the displacement
     * vector it contains the scalar fields "abs_uz" = |u_z| and "von_mises" (von Mises stress).
     * Where |u_z| is (close to) zero the plate does not move vertically. These are the nodal
     * lines, where the sand gathers.
     */
    void write_vtu(const Plate          &plate,
                   const Vector<double> &displacement,
                   const std::string    &filename);

    /**
     * Write the top face of the plate to a CSV table "x,y,uz,von_mises": the vertical
     * displacement u_z and the von Mises stress, evaluated at the Q_p nodes of the top face.
     *
     * On our uniform mesh these points form a regular grid of (p N + 1) x (p N + 1) points
     * (N cells per side, degree p). Rows are sorted by y, then by x, which makes it easy to
     * reshape the data into 2D arrays, e.g. in numpy.
     */
    void write_top_surface_csv(const Plate          &plate,
                               const Vector<double> &displacement,
                               const std::string    &filename);

    /** Write a CSV table "mode,frequency_hz". */
    void write_eigenfrequencies_csv(const std::vector<Eigenmode> &modes,
                                    const std::string            &filename);

    /** Write a CSV table "frequency_hz,response_amplitude". */
    void write_sweep_csv(const std::vector<SweepPoint> &curve, const std::string &filename);
} // namespace Chladni
