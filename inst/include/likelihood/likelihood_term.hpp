/**
 * @file likelihood_term.hpp
 * @brief Generic likelihood contribution scaffold.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_LIKELIHOOD_LIKELIHOOD_TERM_HPP
#define FIMS_LIKELIHOOD_LIKELIHOOD_TERM_HPP

#include <cstdint>
#include <functional>
#include <string>

#include "../common/fims_vector.hpp"
#include "value_ref.hpp"

namespace fims_likelihood {

/**
 * @brief Semantic category for likelihood reporting and setup.
 */
enum class LikelihoodTermType { Data, Prior, RandomEffect };

/**
 * @brief Generic likelihood term with up to three distribution arguments.
 *
 * @details This is a small composition object. Distribution-specific math is
 * supplied through log_density, while inputs come from ValueRef objects.
 */
template <typename Type>
struct LikelihoodTerm {
  typedef std::function<Type(Type, Type, Type)> LogDensityFunction;

  LikelihoodTermType type = LikelihoodTermType::Data;
  std::string name;
  uint32_t source_id = 0;
  ValueRef<Type> x;
  ValueRef<Type> location;
  ValueRef<Type> scale;
  LogDensityFunction log_density;
  fims::Vector<Type> log_density_values;
  Type log_density_sum = static_cast<Type>(0);

  LikelihoodTerm() {}

  LikelihoodTerm(LikelihoodTermType type, const std::string& name,
                 ValueRef<Type> x, ValueRef<Type> location,
                 ValueRef<Type> scale, LogDensityFunction log_density)
      : type(type),
        name(name),
        x(x),
        location(location),
        scale(scale),
        log_density(log_density) {}

  /**
   * @brief Evaluate and store individual and total log-density contributions.
   */
  Type evaluate() {
    if (!this->log_density) {
      throw std::runtime_error("LikelihoodTerm has no log-density function.");
    }

    size_t n = this->x.size();
    this->log_density_values.resize(n);
    this->log_density_sum = static_cast<Type>(0);

    for (size_t i = 0; i < n; ++i) {
      Type value = this->log_density(
          this->x[i], this->location[this->index_for(this->location, i)],
          this->scale[this->index_for(this->scale, i)]);
      this->log_density_values[i] = value;
      this->log_density_sum += value;
    }
    return this->log_density_sum;
  }

 private:
  size_t index_for(const ValueRef<Type>& ref, size_t i) const {
    return ref.size() == 1 ? 0 : i;
  }
};

}  // namespace fims_likelihood

#endif
