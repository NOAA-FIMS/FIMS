/*
 * File:   rcpp_lpdf.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_LPDF_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_LPDF_HPP

#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief LPDFInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible log probability density
 * function
 */
class LPDFInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, LPDFInterfaceBase*> live_objects;
  /**< map associating the ids of LPDFInterfaceBase to the objects */

  LPDFInterfaceBase() {
    this->id = LPDFInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    LPDFInterfaceBase */
    LPDFInterfaceBase::live_objects[this->id] = this;
    LPDFInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~LPDFInterfaceBase() {}

  /** @brief get the ID of the interface base object
   */
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child log probability density function interface objects to inherit */
  virtual double evaluate_lpdf() = 0;
};

uint32_t LPDFInterfaceBase::id_g = 1;
std::map<uint32_t, LPDFInterfaceBase*> LPDFInterfaceBase::live_objects;

#endif