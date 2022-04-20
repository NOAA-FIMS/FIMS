/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the module_name functor class
 * which is the base class for all module_name functors.
 */
#ifndef POPULATION_DYNAMICS_module_name_sub_module_HPP
#define POPULATION_DYNAMICS_module_name_sub_module_HPP


//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "module_name_base.hpp"

namespace fims {

 /**
  *  @brief sub_modulemodule_name class that returns the sub_module function value
 * from fims_math.
 */ 
template<typename T>
struct sub_modulemodule_name : public module_nameBase<T> {
    T median;/*!< 50% quantile of the value of the quantity of interest (x); e.g. age at which 50% of the fish are selected */
    T slope; /*!<scalar multiplier of difference between quantity of interest value (x) and median */

    sub_modulemodule_name(): module_nameBase<T>(){

    }

    /**
    * @brief 
    * 
    * @param x  description
    */
    virtual const T evaluate(const T& x) {
        return fims::sub_module<T>(median, slope, x);
    }
};

}

#endif /* POPULATION_DYNAMICS_module_name_sub_module_HPP */