/**
 * @file pella_tomlinson.hpp
 * @brief Declares the PellaTomlinsonDepletion class which implements the
 * Pella--Tomlinson function.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_DEPLETION_PELLA_TOMLINSON_HPP
#define POPULATION_DYNAMICS_DEPLETION_PELLA_TOMLINSON_HPP

#include "../../../common/fims_vector.hpp"
#include "../../../common/fims_math.hpp"
#include "depletion_base.hpp"

namespace fims_popdy {

/**
 *  @brief PellaTomlinsonDepletion class that returns the Pella--Tomlinson
 * function.
 */
template <typename Type>
struct PellaTomlinsonDepletion : public DepletionBase<Type> {

  PellaTomlinsonDepletion() : DepletionBase<Type>() {}

  virtual ~PellaTomlinsonDepletion() {}

  /**
   * @brief Method of the depletion class that implements the
   * Pella--Tomlinson production function for depletion, d at time, t.
   *
   * \f[ \frac{growth_rate}{shape - 1.0} * d_{t-1} *  (1.0 - d_{t-1}^{shape - 1.0} - Catch_{t-1} /
   * carrying_capacity) \f]
   *
   * @param depletion_ym1 Expected depletion from previous time step.
   * @param catch_ym1 Catch from previous time step.
   */
  virtual const Type evaluate_mean(const Type& depletion_ym1,
                                   const Type& catch_ym1) {

    return depletion_ym1 +
           (this->growth_rate[0] / (this->shape[0] - 1.0)) * depletion_ym1 *
               (1.0 - fims_math::pow(depletion_ym1, this->shape[0] - 1.0)) -
           catch_ym1 / this->carrying_capacity[0];
  }

  /**
   * @brief Create a map of report vectors for the maturity object.
   */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>>& report_vectors) {
    report_vectors["growth_rate"].emplace_back(this->growth_rate);
    report_vectors["carrying_capacity"].emplace_back(this->carrying_capacity);
    report_vectors["shape"].emplace_back(this->shape);
  }
  virtual void get_report_vector_count(
      std::map<std::string, size_t>& report_vector_count) {
    report_vector_count["growth_rate"] += 1;
    report_vector_count["carrying_capacity"] += 1;
    report_vector_count["shape"] += 1;
  }

};
}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_DEPLETION_PELLA_TOMLINSON_HPP */