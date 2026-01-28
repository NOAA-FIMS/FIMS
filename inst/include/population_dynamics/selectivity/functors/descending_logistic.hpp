/**
 * @file descending_logistic.hpp
 * @brief Declares the DescendingLogisticSelectivity class which implements the
 * descending logistic function in the selectivity module.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_DESCENDING_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_DESCENDING_LOGISTIC_HPP

// #include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 * @brief DescendingLogisticSelectivity class that returns the Descending
 * logistic function value from fims_math.
 */
template <typename Type>
struct DescendingLogisticSelectivity : public SelectivityBase<Type> {
  fims::Vector<Type>
      inflection_point_desc;     /**< 50% quantile of the value of
     the quantity of interest (x) on the descending limb of the Descending
     logistic curve; e.g. age at which 50% of the fish are selected */
  fims::Vector<Type> slope_desc; /**<scalar multiplier of difference between
   quantity of interest  value (x) and inflection_point on the
   descending limb of the Descending  logistic  curve */

  DescendingLogisticSelectivity() : SelectivityBase<Type>() {}

  virtual ~DescendingLogisticSelectivity() {}

  /**
   * @brief Method of the Descending logistic selectivity class that implements
   * the Descending logistic function from FIMS math.
   *
   * \f$\left(1.0-\frac{1.0}{ 1.0 + exp(-1.0 * slope\_desc (x -
   * inflection_point\_desc))} \right)\f$
   *
   * @param x  The independent variable in the descending logistic function
   * (e.g., age or size in selectivity).
   */
  virtual const Type evaluate(const Type& x) {
    return 1 - fims_math::logistic<Type>(inflection_point_desc[0],
                                         slope_desc[0], x);
  }

  /**
   * @brief Method of the descending logistic selectivity class that implements
   * the descending logistic function from FIMS math.
   *
   * \f$ \left(1.0-\frac{1.0}{ 1.0 + exp(-1.0 *
   * slope\_desc (x - inflection\_point\_desc))} \right) \f$
   *
   * @param x  The independent variable in the descending logistic function
   * (e.g., age or size in selectivity).
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate(const Type& x, size_t pos) {
    return 1.0 - fims_math::logistic<Type>(
                     inflection_point_desc.get_force_scalar(pos),
                     slope_desc.get_force_scalar(pos), x);
  }

  /**
   * @brief Create a map of report vectors for the selectivity object.
   */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>>& report_vectors) {
    report_vectors["inflection_point_desc"].emplace_back(
        inflection_point_desc.to_tmb());
    report_vectors["slope_desc"].emplace_back(slope_desc.to_tmb());
  }

  virtual void get_report_vector_count(
      std::map<std::string, size_t>& report_vector_count) {
    report_vector_count["inflection_point_desc"] += 1;
    report_vector_count["slope_desc"] += 1;
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_DESCENDING_LOGISTIC_HPP */
