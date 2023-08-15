/*! \file recruitment_base.hpp
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

#include <cmath> // for using std::pow and M_PI

#include "../../../common/fims_math.hpp" // for using fims::log()
#include "../../../common/model_object.hpp"
#include "../../../distributions/distributions.hpp"

namespace fims {

/** @brief Base class for all recruitment functors.
 *
 * @tparam Type The type of the recruitment functor.
 *
 */
template <class Type> struct RecruitmentBase : public FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for recruitment object*/

  typename ModelTraits<Type>::ParameterVector
      recruit_deviations;            /*!< A vector of recruitment deviations */
  bool constrain_deviations = false; /*!< A flag to indicate if recruitment
                                 deviations are summing to zero or not */
  typename ModelTraits<Type>::DataVector
      recruit_bias_adjustment; /*!< A vector of bias adj values
       (incorporating sigma_recruit)*/
  // Initially fixing bias adjustment (b_y in collobarative
  // workflow specification) to 1.0.
  // In the future, this would be set by the user.
  typename ModelTraits<Type>::DataVector
      recruit_bias_adjustment_fraction; /*!< A vector of bias adjustment
                                         fractions (on the 0 to 1 range)*/
  bool use_recruit_bias_adjustment =
      true; /*!< A flag to indicate if recruitment deviations are bias adjusted
             */
  Type log_sigma_recruit; /*!< Log standard deviation of log recruitment
                       deviations */
  Type log_rzero;         /*!< Log of unexploited recruitment.*/

  bool estimate_recruit_deviations =
      true; /*!< A flag to indicate if recruitment deviations are estimated or
     not */

  /** @brief Constructor.
   */
  RecruitmentBase() { this->id = RecruitmentBase::id_g++; }

  virtual ~RecruitmentBase() {}

  /**
   * @brief Prepares the recruitment bias adjustment vector.
   *
   */
  void Prepare() {
    this->recruit_bias_adjustment_fraction.resize(
        this->recruit_deviations.size());
    this->recruit_bias_adjustment.resize(this->recruit_deviations.size());
    this->PrepareConstrainedDeviations();
  }

  /** @brief Calculates the expected recruitment for a given spawning input.
   *
   * @param spawners A measure for spawning output.
   * @param ssbzero A measure for spawning output in unfished population.
   *
   */
  virtual const Type
  evaluate(const Type &spawners,
           const Type &ssbzero) = 0; // need to add input parameter values

  /** @brief Calculates the negative log likelihood of recruitment deviations.
   *
   */
  virtual const Type evaluate_nll() {
    Type nll = 0.0; /*!< The negative log likelihood value */

    if (!this->estimate_recruit_deviations) {
      return nll;
    } else {
#ifdef TMB_MODEL
      fims::Dnorm<Type> dnorm;
      dnorm.sd = fims::exp(this->log_sigma_recruit);
      for (size_t i = 0; i < this->recruit_deviations.size(); i++) {
        dnorm.x = fims::log(this->recruit_deviations[i]);
        dnorm.mean = 0.0;
        if (this->use_recruit_bias_adjustment) {
          dnorm.mean -= this->recruit_bias_adjustment[i];
        }
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

    for (size_t i = 0; i < this->recruit_deviations.size(); i++) {
      sum += this->recruit_deviations[i];
    }

    FIMS_LOG << "recruit_deviations: \n";
    for (size_t i = 0; i < this->recruit_deviations.size(); i++) {
      this->recruit_deviations[i] -= sum / (this->recruit_deviations.size());
      FIMS_LOG << this->recruit_deviations[i] << std::endl;
    }
  }

  /** @brief Prepare recruitment bias adjustment.
   *  Based on Methot & Taylor (2011).
   */
  void PrepareBiasAdjustment() {
    Type recruit_bias_adjustment_size = this->recruit_bias_adjustment.size();

    if (!this->use_recruit_bias_adjustment) {
      for (size_t i = 0; i < recruit_bias_adjustment_size; i++) {
        this->recruit_bias_adjustment_fraction[i] = 1.0;
        this->recruit_bias_adjustment[i] = 0.0;
      }
    } else {
      for (size_t i = 0; i < recruit_bias_adjustment_size; i++) {
        // Initially fixing bias adjustment (b_y in collobarative
        // workflow specification) to 1.0.
        // In the future, this would be set by the user.
        this->recruit_bias_adjustment_fraction[i] = 1.0;
        this->recruit_bias_adjustment[i] =
            0.5 * fims::exp(this->log_sigma_recruit) *
            fims::exp(this->log_sigma_recruit) *
            this->recruit_bias_adjustment_fraction[i];
      }
    }
  }
};

template <class Type> uint32_t RecruitmentBase<Type>::id_g = 0;
} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP */
