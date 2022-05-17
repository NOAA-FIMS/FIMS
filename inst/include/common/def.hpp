/*
 * File:   def.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on September 30, 2021, 3:59 PM
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
#ifndef DEF_HPP
#define DEF_HPP
#include <vector>

namespace fims {

#ifdef STD_LIB

/**
 * Default trait. These are "T" specific
 * traits that depend on modeling platform.
 */
template <typename T>
struct FIMSTraits {
  typedef double real_t;
  typedef double variable_t;
  typedef typename std::vector<double> data_vector;
  typedef typename std::vector<double> variable_vector;
  typedef typename std::vector<std::vector<double> > data_matrix;
  typedef typename std::vector<std::vector<double> > variable_matrix;
};

#endif
}  // namespace fims

#endif /* TRAITS_HPP */
