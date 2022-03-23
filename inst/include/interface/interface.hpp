/*
 * File:   interface.hpp
 *
 * Author: Andrea Havron
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: andrea.havron@noaa.gov
 *
 * Created on February 23, 2022
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

#ifndef FIMS_INTERFACE_HPP
#define FIMS_INTERFACE_HPP
/*
* Interface file. Uses pre-processing macros
* to interface with multiple modeling platforms.
*/


//traits for interfacing with TMB
#ifdef TMB_MODEL

//#define TMB_LIB_INIT R_init_FIMS
#include <TMB.hpp>

template<typename Type>
struct ModelTraits{
  typedef typename CppAD::vector<Type> DataVector;
  typedef typename CppAD::vector<Type> ParameterVector;
};

#endif /* TMB_MODEL */

#endif /* FIMS_INTERFACE_HPP */
