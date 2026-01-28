/**
 * @file logistic.hpp
 * @brief Declares the LogisticSelectivity class which implements the logistic
 * function from fims_math in the selectivity module.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP
#pragma once
#include <map>
#include "selectivity_base.hpp"

// forward declaration to avoid circular includes
namespace fims {
template <typename Type> class Vector;
} // namespace fims

namespace fims_popdy {
// forward declaration to avoid circular includes
// template <typename Type> class SelectivityBase;

/**
 *  @brief LogisticSelectivity class that returns the logistic function value
 * from fims_math.
 */
template <typename Type>
struct LogisticSelectivity : public SelectivityBase<Type> {
  fims::Vector<Type>
      inflection_point;     /**< 50% quantile of the value of the quantity of
  interest (x); e.g. age at which 50% of the fish are selected */
  fims::Vector<Type> slope; /**<scalar multiplier of difference between quantity
            of interest value (x) and inflection_point */

  LogisticSelectivity();
  virtual ~LogisticSelectivity();

  /**
   * @brief Method of the logistic selectivity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection\_point))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual const Type evaluate(const Type &x);
  /**
   * @brief Method of the logistic selectivity class that implements the
   * logistic function from FIMS math.
   *
   * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope_t (x - {inflection\_point}_t))} \f]
   *
   * @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type &x, size_t pos);
  /**<brief Create report vectors for the logistic selectivity */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>> &report_vectors);
  /**<brief Get the number of report vectors for the logistic selectivity */
  virtual void
  get_report_vector_count(std::map<std::string, size_t> &report_vector_count);
};

} // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP */
