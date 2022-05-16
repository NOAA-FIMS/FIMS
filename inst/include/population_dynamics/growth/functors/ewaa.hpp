/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the growth functor class
 * which is the base class for all growth functors.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_EWAA_HPP
#define POPULATION_DYNAMICS_GROWTH_EWAA_HPP


//#include "../../../interface/interface.hpp"
#include "growth_base.hpp"

namespace fims {

 /**
  *  @brief EWAAgrowth class that returns the EWAA function value.
 */ 
template<typename T>
struct EWAAgrowth : public GrowthBase<T> {
    //add submodule class members here
    //these include parameters of the submodule

    EWAAgrowth(): GrowthBase<T>(){

    }

    /**
    * @brief 
    * 
    * @param x  description
    */
    virtual const T evaluate(const T& x) {
        //you will need to add class members as arguments to the function below
        return fims::EWAA<T>(x);
    }
};

}

#endif /* POPULATION_DYNAMICS_GROWTH_EWAA_HPP */