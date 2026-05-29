/**
 * @file parameter.hpp
 * @brief Parameter primitive for likelihood construction.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_LIKELIHOOD_PARAMETER_HPP
#define FIMS_LIKELIHOOD_PARAMETER_HPP

#include <string>

#include "../common/fims_vector.hpp"
#include "transform.hpp"

namespace fims_likelihood {

/**
 * @brief How a parameter participates in estimation.
 */
enum class EstimationRole { Constant, FixedEffect, RandomEffect };

/**
 * @brief Owns raw parameter values and exposes transformed model-scale values.
 *
 * @details The raw vector is the scale used by the optimizer. The value()
 * method is the scale used by model and likelihood code.
 */
template <typename Type>
struct Parameter {
  typedef Type value_type;

  std::string name;
  fims::Vector<Type> raw;
  Transform transform = Transform::Identity;
  EstimationRole role = EstimationRole::Constant;
  Type lower_bound = static_cast<Type>(0);
  Type upper_bound = static_cast<Type>(1);

  Parameter() {}

  Parameter(const std::string& name, const fims::Vector<Type>& raw,
            Transform transform = Transform::Identity,
            EstimationRole role = EstimationRole::Constant)
      : name(name), raw(raw), transform(transform), role(role) {}

  /**
   * @brief Number of raw parameter values.
   */
  size_t size() const { return this->raw.size(); }

  /**
   * @brief Return a transformed value, treating length-one vectors as scalars.
   */
  Type value(size_t i = 0) const {
    size_t index = this->raw.size() == 1 ? 0 : i;
    return apply_transform(this->raw[index], this->transform,
                           this->lower_bound, this->upper_bound);
  }

  /**
   * @brief Return the log absolute Jacobian for the transformed value.
   */
  Type log_jacobian(size_t i = 0) const {
    size_t index = this->raw.size() == 1 ? 0 : i;
    return fims_likelihood::log_jacobian(
        this->raw[index], this->transform, this->lower_bound,
        this->upper_bound);
  }
};

}  // namespace fims_likelihood

#endif
