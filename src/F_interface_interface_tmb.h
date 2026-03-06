#ifndef FIMS_INTERFACE_INTERFACE_TMB_H
#define FIMS_INTERFACE_INTERFACE_TMB_H

#ifndef FIMS_INTERFACE__TMBHPP
#define FIMS_INTERFACE__TMB_HPP
/**
 * @file interface.hpp
 * @brief An interface to the modeling platforms, e.g., TMB.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */



/*
 * @brief Interface file. Uses pre-processing macros
 * to interface with multiple modeling platforms.
 */

// traits for interfacing with TMB
#pragma once

#ifdef TMB_MODEL
//  #include <TMB.hpp>
// use isnan macro in math.h instead of TMB's isnan for fixing the r-cmd-check
// issue
#include <math.h>

//  #pragma once
//  #include <TMB.hpp>
namespace tmbutils {
template<typename Type>
struct vector;
}

template <typename Type>
tmbutils::vector<Type> ADREPORTvector(tmbutils::vector<tmbutils::vector<Type> > x) ;


#endif /* TMB_MODEL */

#ifndef TMB_MODEL
/**
 * @brief TMB macro that simulates data.
 */
#define FIMS_SIMULATE_F(F)
/**
 * @brief TMB macro that reports variables.
 */
#define FIMS_REPORT_F(name, F)
/**
 * @brief TMB macro that reports variables and uncertainties.
 */
#define ADREPORT_F(name, F)
#endif

#endif /* FIMS_INTERFACE_HPP */

#endif
