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
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_BEVERTON_HOLT_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_BEVERTON_HOLT_HPP

#include "../../../interface/interface.hpp"
#include "../../../fims_math.hpp"
#include "recruitment_base.hpp"

namespace fims{

 /* @brief BevertonHolt class that returns the Beverton Holt SR
 * from fims_math.
 *
 * @param steep Recruitment relative to unfished recruitment at 
 * 20% of unfished spawning biomass.
 * @param rzero Unexploited recruitment. 
 */ 
  template<typename Type>
  struct BevertonHolt :public RecruitmentBase<>{

    Type steep;
    Type rzero;

    BevertonHolt():RecruitmentBase<Type>(){
    }

/* @brief Beverton Holt implementation of the stock recruitment function.
*
* The Beverton Holt stock recruitment implementation:
* \f$ \frac{0.8 R_{0} h S_{t-1}}{0.2 R_{0} \phi_{0} (1 - h) + S_{t-1} (h - 0.2)} \f$
*
* @param phizero Unexploited spawners per recruit.
* @param spawners A measure of spawning output.
*/
    virtual const Type evaluate(const Type& phizero, const Type& spawners){
        return fims::beverton_holt_sr(steep, rzero, phizero, spawners);
    }
  }

}

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_BEVERTON_HOLT_HPP */

