/**
 * @file normal.hpp
 * @brief Math-only normal distribution kernel.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_DISTRIBUTIONS_KERNELS_NORMAL_HPP
#define FIMS_DISTRIBUTIONS_KERNELS_NORMAL_HPP

#include "../../common/fims_math.hpp"

namespace fims_distributions {
namespace kernels {

/**
 * @brief Normal distribution log-density.
 *
 * @details This kernel intentionally owns only distribution math. It does not
 * know whether x is data, a prior target, or a random effect.
 */
template <typename Type>
struct Normal {
  static Type log_density(const Type& x, const Type& mean, const Type& sd) {
    const Type pi = static_cast<Type>(3.14159265358979323846264338327950288);
    Type z = (x - mean) / sd;
    return -static_cast<Type>(0.5) *
               fims_math::log(static_cast<Type>(2.0) * pi) -
           fims_math::log(sd) - static_cast<Type>(0.5) * z * z;
  }
};

}  // namespace kernels
}  // namespace fims_distributions

#endif
