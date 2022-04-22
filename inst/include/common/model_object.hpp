/*
 * File:   model_object.hpp
 *
 * Author: Matthew Supernaw, Andrea Havron
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov, andrea.havron@noaa.gov
 *
 * Created on March 2, 2022
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

#ifndef FIMS_COMMON_MODEL_OBJECT_HPP
#define FIMS_COMMON_MODEL_OBJECT_HPP

#include <stdint.h>

#include <vector>

#include "def.hpp"

namespace fims {

template <typename T>
struct FIMSObject {
  uint32_t id;  // unique identifier assigned for all fims objects
  std::vector<T*> parameters;  // list of estimable parameters
  std::vector<T*>
      random_effects_parameters;  // list of all random effects parameters
  std::vector<T*> fixed_effects_parameters;  // list of fixed effects parameters

  uint32_t GetId() const { return id; }
};

}  // namespace fims

#endif /* FIMS_COMMON_MODEL_OBJECT_HPP */
