/*
 * File:   rcpp_selectivity.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_SELECTIVITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_SELECTIVITY_HPP

#include "../../../population_dynamics/selectivity/selectivity.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * Selectivity Rcpp interface                                   *
 ***************************************************************/

/**
 * @brief SelectivityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Selectivity function
 * */
class SelectivityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, SelectivityInterfaceBase*>
      live_objects; /**< map associating the ids of
                                SelectivityInterfaceBase to the objects */

  SelectivityInterfaceBase() {
    this->id = SelectivityInterfaceBase::id_g++;
    // Create instance of map: key is id and value is pointer to
    // SelectivityInterfaceBase
    SelectivityInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~SelectivityInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() = 0;

  /**
   * @brief evaluate the function
   *
   */
  virtual double evaluate(double x) = 0;
};

uint32_t SelectivityInterfaceBase::id_g = 1;
std::map<uint32_t, SelectivityInterfaceBase*>
    SelectivityInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(fims$logistic_selectivity)
 */
class LogisticSelectivityInterface : public SelectivityInterfaceBase {
 public:
  Parameter median; /**< the index value at which the response reaches .5 */
  Parameter slope;  /**< the width of the curve at the median */

  LogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  virtual ~LogisticSelectivityInterface() {}

  /** @brief returns the id for the logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the logistic selectivity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims::LogisticSelectivity<double> LogisticSel;
    LogisticSel.median = this->median.value_m;
    LogisticSel.slope = this->slope.value_m;
    return LogisticSel.evaluate(x);
  }

#ifdef TMB_MODEL

  template <typename T>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims::Information<T> > info =
        fims::Information<T>::GetInstance();

    std::shared_ptr<fims::LogisticSelectivity<T> > selectivity =
        std::make_shared<fims::LogisticSelectivity<T> >();

    // set relative info
    selectivity->id = this->id;
    selectivity->median = this->median.value_m;
    if (this->median.estimated_m) {
      if (this->median.is_random_effect_m) {
        info->RegisterRandomEffect(selectivity->median);
      } else {
        info->RegisterParameter(selectivity->median);
      }
    }
    selectivity->slope = this->slope.value_m;
    if (this->slope.estimated_m) {
      if (this->slope.is_random_effect_m) {
        info->RegisterRandomEffect(selectivity->slope);
      } else {
        info->RegisterParameter(selectivity->slope);
      }
    }

    // add to Information
    info->selectivity_models[selectivity->id] = selectivity;

    return true;
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(fims$logistic_selectivity)
 */
class DoubleLogisticSelectivityInterface : public SelectivityInterfaceBase {
 public:
  Parameter median_asc; /**< the index value at which the response reaches .5 */
  Parameter slope_asc;  /**< the width of the curve at the median */
  Parameter
      median_desc;      /**< the index value at which the response reaches .5 */
  Parameter slope_desc; /**< the width of the curve at the median */

  DoubleLogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  virtual ~DoubleLogisticSelectivityInterface() {}

  /** @brief returns the id for the double logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the double logistic selectivity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims::DoubleLogisticSelectivity<double> DoubleLogisticSel;
    DoubleLogisticSel.median_asc = this->median_asc.value_m;
    DoubleLogisticSel.slope_asc = this->slope_asc.value_m;
    DoubleLogisticSel.median_desc = this->median_desc.value_m;
    DoubleLogisticSel.slope_desc = this->slope_desc.value_m;
    return DoubleLogisticSel.evaluate(x);
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims::Information<Type> > info =
        fims::Information<Type>::GetInstance();

    std::shared_ptr<fims::DoubleLogisticSelectivity<Type> > selectivity =
        std::make_shared<fims::DoubleLogisticSelectivity<Type> >();

    // set relative info
    selectivity->id = this->id;
    selectivity->median_asc = this->median_asc.value_m;
    if (this->median_asc.estimated_m) {
      if (this->median_asc.is_random_effect_m) {
        info->RegisterRandomEffect(selectivity->median_asc);
      } else {
        info->RegisterParameter(selectivity->median_asc);
      }
    }
    selectivity->slope_asc = this->slope_asc.value_m;
    if (this->slope_asc.estimated_m) {
      if (this->slope_asc.is_random_effect_m) {
        info->RegisterRandomEffect(selectivity->slope_asc);
      } else {
        info->RegisterParameter(selectivity->slope_asc);
      }
    }
    selectivity->median_desc = this->median_desc.value_m;
    if (this->median_desc.estimated_m) {
      if (this->median_desc.is_random_effect_m) {
        info->RegisterRandomEffect(selectivity->median_desc);
      } else {
        info->RegisterParameter(selectivity->median_desc);
      }
    }
    selectivity->slope_desc = this->slope_desc.value_m;
    if (this->slope_desc.estimated_m) {
      if (this->slope_desc.is_random_effect_m) {
        info->RegisterRandomEffect(selectivity->slope_desc);
      } else {
        info->RegisterParameter(selectivity->slope_desc);
      }
    }

    // add to Information
    info->selectivity_models[selectivity->id] = selectivity;

    return true;
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif