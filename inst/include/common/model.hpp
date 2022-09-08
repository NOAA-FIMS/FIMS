/*
 * File:   model.hpp
 *
 * Author: Matthew Supernaw, Andrea Havron
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov, andrea.havron@noaa.gov
 *
 * Created on September 30, 2021, 1:08 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_COMMON_MODEL_HPP
#define FIMS_COMMON_MODEL_HPP

#include <memory>

#include "../population_dynamics/fleet/fleet.hpp"
#include "../population_dynamics/population/population.hpp"
#include "../population_dynamics/recruitment/recruitment.hpp"
#include "../population_dynamics/selectivity/selectivity.hpp"

namespace fims {

/**
 * Model base
 */
template <typename T>
class Model {
 public:
  static std::shared_ptr<Model<T> > fims_model;
  std::vector<std::shared_ptr<Population<T> > > populations;
  std::vector<std::shared_ptr<Fleet<T> > > fleets;
    
    virtual ~Model(){
        
    }
};
template <typename T>
std::shared_ptr<Model<T> > Model<T>::fims_model = nullptr;

}  // namespace fims

#endif /* FIMS_COMMON_MODEL_HPP */
