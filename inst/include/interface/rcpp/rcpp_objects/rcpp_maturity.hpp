/*
 * File:   rcpp_maturity.hpp
 *
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP

#include "../../../population_dynamics/maturity/maturity.hpp"
#include "rcpp_interface_base.hpp"

/**
 * maturity Rcpp interface
 */

/**
 * @brief MaturityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible maturity function
 */
class MaturityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, MaturityInterfaceBase*>
      live_objects; /**< map associating the ids of
                                MaturityInterfaceBase to the objects */

  MaturityInterfaceBase() {
    this->id = MaturityInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    MaturityInterfaceBase */
    MaturityInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~MaturityInterfaceBase() {}

  /** @brief get the ID of the interface base object
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief evaluate the function
   *
   */
  virtual double evaluate(double x) = 0;
};

uint32_t MaturityInterfaceBase::id_g = 1;
std::map<uint32_t, MaturityInterfaceBase*> MaturityInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for logistic maturity as an S4 object. To
 * instantiate from R: logistic_maturity <- new(logistic_maturity)
 */
class LogisticMaturityInterface : public MaturityInterfaceBase {
 public:
  Parameter
      inflection_point; /**< the index value at which the response reaches .5 */
  Parameter slope;      /**< the width of the curve at the inflection_point */

  LogisticMaturityInterface() : MaturityInterfaceBase() {}

  virtual ~LogisticMaturityInterface() {}

  /** @brief returns the id for the logistic maturity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the logistic maturity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in maturity).
   */
  virtual double evaluate(double x) {
    fims_popdy::LogisticMaturity<double> LogisticMat;
    LogisticMat.inflection_point.resize(1);
    LogisticMat.inflection_point[0] = this->inflection_point.value_m;
    LogisticMat.slope.resize(1);
    LogisticMat.slope[0] = this->slope.value_m;
    return LogisticMat.evaluate(x);
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::LogisticMaturity<Type> > maturity =
        std::make_shared<fims_popdy::LogisticMaturity<Type> >();

    // set relative info
    maturity->id = this->id;
    maturity->inflection_point.resize(1);
    maturity->inflection_point[0] = this->inflection_point.value_m;
    if (this->inflection_point.estimated_m) {
      info->RegisterParameterName("maturity inflection_point");
      if (this->inflection_point.is_random_effect_m) {
        info->RegisterRandomEffect(maturity->inflection_point[0]);
      } else {
        info->RegisterParameter(maturity->inflection_point[0]);
      }
    }
    maturity->slope.resize(1);
    maturity->slope[0] = this->slope.value_m;
    if (this->slope.estimated_m) {
      info->RegisterParameterName("maturity slope");
      if (this->slope.is_random_effect_m) {
        info->RegisterRandomEffect(maturity->slope[0]);
      } else {
        info->RegisterParameter(maturity->slope[0]);
      }
    }

    // add to Information
    info->maturity_models[maturity->id] = maturity;

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