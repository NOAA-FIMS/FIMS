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


#include "../../../interface/interface.hpp"
#include "../../../fims_math.hpp"
#include "selectivity_base.hpp"

namespace fims {

template<typename T>
struct LogisticSelectivity : public SelectivityBase<T> {
    T a50;
    T slope;

    LogisticSelectivity(): SelectivityBase<T>(){

    }

    virtual const T evaluate(const T& age) {
        return fims::logistic<T>(a50, slope, age);
    }
};

}

#endif /* POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP */

