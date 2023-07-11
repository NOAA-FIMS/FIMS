/*
 * File:   rcpp_nll.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_NLL_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_NLL_HPP

#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief NLLInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible negative log-likelihood
 * function
 * */
class NLLInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  static std::map<uint32_t, NLLInterfaceBase*> live_objects;
  /**< map associating the ids of NLLInterfaceBase to the objects */

  NLLInterfaceBase() {
    this->id = NLLInterfaceBase::id_g++;
    NLLInterfaceBase::live_objects[this->id] = this;
    NLLInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~NLLInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child nll interface objects to inherit **/
  virtual double evaluate_nll() = 0;
};

uint32_t NLLInterfaceBase::id_g = 1;
std::map<uint32_t, NLLInterfaceBase*> NLLInterfaceBase::live_objects;


#endif