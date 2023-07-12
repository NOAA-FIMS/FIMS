/*
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Beverton Holt stock recruitment function
 * The purpose of this file is to call the Beverton Holt stock
 * recruitment function from fims_math and does the calculation.
 * Inherits from recruitment base.
 *
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP

#include "recruitment_base.hpp"

namespace fims {

/** @brief BevertonHolt class that returns the Beverton Holt SR
 * from fims_math.
 *
 * @param steep Recruitment relative to unfished recruitment at
 * 20% of unfished spawning biomass. Should be a value between 0.2 and 1.0.
 */
template <typename Type>
struct SRBevertonHolt : public RecruitmentBase<Type> {
  // Here we define the members that will be used in the Beverton Holt SR
  // function. These members are needed by Beverton Holt but will not be common
  // to all recruitment functions like spawners is below.
  Type logit_steep; /*!< Transformed value of recruitment relative to unfished
                 recruitment at 20% of unfished spawning biomass. Should be a
                 value between 0.2 and 1.0.*/

  SRBevertonHolt() : RecruitmentBase<Type>() {}

  virtual ~SRBevertonHolt() {}

  /** @brief Beverton Holt implementation of the stock recruitment function.
   *
   * The Beverton Holt stock recruitment implementation:
   * \f$ \frac{0.8 R_{0} h S_{t-1}}{0.2 R_{0} \phi_{0} (1 - h) + S_{t-1} (h -
   * 0.2)} \f$
   *
   * @param spawners A measure of spawning output.
   * @param phi_0 Number of spawners per recruit of an unfished population
   */
  virtual const Type evaluate(const Type& spawners, const Type& phi_0) {
    Type recruits;
    Type steep;
    Type steep_lo = 0.2;
    Type steep_hi = 1.0;
    Type rzero;

    // Transform input parameters
    steep = fims::inv_logit(steep_lo, steep_hi, this->logit_steep);
    rzero = fims::exp(this->log_rzero);

    recruits = (0.8 * rzero * steep * spawners) /
               (0.2 * phi_0 * rzero * (1.0 - steep) + spawners * (steep - 0.2));

    return recruits;
  }
};

}  // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP */
