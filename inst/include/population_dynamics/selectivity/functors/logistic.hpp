/*
 * File:   logistic.hpp
 *
 * Author: Matthew Supernaw, Andrea Havron, Kelli Johnson
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on September 30, 2021, 12:10 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained by
 * using this  software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express or
 * implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
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

