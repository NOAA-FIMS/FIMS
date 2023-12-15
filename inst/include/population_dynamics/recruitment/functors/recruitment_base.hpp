/** \file recruitment_base.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Recruitment base file
 * The purpose of this file is to serve as the parent class where
 * recruitment functions are called.
 *
 * DEFINE guards for recruitment base outline to define the
 * recruitment hpp file if not already defined.
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP

#include <cmath>  // for using std::pow and M_PI

#include "../../../common/fims_math.hpp"  // for using fims_math::log()
#include "../../../common/fims_vector.hpp"
#include "../../../common/model_object.hpp"
#include "../../../distributions/distributions.hpp"

namespace fims_popdy {

/** @brief Base class for all recruitment functors.
 *
 * @tparam Type The type of the recruitment functor.
 *
 */
template <class Type>
struct RecruitmentBase : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /**< reference id for recruitment object*/

  fims::Vector<Type>
      log_recruit_devs; /*!< A vector of log recruitment deviations */
  bool constrain_deviations = false; /*!< A flag to indicate if recruitment
                                 deviations are summing to zero or not */

  Type log_sigma_recruit; /**< Log standard deviation of log recruitment
                       deviations */
  Type log_rzero;         /**< Log of unexploited recruitment.*/

  bool estimate_log_recruit_devs = true; /*!< A flag to indicate if recruitment
                                  deviations are estimated or not */

  /** @brief Constructor.
   */
  RecruitmentBase() { this->id = RecruitmentBase::id_g++; }

  virtual ~RecruitmentBase() {}

  /**
   * @brief Prepares the recruitment deviations vector.
   *
   */
  void Prepare() { this->PrepareConstrainedDeviations(); }

  /** @brief Calculates the expected recruitment for a given spawning input.
   *
   * @param spawners A measure for spawning output.
   * @param ssbzero A measure for spawning output in unfished population.
   *
   */
  virtual const Type evaluate(
      const Type &spawners,
      const Type &ssbzero) = 0;  // need to add input parameter values

  /** @brief Calculates the negative log likelihood of recruitment deviations.
   *
   */
  virtual const Type evaluate_nll() {
    Type nll = 0.0; /**< The negative log likelihood value */

    if (!this->estimate_log_recruit_devs) {
      return nll;
    } else {
#ifdef TMB_MODEL
      fims_distributions::Dnorm<Type> dnorm;
      dnorm.sd = fims_math::exp(this->log_sigma_recruit);
      for (size_t i = 0; i < this->log_recruit_devs.size(); i++) {
        dnorm.x = this->log_recruit_devs[i];
        dnorm.mean = 0.0;
        nll -= dnorm.evaluate(true);
      }
#endif
      return nll;
    }
  }

  /** @brief Prepare constrained recruitment deviations.
   *  Based on ADMB sum-to-zero constraint implementation. We still
   *  need to adde an additional penalty to the PrepareConstrainedDeviations
   *  method. More discussion can be found here:
   *  https://groups.google.com/a/admb-project.org/g/users/c/63YJmYGEPuE
   */
  void PrepareConstrainedDeviations() {
    if (!this->constrain_deviations) {
      return;
    }

    Type sum = 0.0;

    for (size_t i = 0; i < this->log_recruit_devs.size(); i++) {
      sum += this->log_recruit_devs[i];
    }

    RECRUITMENT_LOG << "log_recruit_devs: \n";
    for (size_t i = 0; i < this->log_recruit_devs.size(); i++) {
      this->log_recruit_devs[i] -= sum / (this->log_recruit_devs.size());
      RECRUITMENT_LOG << this->log_recruit_devs[i] << std::endl;
    }
  }
};

template <class Type>
uint32_t RecruitmentBase<Type>::id_g = 0;
}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP */
