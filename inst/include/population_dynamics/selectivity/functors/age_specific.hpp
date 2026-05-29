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
 *  @brief AgeSpecificSelectivity class that returns the inverse logit function value
 * from fims_math.
 *
 * Age-specific selectivity allows users to estimate age-specific selectivity values,
 * with great inherent flexibility. The number of parameters (either estimated
 * or fixed) is equal to the number of ages. Users are recommended to fix selectivity
 * for at least one age to a value equal or close to 1.
 */
template <typename Type>
struct AgeSpecificSelectivity : public SelectivityBase<Type> {
  fims::Vector<Type>
      logit_sel_at_age;     
  size_t n_ages; // AJ: is this the best way to provide n_ages?
  size_t min_age; // AJ: need to create/pass this value / correct specification?
  //size_t n_years; // AJ: year-implementation
  //std::map<int, size_t> age_index;
  //std::map<size_t, std::map<int, size_t>>index_map; // AJ: might remove

  AgeSpecificSelectivity() : SelectivityBase<Type>() {}

  virtual ~AgeSpecificSelectivity() {}

  /**
   * @brief Method of the age-specific selectivity class that implements the
   * inverse logit function from FIMS math to properly transform parameter
   * values.
   *
   * a + (b - a) / (static_cast<Type>(1.0) + fims_math::exp(-logit_x))
   *
   *
   * @param x  The independent variable in the age-specific selectivity function 
   * (e.g., age).
   */
  virtual const Type evaluate(const Type &x) {
    Type a = static_cast<Type>(0.0);
    Type b = static_cast<Type>(1.0);
    double x_temp = fims_math::Value(x);
    size_t x_final = static_cast<size_t>(x_temp);
    //for(size_t i = 0; i < ages.size(); i++) { //Matthew Supernaw code
    //  age_index[ages[i]] = i;
    //}
    // use Value() function to convert Type &x to double, then convert double to integer using static_cast
    size_t i_age = x_final - min_age;
    return fims_math::inv_logit<Type>(a, b, logit_sel_at_age[i_age]); // AJ: figure out default rules for x (always starts at 1? guaranteed to run through all n_ages)
      //AJ: we can add conditionals in case x doesn't start at age-0
        //AJ: is x obtained by calling get_ages(data) or something similar?
        //AJ: how can we obtain the minimum of possible x's, to scale x vector to start at zero
      //AJ: run past Kelli/Nathan/Ian
  }

  /**
   * @copydoc AgeSpecificSelectivity::evaluate(const Type &x)
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type &x, size_t pos) {
    //formula for i_age_year
    //size_t i_age_year = pos * this->n_ages + x; // might need adjustment if x starts at 1 // EML: Is x just an additional time-varying "offset" to mean selectivity at age estimates?
    // does pos always start at 0, so that we apply the index correctly
    Type a = static_cast<Type>(0.0);
    Type b = static_cast<Type>(1.0);
    double x_temp = fims_math::Value(x);
    size_t x_final = static_cast<size_t>(x_temp);
    size_t i_age_year = pos * n_ages + x_final - min_age;
    return fims_math::inv_logit<Type>(a, b, logit_sel_at_age.get_force_scalar_wrap(i_age_year)); 

    //size_t i_age_year = x - this->min_age;
    //return fims_math::inv_logit<Type>(a, b, this->logit_sel_at_age[i_age_year]);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_AGE_SPECIFIC_SELECTIVITY_HPP */