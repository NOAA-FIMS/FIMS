/**
 * @file empirical.hpp
 * @brief Defines the EmpiricalMaturity class, which inherits from the
 * MaturityBase class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_MATURITY_EMPIRICAL_HPP
#define POPULATION_DYNAMICS_MATURITY_EMPIRICAL_HPP

#include <stdexcept>
#include <string>

#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"
#include "maturity_base.hpp"

namespace fims_popdy {

/**
 * @brief EmpiricalMaturity class that returns a user-supplied proportion
 * mature at age, optionally by year.
 *
 * The values in `maturity_at_age` are proportions mature and are used as
 * given, without a transformation. The value for age \f$a\f$ is found with the
 * zero-based index \f$a - min\_age\f$, so `maturity_at_age[0]` corresponds to
 * `min_age`.
 *
 * `maturity_at_age` can hold 1 value for all ages and years, `n_ages` values
 * used in every year, or `n_ages` values per year with age changing fastest
 * within each year.
 */
template <typename Type>
struct EmpiricalMaturity : public MaturityBase<Type> {
  /**
   * @brief Proportion mature at age, optionally by year.
   */
  fims::Vector<Type> maturity_at_age;

  /**
   * @brief The number of modeled ages.
   */
  size_t n_ages = 0;

  /**
   * @brief The minimum modeled age, used to map an age to `maturity_at_age`.
   */
  size_t min_age = 0;

  EmpiricalMaturity() : MaturityBase<Type>() {}

  virtual ~EmpiricalMaturity() {}

  /**
   * @brief Returns the proportion mature at age when maturity does not vary
   * by year.
   *
   * @param x The age at which maturity is evaluated.
   * @return The proportion mature at age `x`.
   * @throws std::invalid_argument if `maturity_at_age` varies by year, because
   * a year is needed to choose among the values.
   */
  virtual const Type evaluate(const Type& x) {
    // Without a year, time-varying values would silently give every year the
    // first year's maturity.
    if (maturity_at_age.size() > n_ages) {
      throw std::invalid_argument(
          "EmpiricalMaturity: maturity_at_age varies by year, so evaluate() "
          "needs a year. Use evaluate(x, pos).");
    }
    return evaluate(x, 0);
  }

  /**
   * @brief Returns the proportion mature at age in a year.
   *
   * @param x The age at which maturity is evaluated.
   * @param pos Position index, e.g., which year.
   * @return The proportion mature at age `x` in year `pos`.
   * @throws std::invalid_argument if `x` is outside the modeled ages, if the
   * length of `maturity_at_age` is not 1 or a multiple of `n_ages`, or if
   * `pos` is beyond the years in `maturity_at_age`.
   */
  virtual const Type evaluate(const Type& x, size_t pos) {
    size_t n_values = maturity_at_age.size();
    if (n_values == 1) {
      return maturity_at_age[0];
    }
    if (n_ages == 0 || n_values % n_ages != 0) {
      throw std::invalid_argument(
          "EmpiricalMaturity: maturity_at_age has " + std::to_string(n_values) +
          " values, which is not 1 or a multiple of n_ages (" +
          std::to_string(n_ages) + ").");
    }

    double age = fims_math::Value(x);
    if (age < static_cast<double>(min_age) ||
        age >= static_cast<double>(min_age + n_ages)) {
      throw std::invalid_argument("EmpiricalMaturity: age " +
                                  std::to_string(age) +
                                  " is outside the modeled ages.");
    }
    size_t i_age = static_cast<size_t>(age) - min_age;

    size_t i_value = (n_values == n_ages) ? i_age : pos * n_ages + i_age;
    if (i_value >= n_values) {
      throw std::invalid_argument("EmpiricalMaturity: year index " +
                                  std::to_string(pos) + " is beyond the " +
                                  std::to_string(n_values / n_ages) +
                                  " years in maturity_at_age.");
    }
    return maturity_at_age[i_value];
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_MATURITY_EMPIRICAL_HPP */
