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
     * vector it contains the scalar field "abs_uz" = |u_z|: where it is (close to) zero the
     * plate does not move vertically. These are the nodal lines, where the sand gathers.
     */
    void write_vtu(const Plate          &plate,
                   const Vector<double> &displacement,
                   const std::string    &filename);

    /** Write a CSV table "mode,frequency_hz". */
    void write_eigenfrequencies_csv(const std::vector<Eigenmode> &modes,
                                    const std::string            &filename);

    /** Write a CSV table "frequency_hz,response_amplitude". */
    void write_sweep_csv(const std::vector<SweepPoint> &curve, const std::string &filename);
} // namespace Chladni
