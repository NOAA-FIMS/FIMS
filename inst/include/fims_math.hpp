/*
 * File:   fims_math.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on September 30, 2021, 1:43 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the 
 * source folder for reuse information.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 */
#ifndef FIMS_MATH_HPP
#define FIMS_MATH_HPP

//note: this is modeling platform specific, must be controlled by
//preprocessing macros
//#include "def.hpp"
#include <cmath>

namespace fims {


#ifdef STD_LIB


/**
 * @brief The exponential function.
 * 
 * @param x value to exponentiate. Please use fims::exp<double>(x) if x is an integer.
 * @return the exponentiated value
 */
template <class T>
inline const T exp(const T& x) {
    return std::exp(x);
}

/**
 * @brief The natural log function (base e)
 * @param x the value to take the log of. Please use fims::log<double>(x) if x is an integer.
 * @return
 */
template <class T>
inline const T log(const T& x) {
    return std::log(x);
}

/**
 * @brief The general logistic function
 * @param median the median (inflection point) of the logistic function
 * @param slope the slope of the logistic function
 * @param x the index the logistic function should be evaluated at
 * @return
 */
template <class T>
inline const T logistic(const T& median, const T& slope, const T& x) {
    return (1.0) / (1.0 + exp(-1.0 * slope * (x - median)));
}

#endif

}

#endif /* FIMS_MATH_HPP */

