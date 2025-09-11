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
   * \f[ \frac{r}{m - 1.0} * d_{t-1} *  (1.0 - d_{t-1}^{m - 1.0} - Catch_{t-1} /
   * K \f]
   *
   * @param depletion_ym1 Expected depletion from previous time step.
   * @param catch_ym1 Catch from previous time step.
   */
  virtual const Type evaluate_mean(const Type& depletion_ym1,
                                   const Type& catch_ym1) {
    Type r;
    Type K;
    Type m;

    r = fims_math::exp(this->log_r[0]);
    K = fims_math::exp(this->log_K[0]);
    m = fims_math::exp(this->log_m[0]);

    return depletion_ym1 +
           (r / (m - 1.0)) * depletion_ym1 *
               (1.0 - fims_math::pow(depletion_ym1, m - 1.0)) -
           catch_ym1 / K;
  }
};
}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_DEPLETION_PELLA_TOMLINSON_HPP */