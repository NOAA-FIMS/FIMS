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
#ifndef FIMS_MATH_HPP
#define FIMS_MATH_HPP

// note: this is modeling platform specific, must be controlled by
// preprocessing macros
//#include "def.hpp"
#include <cmath>

namespace fims
{
#ifdef TMB_MODEL
#include <TMB.hpp>

    /**
     * @brief The exponential function.
     * The code cannot be tested using the compilation flag
     * -DTMB_MODEL through CMake and Google Test
     * @param x value to exponentiate. Please use fims::exp<double>(x) if x is an integer.
     * @return the exponentiated value
     */
    template <class T>
    inline const T exp(const T &x)
    {
        return exp(x);
    }

    /**
     * @brief The natural log function (base e)
     * The code cannot be tested using the compilation flag
     * -DTMB_MODEL through CMake and Google Test.
     * @param x the value to take the log of. Please use fims::log<double>(x) if x is an integer.
     * @return the log of the value
     */
    template <class T>
    inline const T log(const T &x)
    {
        return log(x);
    }

#endif

}

#endif /* FIMS_MATH_HPP */
