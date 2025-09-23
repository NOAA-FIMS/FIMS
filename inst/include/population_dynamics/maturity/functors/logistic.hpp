/**
 * @file logistic.hpp
 * @brief Defines the LogisticMaturity class, which inherits from the
 * MaturityBase class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 *
 */
#ifndef POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP

#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "maturity_base.hpp"

namespace fims_popdy {

/**
 *  @brief LogisticMaturity class that returns the logistic function value
 * from fims_math.
 */
template <typename Type>
struct LogisticMaturity : public MaturityBase<Type> {
  fims::Vector<Type>
      inflection_point; /**< 50 percent quantile of the value of the quantity of
interest (x); e.g. age at which 50 percent of the fish are mature */
  fims::Vector<Type> slope; /**<scalar multiplier of difference between quantity
            of interest value (x) and inflection_point */

  LogisticMaturity() : MaturityBase<Type>() {}

  /**
   * @brief Method of the logistic maturity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection_point))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size at maturity).
   */

  virtual const Type evaluate(const Type& x) {
    return fims_math::logistic<Type>(inflection_point[0], slope[0], x);
  }

  /**
   * @brief Method of the logistic maturity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope_t (x - {inflection\_point}_t))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type& x, size_t pos) {
    return fims_math::logistic<Type>(inflection_point.get_force_scalar(pos),
                                     slope.get_force_scalar(pos), x);
  }

  /**
   * @brief Create a map of report vectors for the maturity object.
   */
   virtual void create_report_vectors(
    std::map<std::string, fims::Vector<fims::Vector<Type>> >& report_vectors){
    report_vectors["inflection_point"].emplace_back(inflection_point);
    report_vectors["slope"].emplace_back(slope);
  }
  virtual void get_report_vector_count(
    std::map<std::string, size_t>& report_vector_count){
      report_vector_count["inflection_point"] += 1;
      report_vector_count["slope"] += 1;
    }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_MATURITY_LOGISTIC_HPP */
