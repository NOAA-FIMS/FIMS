/*
 * File:   information.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */

#ifndef FIMS_COMMON_INFORMATION_HPP
#define FIMS_COMMON_INFORMATION_HPP

#include <map>
#include <memory>
#include <vector>

#include "../interface/interface.hpp"
#include "model_object.hpp"

namespace fims {

/**
 * Contains all objects and data pre-model construction
 */
template <typename T>
class Information {
  typedef fims::FIMSTrait<T>::variable_t variable_t;
  typedef fims::FIMSTrait<T>::real_t real_t;
  std::vector<variable_t*> parameters;  // list of all estimated parameters
  std::vector<variable_t*>
      random_effects_parameters;  // list of all random effects parameters
  std::vector<variable_t*>
      fixed_effects_parameters;  // list of all fixed effects parameters

  bool CreateModel() {}
};
}  // namespace fims

#endif /* FIMS_COMMON_INFORMATION_HPP */
