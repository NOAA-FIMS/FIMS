/*! \file recruitment_base.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Recruitment nll file
 * The purpose of this file is to serve as the class for recruitment negative log likelihood.
 *
 * DEFINE guards for recruitment nll outline to define the
 * recruitment hpp file if not already defined.
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_NLL_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_NLL_HPP


#include <cmath>  // for using std::pow and M_PI

#include "../../../common/fims_math.hpp"  // for using fims::log()
#include "../../../common/model_object.hpp"
#include "recruitment_base.hpp"
#include "../../../distributions/distributions.hpp"

namespace fims {
#ifdef TMB_MODEL

/** @brief Recruitment negative log-likelihood (nll).
 *
 * @tparam Type The type of the recruitment functor.
 *
 */
template <class Type>
struct RecruitmentNLL : public RecruitmentBase<Type> {
    static uint32_t id_g; /*!< reference id for recruitment nll object*/


  /** @brief Constructor.
   */
  RecruitmentNLL() { this->id = RecruitmentNLL::id_g++; }

  virtual ~RecruitmentNLL() {}

  virtual const Type evaluate(const Type &spawners, const Type &ssbzero) {
    Type recruits = 0.0;
    return recruits;
  }

  /** @brief likelihood component function.
   * Returns the negative log likelihood (nll).
   * Based on equation (A.3.10) in Methot and Wetzel (2013)
   * but with the addition of the constant terms.
   */
  Type evaluate_nll() {
    Type nll = 0.0; /*!< The negative log likelihood value */
    if (!this->estimate_recruit_deviations) {
      return nll;
    } else {
      //Rcout << "Rec devs being passed to C++ are "
      //      << recruit_deviations << std::endl;
      fims::Dnorm<Type> dnorm;
      dnorm.sd = this->log_sigma_recruit;

      for (size_t i = 0; i < this->recruit_deviations.size(); i++) { 
        dnorm.mean = 0.0;
        if(this->use_recruit_bias_adjustment){
          dnorm.mean -= this->recruit_bias_adjustment[i];
        } 
          dnorm.x = log(this->recruit_deviations[i]);
          nll += dnorm.evaluate(true);
        }
      return nll;
    }
  }
};

template <class Type>
uint32_t RecruitmentNLL<Type>::id_g = 0;
#endif
}  // namespace fims

// #endif /* TMB_MODEL */

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_NLL_HPP */