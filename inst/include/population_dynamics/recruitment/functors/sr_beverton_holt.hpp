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
   * The Beverton--Holt stock--recruitment is implemented as follows
   * \f[R_t(S_{t-1})  = \frac{0.8 R_{0} h S_{t-1}}{0.2 R_{0} \phi_{0} (1 - h) + S_{t-1} (h -
   * 0.2)} \f]
   * where \f$R_t\f$ and \f$S_t\f$ are mean recruitment and spawning biomass at
   * time \f$t\f$, \f$h\f$ is steepness, and \f$\phi_0\f$ is the unfished
   * spawning biomass per recruit. The initial FIMS model implements a static
   * spawning biomass-per-recruit function, with the ability to overload the
   * method in the future to allow for time-variation in spawning biomass per
   * recruit that results from variation in life-history characteristics (e.g.,
   * natural mortality, maturity, or weight-at-age). Recruitment deviations
   * (\f$r_t\f$) are assumed to be normally distributed in log space with
   * standard deviation \f$\sigma_R\f$, \f$r_t \sim N(0,\sigma_R^2)\f$. Because
   * \f$r_t\f$ are applied as multiplicative, lognormal deviations, predictions
   * of realized recruitment include a term for bias correction
   * (\f$\sigma^2_R/2\f$). However, true \f$r_t\f$ values are not known, but
   * rather estimated (\f$\hat{r}_t\f$), and thus the bias correction applies
   * an adjustment factor, \f$b_t=\frac{E[SD(\hat{r}_{t})]^2}{\sigma_R^2}\f$
   * (Methot and Taylor, 2011). The adjusted bias correction, mean recruitment,
   * and recruitment deviations are then used to compute realized recruitment
   * (\f$R^*_t\f$),
   * \f[R^*_t=R_t\cdot\mathrm{exp}\Bigg(\hat{r}_{t}-b_t\frac{\sigma_R^2}{2}\Bigg)\f]
   * The recruitment function should take as input the values of \f$S_t\f$,
   * \f$h\f$, \f$R_0\f$, \f$\phi_0\f$, \f$\sigma_R\f$, and \f$\hat{r}_{t}\f$,
   * and return mean-unbiased (\f$R_t\f$) and realized (\f$R^*_t\f$)
   * recruitment.
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
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP */
