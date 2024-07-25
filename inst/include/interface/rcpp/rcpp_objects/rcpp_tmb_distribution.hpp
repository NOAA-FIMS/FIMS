/*
 * File:   rcpp_distributions.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_TMB_DISTRIBUTION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_TMB_DISTRIBUTION_HPP

#include "../../../distributions/distributions.hpp"
#include "../../interface.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Distributions Rcpp Interface
 *
 */
class DistributionsInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t
      id_g;    /**< static id of the DistributionsInterfaceBase object */
  uint32_t id; /**< local id of the DistributionsInterfaceBase object */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, DistributionsInterfaceBase *> live_objects; /**<
map relating the ID of the DistributionsInterfaceBase to the
DistributionsInterfaceBase objects */

  DistributionsInterfaceBase() {
    this->id = DistributionsInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    DistributionsInterfaceBase */
    DistributionsInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~DistributionsInterfaceBase() {}

  /** @brief get_id method for child distribution interface objects to inherit
   */
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child distribution interface objects to inherit
   */
  virtual double evaluate() = 0;
};

uint32_t DistributionsInterfaceBase::id_g =
    1; /**< static id of the DistributionsInterfaceBase object */
std::map<uint32_t,
         DistributionsInterfaceBase
             *> /**< local id of the DistributionsInterfaceBase object */
    DistributionsInterfaceBase::live_objects; /**<
      map relating the ID of the DistributionsInterfaceBase to the
      DistributionsInterfaceBase objects */

