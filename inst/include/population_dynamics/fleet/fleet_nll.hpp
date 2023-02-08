/*! \file fleet_nll.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Fleet nll file
 * The purpose of this file is to serve as the class for fleet negative
 * log likelihood.
 *
 * DEFINE guards for fleet nll outline to define the
 * fleet hpp file if not already defined.
 */
#ifndef FIMS_POPULATION_DYNAMICS_FLEET_NLL_HPP
#define FIMS_POPULATION_DYNAMICS_FLEET_NLL_HPP

#include "../../common/model_object.hpp"
#include "../../distributions/distributions.hpp"
#include "fleet.hpp"

namespace fims {
#ifdef TMB_MODEL

/** @brief fleet negative log-likelihood (nll).
 *
 * @tparam Type The type of the fleet functor.
 *
 */
template <class Type>
struct FleetIndexNLL : public Fleet<Type> {
  static uint32_t id_g; /*!< reference id for fleet nll object*/

  /** @brief Constructor.
   */
  FleetIndexNLL() { this->id = FleetIndexNLL::id_g++; }

  virtual ~FleetIndexNLL() {}

  /** @brief likelihood component function.
   * Returns the negative log likelihood (nll).
   * Based on equation (A.3.10) in Methot and Wetzel (2013)
   * but with the addition of the constant terms.
   */
  Type evaluate() {
    Type nll = 0.0; /*!< The negative log likelihood value */

      fims::Dnorm<Type> dnorm;
      dnorm.sd = std::exp(this->log_obs_error);
      for (size_t i = 0; i < this->expected_index.size(); i++) {
        dnorm.x = std::log(this->observed_index_data[i]);
        dnorm.mean = std::log(this->expected_index[i]);
        nll -= dnorm.evaluate(true);
      }
      return nll;
    }
  };

/** @brief fleet negative log-likelihood (nll).
 *
 * @tparam Type The type of the fleet functor.
 *
 */
template <class Type>
struct FleetAgeCompNLL : public Fleet<Type> {
  static uint32_t id_g; /*!< reference id for fleet nll object*/

  /** @brief Constructor.
   */
  FleetAgeCompNLL() { this->id = FleetAgeCompNLL::id_g++; }

  virtual ~FleetAgeCompNLL() {}

  /** @brief likelihood component function.
   * Returns the negative log likelihood (nll).
   * Based on equation (A.3.10) in Methot and Wetzel (2013)
   * but with the addition of the constant terms.
   */
  Type evaluate() {
    Type nll = 0.0; /*!< The negative log likelihood value */

      fims::Dmultinom<Type> dmultinom;
      if(this->oberved_agecomp_data.size() != this->age_composition.size()){
       FIMS_LOG << "Error: observed age comp is of size " <<  this->oberved_agecomp_data.size() <<
       " and expected is of size " << this->age_composition.size() << std::endl;
      } else{
      for (size_t i = 0; i < this->age_composition.size(); i++) {
        dmultinom.x[i] = this->observed_agecomp_data[i];
        dmultinom.p[i] = this->age_composition[i];//probabilities for ages
        nll -= dmultinom.evaluate(true);
      }
      }
      return nll;
    }
  };


template <class Type>
uint32_t FleetIndexNLL<Type>::id_g = 0;

template <class Type>
uint32_t FleetAgeCompNLL<Type>::id_g = 0;
#endif
}  // namespace fims

// #endif /* TMB_MODEL */

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_NLL_HPP */
