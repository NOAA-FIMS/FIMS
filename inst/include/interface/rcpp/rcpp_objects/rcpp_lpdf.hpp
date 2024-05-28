/**
 * @file rcpp_lpdf.hpp
 * @brief The Rcpp interface to declare different probability density
 * functions. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_LPDF_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_LPDF_HPP

#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief LPDFInterfaceBase class should be inherited to define different Rcpp
 * interfaces for each possible log probability density function (pdf).
 */
class LPDFInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static ID of the LPDFInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local ID of the LPDFInterfaceBase object.
   * TODO: think about why this is not id_m like other modules?
   */  
  uint32_t id;
  /**
   * @brief The map associating the ID of the LPDFInterfaceBase to the
     LPDFInterfaceBase objects. This is a live object, which is an
     object that has been created and lives in memory.
   */
  static std::map<uint32_t, LPDFInterfaceBase*> live_objects;

  /**
   * @brief The constructor.
   */
  LPDFInterfaceBase() {
    this->id = LPDFInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    LPDFInterfaceBase */
    LPDFInterfaceBase::live_objects[this->id] = this;
    LPDFInterfaceBase::fims_interface_objects.push_back(this);
  }

  /**
   * @brief The destructor.
   */
  virtual ~LPDFInterfaceBase() {}

  /**
   * @brief Get the ID for the child distribution interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief A method for each child log probability density function (pdf)
   * interface object to inherit so each pdf can have an evaluate() function.
   * TODO: Think about why this is evaluate_lpdf instead of just evaluate?
   */
  virtual double evaluate_lpdf() = 0;
};
// static id of the LPDFInterfaceBase object
uint32_t LPDFInterfaceBase::id_g = 1;
// local id of the LPDFInterfaceBase object map relating the ID of the
// LPDFInterfaceBase to the LPDFInterfaceBase objects
std::map<uint32_t, LPDFInterfaceBase*> LPDFInterfaceBase::live_objects;

#endif