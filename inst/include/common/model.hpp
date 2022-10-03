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
#include <future>

#include "information.hpp"

namespace fims {

/**
 * Model base
 */
template <typename T>
class Model {//may need singleton
 public:
  static std::shared_ptr<Model<T> >
      fims_model; /*!< Create a shared fims_model as a pointer to Model*/
  std::vector<std::shared_ptr<Population<T> > >
      populations;                                 /*!< list of all population*/
  std::vector<std::shared_ptr<Fleet<T> > > fleets; /*!< list of all fleets*/
  virtual ~Model() {}
};


}  // namespace fims

#endif /* FIMS_COMMON_MODEL_HPP */
