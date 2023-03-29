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

/****************************************************************
 * maturity Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief MaturityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible maturity function
 * */
class MaturityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  static std::map<uint32_t, MaturityInterfaceBase*>
      maturity_objects; /**< map associating the ids of
                              MaturityInterfaceBase to the objects */

  MaturityInterfaceBase() {
    this->id = MaturityInterfaceBase::id_g++;
    MaturityInterfaceBase::maturity_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~MaturityInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() = 0;

  /**
   * @brief evaluate the function
   *
   */
  virtual double evaluate(double x) = 0;
};

uint32_t MaturityInterfaceBase::id_g = 1;
std::map<uint32_t, MaturityInterfaceBase*>
    MaturityInterfaceBase::maturity_objects;

/**
 * @brief Rcpp interface for logistic maturity as an S4 object. To
 * instantiate from R: logistic_maturity <- new(fims$logistic_maturity)
 */
class LogisticMaturityInterface : public MaturityInterfaceBase {
 public:
  Parameter median; /**< the index value at which the response reaches .5 */
  Parameter slope;  /**< the width of the curve at the median */

  LogisticMaturityInterface() : MaturityInterfaceBase() {}

  virtual ~LogisticMaturityInterface() {}

  /** @brief returns the id for the logistic maturity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the logistic maturity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in maturity).
   */
  virtual double evaluate(double x) {
    fims::LogisticMaturity<double> LogisticMat;

    LogisticMat.median = this->median.value;
    LogisticMat.slope = this->slope.value;
    return LogisticMat.evaluate(x);
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::LogisticMaturity<TMB_FIMS_REAL_TYPE> > lm0 =
        std::make_shared<fims::LogisticMaturity<TMB_FIMS_REAL_TYPE> >();

    // set relative info
    lm0->id = this->id;
    lm0->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d0->RegisterRandomEffect(lm0->median);
      } else {
        d0->RegisterParameter(lm0->median);
      }
    }
    lm0->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d0->RegisterRandomEffect(lm0->slope);
      } else {
        d0->RegisterParameter(lm0->slope);
      }
    }

    // add to Information
    d0->maturity_models[lm0->id] = lm0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::LogisticMaturity<TMB_FIMS_FIRST_ORDER> > lm1 =
        std::make_shared<fims::LogisticMaturity<TMB_FIMS_FIRST_ORDER> >();

    // set relative info
    lm1->id = this->id;
    lm1->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d1->RegisterRandomEffect(lm1->median);
      } else {
        d1->RegisterParameter(lm1->median);
      }
    }
    lm1->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d1->RegisterRandomEffect(lm1->slope);
      } else {
        d1->RegisterParameter(lm1->slope);
      }
    }

    // add to Information
    d1->maturity_models[lm1->id] = lm1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::LogisticMaturity<TMB_FIMS_SECOND_ORDER> > lm2 =
        std::make_shared<fims::LogisticMaturity<TMB_FIMS_SECOND_ORDER> >();

    // set relative info
    lm2->id = this->id;
    lm2->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d2->RegisterRandomEffect(lm2->median);
      } else {
        d2->RegisterParameter(lm2->median);
      }
    }
    lm2->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d2->RegisterRandomEffect(lm2->slope);
      } else {
        d2->RegisterParameter(lm2->slope);
      }
    }

    // add to Information
    d2->maturity_models[lm2->id] = lm2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::LogisticMaturity<TMB_FIMS_THIRD_ORDER> > lm3 =
        std::make_shared<fims::LogisticMaturity<TMB_FIMS_THIRD_ORDER> >();

    // set relative info
    lm3->id = this->id;
    lm3->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d3->RegisterRandomEffect(lm3->median);
      } else {
        d3->RegisterParameter(lm3->median);
      }
    }
    lm3->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d3->RegisterRandomEffect(lm3->slope);
      } else {
        d3->RegisterParameter(lm3->slope);
      }
    }

    // add to Information
    d3->maturity_models[lm3->id] = lm3;

    return true;
  }
};

#endif