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

#include "../../../common/model_object.hpp"
#include <cmath>                         // for using std::pow
#include "../../../common/fims_math.hpp" //for using fims::log()

namespace fims
{

  /** @brief Base class for all recruitment functors.
   *
   * @tparam Type The type of the recruitment functor.
   *
   */
  template <class Type>
  struct RecruitmentBase : public FIMSObject<Type>
  {
    static uint32_t id_g; /*!< reference id for recruitment object*/

    std::vector<Type> recruit_deviations;      /*!< A vector of recruitment deviations */
    bool constrain_deviations = true;          /*!< A flag to indicate if recruitment deviations are summing to zero or not */
    std::vector<Type> recruit_bias_adjustment; /*!< A vector of bias adj values (incorporating sigma_recruit)*/
    // Initially fixing bias adjustment (b_y in collobarative
    // workflow specification) to 1.0.
    // In the future, this would be set by the user.
    std::vector<Type> recruit_bias_adjustment_fraction; /*!< A vector of bias adjustment fractions (on the 0 to 1 range)*/
    bool use_recruit_bias_adjustment = true;            /*!< A flag to indicate if recruitment deviations are bias adjusted */
    Type sigma_recruit;                                 /*!< Standard deviation of log recruitment deviations */
    bool estimate_recruit_deviations = true;            /*!< A flag to indicate if recruitment deviations are estimated or not */

    /** @brief Constructor.
     */
    RecruitmentBase() { this->id = RecruitmentBase::id_g++; }

    /** @brief Calculates the expected recruitment for a given spawning input.
     *
     * @param spawners A measure for spawning output.
     */
    virtual const Type evaluate(
        const Type &spawners) = 0; // need to add input parameter values

    /** @brief Prepare constrained recruitment deviations.
     *
     */
    void PrepareConstrainedDeviations()
    {
      if (!this->constrain_deviations)
      {
        return;
      }

      Type sum = 0.0;

      for (int i = 0; i < this->recruit_deviations.size(); i++)
      {
        sum += this->recruit_deviations[i];
      }

      for (int i = 0; i < this->recruit_deviations.size(); i++)
      {
        this->recruit_deviations[i] -= sum / (this->recruit_deviations.size());
      }
    }

    /** @brief Prepare recruitment bias adjustment.
     *
     */
    void PrepareBiasAdjustment()
    {
      Type recruit_bias_adjustment_size = this->recruit_bias_adjustment.size();

      if (!this->use_recruit_bias_adjustment)
      {
        for (int i = 0; i < recruit_bias_adjustment_size; i++)
        {
          this->recruit_bias_adjustment_fraction[i] = 1.0;
          this->recruit_bias_adjustment[i] = 0.0;
        }
      }
      else
      {
        for (int i = 0; i < recruit_bias_adjustment_size; i++)
        {
          // Initially fixing bias adjustment (b_y in collobarative
          // workflow specification) to 1.0.
          // In the future, this would be set by the user.
          this->recruit_bias_adjustment_fraction[i] = 1.0;
          this->recruit_bias_adjustment[i] = 0.5 * std::pow(this->sigma_recruit, 2) * this->recruit_bias_adjustment_fraction[i];
        }
      }
    }

    /** @brief Prepare recruitment bias adjustment for benchmarking.
     *
     */
    void PrepareBiasAdjustmentBenchmark()
    {
      Type recruit_bias_adjustment_size = this->recruit_bias_adjustment.size();

      for (int i = 0; i < recruit_bias_adjustment_size; i++)
      {
        if (!this->use_recruit_bias_adjustment)
        {
          this->recruit_bias_adjustment_fraction[i] = 1.0;
          this->recruit_bias_adjustment[i] = 0.0;
        }
        else
        {
          // Initially fixing bias adjustment (b_y in collobarative
          // workflow specification) to 1.0.
          // In the future, this would be set by the user.
          this->recruit_bias_adjustment_fraction[i] = 1.0;
          this->recruit_bias_adjustment[i] = 0.5 * std::pow(this->sigma_recruit, 2) * this->recruit_bias_adjustment_fraction[i];
        }
      }
    }

    /** @brief likelihood component function.
    Returns the negative log likelihood (nll).
    Initially based on equation (13) in Methot & Taylor (2011) but with the
    addition of the constant terms.
       */
    Type recruit_nll()
    {
      Type nll;

      nll = 0.0;

      if (!this->estimate_recruit_deviations)
      {
        return nll;
      }
      else
      {
        for (int i = 0; i < this->recruit_deviations.size(); i++)
        {
          // did we include sigma twice?
          // are we sure it's not sigma^2
          nll -= 0.5 * pow((this->recruit_deviations[i] / this->sigma_recruit), 2) +
                         this->recruit_bias_adjustment_fraction[i] * fims::log(this->sigma_recruit) -
                         fims::log(sigma_recruit) - fims::log(sqrt(2 * M_PI));
        }
        return nll;
      }
    }
  };

  template <class Type>
  uint32_t RecruitmentBase<Type>::id_g = 0;
} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP */
