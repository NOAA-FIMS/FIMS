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

namespace fims{

 /** @brief BevertonHolt class that returns the Beverton Holt SR
 * from fims_math.
 *
 * @param steep Recruitment relative to unfished recruitment at 
 * 20% of unfished spawning biomass. Should be a value between 0.2 and 1.0.
 * @param rzero Unexploited recruitment. Should be a positive value.
 * @param phizero the unfished spawning biomass per recruit. Should be a positive value.
 */ 
  template<typename Type>
  struct SRBevertonHolt :public RecruitmentBase<Type>{

    //Here we define the members that will be used in the Beverton Holt SR function.
    //These members are needed by Beverton Holt but will not be common to all 
    //recruitment functions like spawners is below.
    Type steep;/*!< Recruitment relative to unfished recruitment at 20% of unfished spawning biomass. Should be a value between 0.2 and 1.0.*/
    Type rzero; /*!< Unexploited recruitment. Should be a positive value.*/
    Type phizero;/*!< The unfished spawning biomass per recruit. Should be a positive value.*/

    SRBevertonHolt():RecruitmentBase<Type>(){
    }

/** @brief Beverton Holt implementation of the stock recruitment function.
*
* The Beverton Holt stock recruitment implementation:
* \f$ \frac{0.8 R_{0} h S_{t-1}}{0.2 R_{0} \phi_{0} (1 - h) + S_{t-1} (h - 0.2)} \f$
*
* @param spawners A measure of spawning output.
*/
    virtual const Type evaluate(const Type& spawners){
        
              Type recruits;
              recruits = (0.8 * rzero * steep * spawners)/(0.2 * rzero * phizero * (1.0 - steep) + spawners * (steep - 0.2));
              return recruits;
    }
  };

}

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_SR_BEVERTON_HOLT_HPP */

