/**
 * @file lognormal.hpp
 * @brief Math-only lognormal distribution kernel.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_DISTRIBUTIONS_KERNELS_LOGNORMAL_HPP
#define FIMS_DISTRIBUTIONS_KERNELS_LOGNORMAL_HPP

#include "../../common/fims_math.hpp"
#include "normal.hpp"

namespace fims_distributions {
namespace kernels {

/**
 * @brief Lognormal distribution log-density.
 *
 * @details The default log_density() returns the density on the original
 * positive scale and includes the Jacobian adjustment. log_density_log_scale()
 * returns the normal log-density of log(x), matching legacy random-effect
 * behavior that omits the adjustment.
 */
template <typename Type>
struct LogNormal {
  static Type log_density(const Type& x, const Type& meanlog,
                          const Type& sdlog) {
    return log_density_log_scale(x, meanlog, sdlog) - fims_math::log(x);
  }

  static Type log_density_log_scale(const Type& x, const Type& meanlog,
                                    const Type& sdlog) {
    return Normal<Type>::log_density(fims_math::log(x), meanlog, sdlog);
  }
};

}  // namespace kernels
}  // namespace fims_distributions

#endif
