/**
 * @file recruitment_base.hpp
 * @brief Serves as the parent class where recruitment functions are called.
 * @details Defines guards for recruitment base outline to define the
 * recruitment hpp file if not already defined.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
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

  fims::Vector<Type> log_recruit_devs; /*!< A vector of the natural log of
                                          recruitment deviations */
  bool constrain_deviations = false;   /*!< A flag to indicate if recruitment
                                   deviations are summing to zero or not */

  fims::Vector<Type> log_rzero; /**< Natural log of unexploited recruitment.*/
  fims::Vector<Type>
      log_r; /**< Natural log of recruitment used for random effects */
  fims::Vector<Type>
      log_expected_recruitment; /**< Expectation of the recruitment process */

  bool estimate_log_recruit_devs = true; /*!< A flag to indicate if recruitment
                                  deviations are estimated or not */

  int process_id = -999; /*!< id of recruitment process model object*/
  std::shared_ptr<fims_popdy::RecruitmentBase<Type>>
      process; /*!< shared pointer to recruitment processmodule */
  std::shared_ptr<fims_popdy::RecruitmentBase<Type>>
      recruitment; /*!< shared pointer to recruitment module */

  /** @brief Constructor.
   */
  RecruitmentBase() { this->id = RecruitmentBase::id_g++; }

  virtual ~RecruitmentBase() {}

  /**
   * @brief Prepares the recruitment deviations vector.
   *
   */
  void Prepare() {
    // this->PrepareConstrainedDeviations();
    std::fill(log_expected_recruitment.begin(), log_expected_recruitment.end(),
              0.0);
  }

  /** @brief Calculates the expected recruitment for a given spawning input.
   *
   * @param spawners A measure for spawning output.
   * @param ssbzero A measure for spawning output in unfished population.
   *
   */
  virtual const Type evaluate_mean(
      const Type &spawners,
      const Type &ssbzero) = 0;  // need to add input parameter values

  /** @brief Handle error in recruitment
   *
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate_process(size_t pos) = 0;

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

    Type sum = static_cast<Type>(0.0);

    for (size_t i = 0; i < this->log_recruit_devs.size(); i++) {
      sum += this->log_recruit_devs[i];
    }

    for (size_t i = 0; i < this->log_recruit_devs.size(); i++) {
      this->log_recruit_devs[i] -= sum / (this->log_recruit_devs.size());
    }
  }
};

template <class Type>
uint32_t RecruitmentBase<Type>::id_g = 0;
}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP */
