/**
 * @file multinomial.hpp
 * @brief Math-only multinomial distribution kernel.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_DISTRIBUTIONS_KERNELS_MULTINOMIAL_HPP
#define FIMS_DISTRIBUTIONS_KERNELS_MULTINOMIAL_HPP

#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"

namespace fims_distributions {
namespace kernels {

/**
 * @brief Multinomial distribution log-probability mass.
 *
 * @details This kernel intentionally matches FIMS' legacy TMB-style
 * multinomial convention: observations are not rounded before evaluating the
 * log PMF.
 */
template <typename Type>
struct Multinomial {
  static Type log_density(const fims::Vector<Type>& x,
                          const fims::Vector<Type>& prob) {
    Type total = static_cast<Type>(0.0);
    Type log_factorial_sum = static_cast<Type>(0.0);
    Type weighted_log_prob = static_cast<Type>(0.0);

    for (size_t i = 0; i < x.size(); i++) {
      total += x[i];
      log_factorial_sum += fims_math::lgamma(x[i] + static_cast<Type>(1.0));
      weighted_log_prob += x[i] * fims_math::log(prob[i]);
    }

    return fims_math::lgamma(total + static_cast<Type>(1.0)) -
           log_factorial_sum + weighted_log_prob;
  }
};

}  // namespace kernels
}  // namespace fims_distributions

#endif
