#pragma once

#include <chladni/parameters.h>

#include <deal.II/base/exceptions.h>

#include <cmath>
#include <iostream>
#include <string>

namespace Chladni
{
    /** A coarse, rather thick plate so that the tests run in about a second. */
    inline Parameters small_plate_parameters()
    {
        Parameters parameters;
        parameters.side_length       = 0.24;
        parameters.thickness         = 0.01;
        parameters.bolt_radius       = 0.03;
        parameters.cells_per_side    = 4;
        parameters.polynomial_degree = 2;
        return parameters;
    }

    /** Throw with a readable message if |a - b| > tolerance * |b|. */
    inline void check_relative(const double       a,
                               const double       b,
                               const double       tolerance,
                               const std::string &what)
    {
        const bool close = std::abs(a - b) <= tolerance * std::abs(b);
        std::cout << (close ? "  ok   " : "  FAIL ") << what << ": " << a << " vs " << b
                  << std::endl;
        AssertThrow(close, dealii::ExcMessage(what + " failed"));
    }

    /** Throw with a readable message if @p condition is false. */
    inline void check(const bool condition, const std::string &what)
    {
        std::cout << (condition ? "  ok   " : "  FAIL ") << what << std::endl;
        AssertThrow(condition, dealii::ExcMessage(what + " failed"));
    }
} // namespace Chladni
