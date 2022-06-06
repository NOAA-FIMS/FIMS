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
#include <map>

namespace fims {

 /**
  *  @brief EWAAgrowth class that returns the EWAA function value.
 */ 
template<typename T>
struct EWAAgrowth : public GrowthBase<T> {
    //add submodule class members here
    //these include parameters of the submodule
    //a map looks up values based on a reference key
    //in this case, our key is age (first double), and 
    // the value is the weight at that age (second double)
    std::map<double, double> ewaa;  /*!<map of doubles for EWAA values by age, where age 
             starts at zero > */
    

  static EWAAgrowth<T>* instance;

    EWAAgrowth(): GrowthBase<T>(){

    }

    /**
    * @brief Returns the weight at age a (in kg) from the input vector.
    * 
    * @param a  age of the fish, the age vector must start at zero
    */
    virtual const T evaluate(const double& a) {
        return ewaa[a];
    }

};
}
#endif /* POPULATION_DYNAMICS_GROWTH_EWAA_HPP */