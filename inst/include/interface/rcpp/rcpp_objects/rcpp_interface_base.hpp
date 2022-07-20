/*
 * File:   rcpp_interface_base.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on May 31, 2022 at 12:04 PM
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
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_INTERFACE_BASE_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_INTERFACE_BASE_HPP

#include <map>
#include <vector>

#include "../../../common/def.hpp"
#include "../../../common/information.hpp"
#include "../../interface.hpp"

#define RCPP_NO_SUGAR
#include <Rcpp.h>

/**
 * @brief RcppInterface class that defines
 * the interface between R and C++ for parameter types.
 */
class Parameter {
 public:
  double value;
  double min = std::numeric_limits<double>::min();
  double max = std::numeric_limits<double>::max();
  bool is_random_effect = false;
  bool estimated = false;

  Parameter(double value, double min, double max, bool estimated)
      : value(value), min(min), max(max), estimated(estimated) {}

  Parameter(double value) { this->value = value; }

  Parameter() { this->value = 0; }
};

/**
 *@brief Base class for all interface objects
 */
class FIMSRcppInterfaceBase {
 public:
  static std::vector<FIMSRcppInterfaceBase*> fims_interface_objects;

  /** @brief virtual method to inherit to add objects to the TMB model */
  virtual bool add_to_fims_tmb() {
    std::cout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet "
                 "implemented.\n";
    return false;
  }
};


/****************************************************************
 * Growth Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief Rcpp interface that serves as the parent class for
 * Rcpp growth interfaces. This type should be inherited and not
 * called from R directly.
 *
 */
class GrowthInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the GrowthInterfaceBase object */
  uint32_t id;          /**< local id of the GrowthInterfaceBase object */
  static std::map<uint32_t, GrowthInterfaceBase*> live_objects; /**<
  map relating the ID of the GrowthInterfaceBase to the GrowthInterfaceBase
  objects */

  GrowthInterfaceBase() {
    this->id = GrowthInterfaceBase::id_g++;
    GrowthInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~GrowthInterfaceBase() {}

  /** @brief get_id method for child growth interface objects to inherit **/
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child growth interface objects to inherit **/
  virtual double evaluate(double age) = 0;
};

/**
 * @brief Distributions Rcpp Interface
 *
 */
class DistributionsInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g;
  uint32_t id;
  static std::map<uint32_t, DistributionsInterfaceBase*> live_objects;

  DistributionsInterfaceBase() {
    this->id = DistributionsInterfaceBase::id_g++;
    DistributionsInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~DistributionsInterfaceBase() {}

  virtual uint32_t get_id() = 0;

  virtual double evaluate(bool do_log) = 0;
};

std::vector<FIMSRcppInterfaceBase*>
    FIMSRcppInterfaceBase::fims_interface_objects;

uint32_t GrowthInterfaceBase::id_g = 1;
std::map<uint32_t, GrowthInterfaceBase*> GrowthInterfaceBase::live_objects;

uint32_t DistributionsInterfaceBase::id_g = 1;
std::map<uint32_t, DistributionsInterfaceBase*>
    DistributionsInterfaceBase::live_objects;

#endif
