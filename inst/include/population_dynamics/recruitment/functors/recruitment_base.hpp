/*
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

namespace fims{

/* @brief Base class for all recruitment functors.
* 
* @tparam Type The type of the recruitment functor.
* 
*/
    template<class Type>
    struct RecruitmentBase : public FIMSObject<Type>{
        static uint32_t id_g;

/* @brief Constructor.
*/
        RecruitmentBase(){
            this->id = RecruitmentBase::id_g++;
        }

/* @brief Calculates the expected recruitment for a given spawning input.
*
* @param spawners A measure for spawning output.
*/
        virtual const Type evaluate(const Type& spawners) = 0; //need to add input parameter values
    } ; 

    template<class Type>
    uint32_t RecruitmentBase<Type>::id_g = 0;
}

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_BASE_HPP */

