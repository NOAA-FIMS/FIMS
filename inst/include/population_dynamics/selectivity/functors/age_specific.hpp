/**
 * @file age_specific.hpp
 * @brief Declares the AgeSpecificSelectivity class which age-based selectivity.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_AGE_SPECIFIC_SELECTIVITY_HPP
#define POPULATION_DYNAMICS_AGE_SPECIFIC_SELECTIVITY_HPP

// #include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 *  @brief Age-specific selectivity calculated using one parameter per age bin.
 *
 * AgeSpecificSelectivity estimates a separate selectivity value for each age
 * bin. The values stored in `logit_sel_at_age` are on the logit scale and are
 * transformed to the interval [0, 1]:
 *
 * \f[
 * s_a = \frac{1}{1 + \exp(-logit_sel_at_age)}
 * \f]
 *
 * where \f$\logit_sel_at_age\f$ is the logit-scale parameter for age \f$a\f$
 * and \f$s_a\f$ is the resulting selectivity.
 *
 * The parameter at age \f$a\f$ is selected using the zero-based index
 * \f$a - min\_age\f$. Therefore, `logit_sel_at_age[0]` corresponds to
 * `min_age`, `logit_sel_at_age[1]` corresponds to `min_age + 1`, and so on.
 * The parameter vector should normally contain one element for each age class,
 * with `n_ages` matching the number of modeled ages.
 *
 * Age-specific selectivity is highly flexible because it does not impose a
 * smooth parametric curve across ages. This flexibility can create
 * identifiability problems, especially when estimating both selectivity and
 * fishing mortality. Users should ensure that at least one age-specific
 * selectivity parameter is "constant", i.e., not estimated, and is equal to or
 * near 1, or otherwise impose suitable constraints. This can be achieved by
 * setting at least one age-specific parameter (logit_sel_at_age), which are on
 * the logit scale, to a value greater than 10 with estimation_type ==
 * "constant".
 */
template <typename Type>
struct AgeSpecificSelectivity : public SelectivityBase<Type> {
  /**
   * @brief Stores the age-specific selectivity parameters on the logit scale.
   *
   * Each element corresponds to one modeled age, beginning with `min_age`.
   */
  fims::Vector<Type> logit_sel_at_age;

  /**
   * @brief Stores the number of modeled age bins.
   */
  size_t n_ages;

  /**
   * @brief Minimum modeled age, used to map input age to `logit_sel_at_age`.
   */
  size_t min_age;

  AgeSpecificSelectivity() : SelectivityBase<Type>() {}

  virtual ~AgeSpecificSelectivity() {}

  /**
   * @brief Calculates selectivity for an age using its age-specific parameter.
   *
   * The input age is converted to a zero-based parameter index by subtracting
   * `min_age`. This zero-based index, based on `x`, is then used to select the
   * appropriate element of `logit_sel_at_age`, which is a logit-scale
   * parameter, and it is transformed to a selectivity value between zero and
   * one using `fims_math::inv_logit` with lower and upper bounds of zero and
   * one. Consequently, `logit_sel_at_age[0]` corresponds to `min_age`, and
   * each subsequent element corresponds to the next modeled age.
   *
   * @see fims_math::inv_logit
   * @param x The age at which selectivity is evaluated. The value is expected
   * to be an integer-valued age greater than or equal to `min_age`.
   * @return The selectivity for age `x`.
   * @throws std::invalid_argument if the calculated age index is outside
   * `logit_sel_at_age`.
   */
  virtual const Type evaluate(const Type &x) {
    Type a = static_cast<Type>(0.0);
    Type b = static_cast<Type>(1.0);
    double x_temp = fims_math::Value(x);
    size_t x_final = static_cast<size_t>(x_temp);
    size_t i_age = x_final - min_age;
    return fims_math::inv_logit<Type>(a, b, logit_sel_at_age[i_age]);
  }

  /**
   * @copydoc AgeSpecificSelectivity::evaluate(const Type &x)
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type &x, size_t pos) {
    Type a = static_cast<Type>(0.0);
    Type b = static_cast<Type>(1.0);
    double x_temp = fims_math::Value(x);
    size_t x_final = static_cast<size_t>(x_temp);
    size_t i_age_year = pos * n_ages + x_final - min_age;
    return fims_math::inv_logit<Type>(
        a, b, logit_sel_at_age.get_force_scalar_wrap(i_age_year));
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_AGE_SPECIFIC_SELECTIVITY_HPP */