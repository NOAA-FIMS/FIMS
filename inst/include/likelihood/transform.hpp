/**
 * @file transform.hpp
 * @brief Parameter transformation helpers for likelihood construction.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_LIKELIHOOD_TRANSFORM_HPP
#define FIMS_LIKELIHOOD_TRANSFORM_HPP

#include "../common/fims_math.hpp"

namespace fims_likelihood {

/**
 * @brief Supported transformations from the estimated scale to model scale.
 */
enum class Transform { Identity, Log, Logit };

/**
 * @brief Apply a transformation from the estimated scale to model scale.
 *
 * @param x Value on the estimated scale.
 * @param transform Transformation type.
 * @param lower Lower bound for bounded transforms.
 * @param upper Upper bound for bounded transforms.
 * @return Value on the model scale.
 */
template <typename Type>
inline Type apply_transform(const Type& x, Transform transform,
                            const Type& lower = static_cast<Type>(0),
                            const Type& upper = static_cast<Type>(1)) {
  switch (transform) {
    case Transform::Identity:
      return x;
    case Transform::Log:
      return fims_math::exp(x);
    case Transform::Logit:
      return fims_math::inv_logit(lower, upper, x);
  }
  return x;
}

/**
 * @brief Log absolute Jacobian adjustment for a transformation.
 *
 * @details This is useful for Bayesian workflows that need the probability
 * density adjusted when priors are specified on the transformed/model scale.
 *
 * @param x Value on the estimated scale.
 * @param transform Transformation type.
 * @param lower Lower bound for bounded transforms.
 * @param upper Upper bound for bounded transforms.
 * @return Log absolute derivative of model-scale value with respect to x.
 */
template <typename Type>
inline Type log_jacobian(const Type& x, Transform transform,
                         const Type& lower = static_cast<Type>(0),
                         const Type& upper = static_cast<Type>(1)) {
  switch (transform) {
    case Transform::Identity:
      return static_cast<Type>(0);
    case Transform::Log:
      return x;
    case Transform::Logit: {
      Type y = apply_transform(x, transform, lower, upper);
      return fims_math::log(y - lower) + fims_math::log(upper - y) -
             fims_math::log(upper - lower);
    }
  }
  return static_cast<Type>(0);
}

}  // namespace fims_likelihood

#endif
