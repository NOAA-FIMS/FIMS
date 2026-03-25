/**
 * @file sel_at_age.hpp
 * @brief Declares the SelectivityatAge class which age-based selectivity.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_AT_AGE_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_AT_AGE_HPP

// #include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 *  @brief SelectivityatAge class that returns the logistic function value
 * from fims_math.
 *
 * The logistic selectivity function can produce either an ascending or
 * descending curve based on the sign of the slope parameter. A positive slope
 * creates an ascending logistic curve (selectivity increases from 0 to 1 with
 * increasing x), while a negative slope creates a descending logistic curve
 * (selectivity decreases from 1 to 0 with increasing x).
 */
template <typename Type>
struct SelectivityatAge : public SelectivityBase<Type> {
  fims::Vector<Type>
      logit_sel_at_age;     // update definition

  LogisticSelectivity() : SelectivityBase<Type>() {}

  virtual ~LogisticSelectivity() {}

  /**
   * @brief Method of the logistic selectivity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection\_point))} \f]
   *
   * The selectivity curve can be either ascending or descending depending on
   * the sign of the slope parameter:
   * - Positive slope: ascending curve (selectivity increases from 0 to 1)
   * - Negative slope: descending curve (selectivity decreases from 1 to 0)
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual const Type evaluate(const Type &x) {
    //return fims_math::inv_logit<Type>(logit_sel_at_age[x]); // AJ: figure out default rules for x (always starts at 1? guaranteed to run through all n_ages)
      //AJ: we can add conditionals in case x
      //AJ: run past Kelli/Nathan/Ian
    return fims_math::logistic<Type>(inflection_point[0], slope[0], x);
  }

  /**
   * @copydoc LogisticSelectivity::evaluate(const Type &x)
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type &x, size_t pos) {
    //formula for i_age_year
    size_t i_age_year = pos * this->n_ages + x; // might need adjustment if x starts at 1
    //return fims_math::inv_logit<Type>(logit_sel_at_age[i_age_year]);
    return fims_math::logistic<Type>(inflection_point.get_force_scalar(pos),
                                     slope.get_force_scalar(pos), x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP */