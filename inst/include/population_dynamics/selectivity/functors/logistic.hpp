/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the SelectivityBase class
 * which is the base class for all selectivity functors.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP


//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "selectivity_base.hpp"

namespace fims {

 /**
  *  @brief LogisticSelectivity class that returns the logistic function value
 * from fims_math.
 */ 
template<typename T>
struct LogisticSelectivity : public SelectivityBase<T> {
    T median;/*!< 50% quantile of the value of the quantity of interest (x); e.g. age at which 50% of the fish are selected */
    T slope; /*!<scalar multiplier of difference between quantity of interest value (x) and median */

    LogisticSelectivity(): SelectivityBase<T>(){

    }

    /**
    * @brief 
    * 
    * @param x  description
    */
    virtual const T evaluate(const T& x) {
        return fims::logistic<T>(median, slope, x);
    }
};

}

#endif /* POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP */

