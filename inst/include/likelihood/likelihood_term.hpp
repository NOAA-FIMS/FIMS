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
#include <memory>
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
  typedef std::function<Type(const fims::Vector<Type>&,
                             const fims::Vector<Type>&)>
      VectorLogDensityFunction;
  typedef std::function<bool(size_t)> IncludeFunction;

  LikelihoodTermType type = LikelihoodTermType::Data;
  std::string name;
  uint32_t source_id = 0;
  ValueRef<Type> x;
  ValueRef<Type> location;
  ValueRef<Type> scale;
  LogDensityFunction log_density;
  VectorLogDensityFunction vector_log_density;
  IncludeFunction include;
  size_t row_size = 0;
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
    if (this->vector_log_density) {
      return this->evaluate_vector_density();
    }

    if (!this->log_density) {
      throw std::runtime_error("LikelihoodTerm has no log-density function.");
    }

    size_t n = this->x.size();
    this->log_density_values.resize(n);
    this->log_density_sum = static_cast<Type>(0);

    for (size_t i = 0; i < n; ++i) {
      if (this->include && !this->include(i)) {
        this->log_density_values[i] = static_cast<Type>(0);
        continue;
      }
      Type value = this->log_density(
          this->x[i], this->location[this->index_for(this->location, i)],
          this->scale[this->index_for(this->scale, i)]);
      this->log_density_values[i] = value;
      this->log_density_sum += value;
    }
    return this->log_density_sum;
  }

 private:
  Type evaluate_vector_density() {
    if (this->row_size == 0) {
      throw std::runtime_error("LikelihoodTerm row_size is zero.");
    }
    if (this->x.size() % this->row_size != 0) {
      throw std::runtime_error(
          "LikelihoodTerm row_size does not divide observed values.");
    }

    size_t n = this->x.size();
    this->log_density_values.resize(n);
    this->log_density_sum = static_cast<Type>(0);

    for (size_t row_start = 0; row_start < n; row_start += this->row_size) {
      bool include_row = true;
      for (size_t j = 0; j < this->row_size; ++j) {
        if (this->include && !this->include(row_start + j)) {
          include_row = false;
          break;
        }
      }

      if (!include_row) {
        for (size_t j = 0; j < this->row_size; ++j) {
          this->log_density_values[row_start + j] = static_cast<Type>(0);
        }
        continue;
      }

      fims::Vector<Type> x_row(this->row_size);
      fims::Vector<Type> location_row(this->row_size);
      for (size_t j = 0; j < this->row_size; ++j) {
        size_t i = row_start + j;
        x_row[j] = this->x[i];
        location_row[j] =
            this->location[this->index_for(this->location, i)];
      }

      Type value = this->vector_log_density(x_row, location_row);
      for (size_t j = 0; j < this->row_size; ++j) {
        this->log_density_values[row_start + j] = value;
      }
      this->log_density_sum += value;
    }

    return this->log_density_sum;
  }

  size_t index_for(const ValueRef<Type>& ref, size_t i) const {
    return ref.size() == 1 ? 0 : i;
  }
};

/**
 * @brief Build a scalar likelihood term with named helper semantics.
 */
template <typename Type>
inline std::shared_ptr<LikelihoodTerm<Type>> make_likelihood_term(
    LikelihoodTermType type, const std::string& name, ValueRef<Type> x,
    ValueRef<Type> location, ValueRef<Type> scale,
    typename LikelihoodTerm<Type>::LogDensityFunction log_density,
    typename LikelihoodTerm<Type>::IncludeFunction include = nullptr,
    uint32_t source_id = 0) {
  std::shared_ptr<LikelihoodTerm<Type>> term =
      std::make_shared<LikelihoodTerm<Type>>(type, name, x, location, scale,
                                             log_density);
  term->source_id = source_id;
  term->include = include;
  return term;
}

/**
 * @brief Build a row-wise likelihood term with named helper semantics.
 */
template <typename Type>
inline std::shared_ptr<LikelihoodTerm<Type>> make_likelihood_term(
    LikelihoodTermType type, const std::string& name, ValueRef<Type> x,
    ValueRef<Type> location, size_t row_size,
    typename LikelihoodTerm<Type>::VectorLogDensityFunction log_density,
    typename LikelihoodTerm<Type>::IncludeFunction include = nullptr,
    uint32_t source_id = 0) {
  std::shared_ptr<LikelihoodTerm<Type>> term =
      std::make_shared<LikelihoodTerm<Type>>();
  term->type = type;
  term->name = name;
  term->source_id = source_id;
  term->x = x;
  term->location = location;
  term->row_size = row_size;
  term->vector_log_density = log_density;
  term->include = include;
  return term;
}

}  // namespace fims_likelihood

#endif
