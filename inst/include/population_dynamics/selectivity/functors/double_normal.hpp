/**
 * @file double_normal.hpp
 * @brief Declares the DoubleNormalSelectivity class which implements the
 * double normal selectivity function in Methot and Wetzel (2013).
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_NORMAL_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_NORMAL_HPP

// #include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
// #include "../../../common/information.hpp"//Added in case I can run GetNages() here
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 * @brief DoubleNormalSelectivity class that constructs and runs the double normal
 * function, defined below.
 */
template <typename Type>
struct DoubleNormalSelectivity : public SelectivityBase<Type> {
  //size_t nages; // not sure if I need to supply nages/max_age here for Option B/C
  fims::Vector<Type> age_peak_sel_start; /**< age at which selectivity=1
            starts, or p1 */
  fims::Vector<Type> width_peak_sel; /**< width of "top" in which selectivity=1, 
            or p2; determines the age at which selectivity=1 ends */
  fims::Vector<Type> slope_asc; /**< slope of the ascending section, or p3;
            ?: Is it problematic to use the same param name as dbl logistic? */
  fims::Vector<Type> slope_desc; /**< slope of the descending seciton, or p4;
            ?: Is it problematic to use the same param name as dbl logistic? */
  fims::Vector<Type> sel_age_zero_logit; /** selectivity at age0 (parameterized
            in logit space), or p5 */
  fims::Vector<Type> sel_age_A_logit; /** selectivity at age A 
            (parameterized in logit space), or p6 */

  DoubleNormalSelectivity() : SelectivityBase<Type>() {}

  virtual ~DoubleNormalSelectivity() {}

  /**
   * @brief Method of the double normal class that implements the
   * double normal function, provided below.
   *
   * UPDATE
   *
   * @param x  The independent variable in the double normal function (e.g.,
   * age or size in selectivity).
   */
  virtual const Type evaluate(const Type &x) {
    // this->nages = nages; // one way to assign value to nages (borrow from fleet.hpp syntax)?
      // currently, I hope nages gets read from rcpp_selectivity.hpp
    return fims_math::double_normal<Type>(
        //nages, //Option B
        //max_age, //Option C
        age_peak_sel_start[0], width_peak_sel[0], slope_asc[0],
        slope_desc[0], sel_age_zero_logit[0], sel_age_A_logit[0], x);
  }

  /**
   * @brief Method of the double normal selectivity class that implements the
   * double normal function, provided below.
   *
   * UPDATE
   *
   * @param x  The independent variable in the double normal function (e.g.,
   * age or size in selectivity).
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type &x,
                              size_t pos) {
    return fims_math::double_normal<Type>(
      // nages, //Option B
      // max_age, //Option C
      age_peak_sel_start.get_force_scalar(pos),
      width_peak_sel.get_force_scalar(pos),
      slope_asc.get_force_scalar(pos),
      slope_desc.get_force_scalar(pos), 
      sel_age_zero_logit.get_force_scalar(pos), 
      sel_age_A_logit.get_force_scalar(pos), x);
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_DOUBLE_NORMAL_HPP */
