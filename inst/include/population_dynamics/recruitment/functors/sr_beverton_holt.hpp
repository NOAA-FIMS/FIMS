/**
 * @file sr_beverton_holt.hpp
 * @brief Calls the Beverton--Holt stock--recruitment function from fims_math
 * and does the calculation.
 * @details This function inherits from recruitment base.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP

#include "recruitment_base.hpp"
#include "../../../common/fims_vector.hpp"

namespace fims_popdy {

/** @brief BevertonHolt class that returns the Beverton--Holt
 * stock--recruitment from fims_math.
 *
 * @param logit_steep Recruitment relative to unfished recruitment at 20
 * percent of unfished spawning biomass. Steepness is subject to a logit
 * transformation to keep it between 0.2 and 1.0.
 */
template <typename Type>
struct SRBevertonHolt : public RecruitmentBase<Type> {
  // Here we define the members that will be used in the Beverton--Holt
  // stock--recruitment function. These members are needed by the Beverton--Holt
  // stock--recruitment function but will not be common to all recruitment
  // functions like spawners is below.
  fims::Vector<Type> logit_steep; /**< Transformed value of recruitment
                                  relative to unfished
                                  recruitment at 20 percent of unfished
                                  spawning biomass.*/

  SRBevertonHolt() : RecruitmentBase<Type>() {}

  virtual ~SRBevertonHolt() {}

  /** @brief Beverton--Holt implementation of the stock--recruitment function.
   *
   * The Beverton--Holt stock--recruitment implementation:
   * \f$ \frac{0.8 R_{0} h S_{t-1}}{0.2 R_{0} \phi_{0} (1 - h) + S_{t-1} (h -
   * 0.2)} \f$
   *
   * @param spawners A measure of spawning output.
   * @param phi_0 Number of spawners per recruit of an unfished population
   */
  virtual const Type evaluate_mean(const Type& spawners, const Type& phi_0) {
    Type recruits;
    Type steep;
    Type steep_lo = static_cast<Type>(0.2);
    Type steep_hi = static_cast<Type>(1.0);
    Type rzero;

    // Transform input parameters
    steep = fims_math::inv_logit(steep_lo, steep_hi, this->logit_steep[0]);
    rzero = fims_math::exp(this->log_rzero[0]);

    recruits = (static_cast<Type>(0.8) * rzero * steep * spawners) /
               (static_cast<Type>(0.2) * phi_0 * rzero *
                    (static_cast<Type>(1.0) - steep) +
                spawners * (steep - static_cast<Type>(0.2)));

    return recruits;
  }

  /** Empty return of base class function
   * @param pos position index
   */
  virtual const Type evaluate_process(size_t pos) { return 0; }

  /**
   * @brief Create a map of report vectors for the recruitment object.
   */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>>& report_vectors) {
    report_vectors["logit_steep"].emplace_back(this->logit_steep);
    report_vectors["log_rzero"].emplace_back(this->log_rzero);
    report_vectors["log_r"].emplace_back(this->log_r);
    report_vectors["log_devs"].emplace_back(this->log_recruit_devs);
  }

  virtual void get_report_vector_count(
      std::map<std::string, size_t>& report_vector_count) {
    report_vector_count["logit_steep"] += 1;
    report_vector_count["log_rzero"] += 1;
    report_vector_count["log_r"] += 1;
    report_vector_count["log_recruit_devs"] += 1;
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP */
