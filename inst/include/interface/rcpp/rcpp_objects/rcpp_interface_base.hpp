/*
 * File:   rcpp_interface_base.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
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
  double value; /**< initial value of the parameter*/
  double min =
      std::numeric_limits<double>::min(); /**< min value of the parameter*/
  double max =
      std::numeric_limits<double>::max(); /**< max value of the parameter*/
  bool is_random_effect = false;          /**< Is the parameter a random effect
                                             parameter? Default value is false.*/
  bool estimated =
      false; /**< Is the parameter estimated? Default value is false.*/

  /**
   * @brief Constructor for initializing Parameter.
   * @details Inputs include value, min, max, estimated.
   */
  Parameter(double value_m, double min_m, double max_m, bool estimated_m)
      : value(value_m), min(min_m), max(max_m), estimated(estimated_m) {}

  /**
   * @brief Constructor for initializing Parameter.
   * @details Inputs include value.
   */
  Parameter(double value) { this->value = value; }

  /**
   * @brief Constructor for initializing Parameter.
   * @details Set value to 0 when there is no input value.
   */
  Parameter() { this->value = 0; }
};

/**
 *@brief Base class for all interface objects
 */
class FIMSRcppInterfaceBase {
 public:
  /**< FIMS interface object vectors */
  static std::vector<FIMSRcppInterfaceBase *> fims_interface_objects;

  /** @brief virtual method to inherit to add objects to the TMB model */
  virtual bool add_to_fims_tmb() {
    std::cout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet "
                 "implemented.\n";
    return false;
  }
};
std::vector<FIMSRcppInterfaceBase *>
    FIMSRcppInterfaceBase::fims_interface_objects;

#endif