/**
 * @brief Rcpp interface for Dnorm as an S4 object. To instantiate
 * from R:
 * dnorm_ <- new(TMBDnormDistribution)
 *
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  ParameterVector x;    /**< observed data */
  ParameterVector expected_values; /**< mean of x for the normal distribution */
  ParameterVector log_sd;   /**< sd of x for the normal distribution */
  Rcpp::LogicalVector is_na; /**<Boolean; if true, data observation is NA and the likelihood contribution is skipped */

  DnormDistributionsInterface() : DistributionsInterfaceBase() {}

  virtual uint32_t get_id() { return this->id; }

  virtual ~DnormDistributionsInterface() {}

  /**
   * @brief Evaluate normal probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */


  virtual double evaluate() {
    fims_distributions::NormalLPDF<double> dnorm;
    dnorm.x.resize(this->x.size());
    dnorm.expected_values.resize(this->expected_values.size());
    dnorm.log_sd.resize(this->log_sd.size());
    dnorm.is_na.resize(this->is_na.size());
    for(int i=0; i<x.size(); i++){
      dnorm.x[i] = this->x[i].value_m;
    }
    for(int i=0; i<expected_values.size(); i++){
      dnorm.expected_values[i] = this->expected_values[i].value_m;
    }
    for(int i=0; i<log_sd.size(); i++){
      dnorm.log_sd[i] = this->log_sd[i].value_m;
    }
    for(int i=0; i<is_na.size(); i++){
      dnorm.is_na[i] = this->is_na[i];
    }
    return dnorm.evaluate();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::NormalLPDF<Type>> distribution =
        std::make_shared<fims_distributions::NormalLPDF<Type>>();

    // interface to data/parameter value
    distribution->id = this->id;
    distribution->x.resize(this->x.size());
    for(int i=0; i<this->x.size(); i++){
      distribution->x[i] = this->x[i].value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for(int i=0; i<this->expected_values.size(); i++){
      distribution->expected_values[i] = this->expected_values[i].value_m;
    }
    distribution->log_sd.resize(this->log_sd.size());
    for(int i=0; i<this->log_sd.size(); i++){
      distribution->log_sd[i] = this->log_sd[i].value_m;
    }
    distribution->is_na.resize(this->is_na.size());
    for(int i=0; i<is_na.size(); i++){
      distribution->is_na[i] = this->is_na[i];
    }

    info->distribution_models[distribution->id] = distribution;

    return true;
  }

  /**
   * @brief adds the dnorm distribution and its parameters to the TMB model
   */
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
 * @brief Rcpp interface for Dlnorm as an S4 object. To instantiate
 * from R:
 * dlnorm_ <- new(TMBDlnormDistribution)
 *
 */
class DlnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  ParameterVector x;       /**< observation */
  ParameterVector expected_values; /**< mean of the distribution of log(x) */
  ParameterVector log_logsd;   /**< log standard deviation of the distribution of log(x) */
  Rcpp::String lpdf_type; /**< character string indicating type of input: data, re, prior */
  Rcpp::LogicalVector is_na; /**<Boolean; if true, data observation is NA and the likelihood contribution is skipped */

  DlnormDistributionsInterface() : DistributionsInterfaceBase() {}

  virtual ~DlnormDistributionsInterface() {}

  /**
   * @brief get the id of the Dlnorm distributions interface class object
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Evaluate lognormal probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */
  virtual double evaluate() {
    fims_distributions::LogNormalLPDF<double> dlnorm;
    dlnorm.lpdf_type = this->lpdf_type;
    dlnorm.x.resize(this->x.size());
    dlnorm.expected_values.resize(this->expected_values.size());
    dlnorm.log_logsd.resize(this->log_logsd.size());
    dlnorm.is_na.resize(this->is_na.size());
    for(int i=0; i<x.size(); i++){
      dlnorm.x[i] = this->x[i].value_m;
    }
    for(int i=0; i<expected_values.size(); i++){
      dlnorm.expected_values[i] = this->expected_values[i].value_m;
    }
    for(int i=0; i<log_logsd.size(); i++){
      dlnorm.log_logsd[i] = this->log_logsd[i].value_m;
    }
    for(int i=0; i<is_na.size(); i++){
      dlnorm.is_na[i] = this->is_na[i];
    }
    return dlnorm.evaluate();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::LogNormalLPDF<Type>> distribution =
        std::make_shared<fims_distributions::LogNormalLPDF<Type>>();

    // set relative info
    distribution->id = this->id;
    distribution->lpdf_type = this->lpdf_type;
    distribution->x.resize(this->x.size());
    for(int i=0; i<this->x.size(); i++){
      distribution->x[i] = this->x[i].value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for(int i=0; i<this->expected_values.size(); i++){
      distribution->expected_values[i] = this->expected_values[i].value_m;
    }
    distribution->log_logsd.resize(this->log_logsd.size());
    for(int i=0; i<this->log_logsd.size(); i++){
      distribution->log_logsd[i] = this->log_logsd[i].value_m;
    }
    distribution->is_na.resize(this->is_na.size());
    for(int i=0; i<is_na.size(); i++){
      distribution->is_na[i] = this->is_na[i];
    }

    info->distribution_models[distribution->id] = distribution;

    return true;
  }

  /**
   * @brief adds the dlnorm distribution and its parameters to the TMB model
   */
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
 * @brief Rcpp interface for Dmultinom as an S4 object. To instantiate
 * from R:
 * dmultinom_ <- new(TMBDmultinomDistribution)
 *
 */
// template <typename Type>

class DmultinomDistributionsInterface : public DistributionsInterfaceBase {
 public:
  ParameterVector x; /**< Vector of length K of integers */
  ParameterVector expected_values; /**< Vector of length K, specifying the probability
 for the K classes (note, unlike in R these must sum to 1). */
  Rcpp::LogicalVector is_na; /**<Boolean; if true, data observation is NA and the likelihood contribution is skipped */
  Rcpp::NumericVector dims; /**< Dimensions of the number of rows and columns of the multivariate dataset */

  DmultinomDistributionsInterface() : DistributionsInterfaceBase() {}

  virtual ~DmultinomDistributionsInterface() {}

  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Evaluate multinom probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */
  virtual double evaluate() {
    fims_distributions::MultinomialLPMF<double> dmultinom;
    // Declare TMBVector in this scope
    dmultinom.x.resize(this->x.size());
    dmultinom.expected_values.resize(this->expected_values.size());
    dmultinom.is_na.resize(this->is_na.size());
    for(int i=0; i<x.size(); i++){
      dmultinom.x[i] = this->x[i].value_m;
    }
    for(int i=0; i<expected_values.size(); i++){
      dmultinom.expected_values[i] = this->expected_values[i].value_m;
    }
    for(int i=0; i<is_na.size(); i++){
      dmultinom.is_na[i] = this->is_na[i];
    }
    dmultinom.dims.resize(2);
    dmultinom.dims[0] = this->dims[0];
    dmultinom.dims[1] = this->dims[1];
    return dmultinom.evaluate();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::MultinomialLPMF<Type>> distribution =
        std::make_shared<fims_distributions::MultinomialLPMF<Type>>();

    distribution->id = this->id;
    distribution->x.resize(this->x.size());
    for(int i=0; i<this->x.size(); i++){
      distribution->x[i] = this->x[i].value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for(int i=0; i<this->expected_values.size(); i++){
      distribution->expected_values[i] = this->expected_values[i].value_m;
    }
    distribution->is_na.resize(this->is_na.size());
    for(int i=0; i<is_na.size(); i++){
      distribution->is_na[i] = this->is_na[i];
    }
    distribution->dims.resize(2);
    distribution->dims[0] = this->dims[0];
    distribution->dims[1] = this->dims[1];

    info->distribution_models[distribution->id] = distribution;

    return true;
  }

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
