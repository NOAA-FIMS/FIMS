/*
 * File:   rcpp_distributions.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_TMB_DNORM_DISTRIBUTION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_TMB_DNORM_DISTRIBUTION_HPP

#include "../../../distributions/distributions.hpp"
#include "rcpp_interface_base.hpp"

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

uint32_t DistributionsInterfaceBase::id_g = 1;
std::map<uint32_t, DistributionsInterfaceBase*>
    DistributionsInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Dnorm as an S4 object. To instantiate
 * from R:
 * dnorm_ <- new(fims$TMBDnormDistribution)
 *
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  Parameter x;
  Parameter mean;
  Parameter sd;

  DnormDistributionsInterface() : DistributionsInterfaceBase() {}

  uint32_t get_id() { return this->id; }

  virtual ~DnormDistributionsInterface() {}

  /**
   * @brief Evaluate normal probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */
  double evaluate(bool do_log) {
    fims::Dnorm<double> dnorm;
    dnorm.x = this->x.value;
    dnorm.mean = this->mean.value;
    dnorm.sd = this->sd.value;
    return dnorm.evaluate(true);
  }

  bool add_to_fims_tmb() {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_REAL_TYPE> > model0 =
        std::make_shared<fims::Dnorm<TMB_FIMS_REAL_TYPE> >();

    // interface to data/parameter value
    model0->id = this->id;
    model0->x = this->x.value;
    if (this->x.estimated) {
      if (this->x.is_random_effect) {
        d0->RegisterRandomEffect(model0->x);
      } else {
        d0->RegisterParameter(model0->x);
      }
    }

    model0->id = this->id;
    model0->mean = this->mean.value;
    if (this->mean.estimated) {
      if (this->mean.is_random_effect) {
        d0->RegisterRandomEffect(model0->mean);
      } else {
        d0->RegisterParameter(model0->mean);
      }
    }

    model0->id = this->id;
    model0->sd = this->sd.value;
    if (this->sd.estimated) {
      if (this->sd.is_random_effect) {
        d0->RegisterRandomEffect(model0->sd);
      } else {
        d0->RegisterParameter(model0->sd);
      }
    }
    d0->distribution_models[model0->id] = model0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_FIRST_ORDER> > model1 =
        std::make_shared<fims::Dnorm<TMB_FIMS_FIRST_ORDER> >();

    // interface to data/parameter first derivative
    model1->id = this->id;
    model1->x = this->x.value;
    if (this->x.estimated) {
      if (this->x.is_random_effect) {
        d1->RegisterRandomEffect(model1->x);
      } else {
        d1->RegisterParameter(model1->x);
      }
    }

    model1->id = this->id;
    model1->mean = this->mean.value;
    if (this->mean.estimated) {
      if (this->mean.is_random_effect) {
        d1->RegisterRandomEffect(model1->mean);
      } else {
        d1->RegisterParameter(model1->mean);
      }
    }

    model1->id = this->id;
    model1->sd = this->sd.value;
    if (this->sd.estimated) {
      if (this->sd.is_random_effect) {
        d1->RegisterRandomEffect(model1->sd);
      } else {
        d1->RegisterParameter(model1->sd);
      }
    }
    d1->distribution_models[model1->id] = model1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_SECOND_ORDER> > model2 =
        std::make_shared<fims::Dnorm<TMB_FIMS_SECOND_ORDER> >();

    // interface to data/parameter second derivative
    model2->id = this->id;
    model2->x = this->x.value;
    if (this->x.estimated) {
      if (this->x.is_random_effect) {
        d2->RegisterRandomEffect(model2->x);
      } else {
        d2->RegisterParameter(model2->x);
      }
    }

    model2->id = this->id;
    model2->mean = this->mean.value;
    if (this->mean.estimated) {
      if (this->mean.is_random_effect) {
        d2->RegisterRandomEffect(model2->mean);
      } else {
        d2->RegisterParameter(model2->mean);
      }
    }

    model2->id = this->id;
    model2->sd = this->sd.value;
    if (this->sd.estimated) {
      if (this->sd.is_random_effect) {
        d2->RegisterRandomEffect(model2->sd);
      } else {
        d2->RegisterParameter(model2->sd);
      }
    }
    d2->distribution_models[model2->id] = model2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_THIRD_ORDER> > model3 =
        std::make_shared<fims::Dnorm<TMB_FIMS_THIRD_ORDER> >();

    // interface to data/parameter third derivative
    model3->id = this->id;
    model3->x = this->x.value;
    if (this->x.estimated) {
      if (this->x.is_random_effect) {
        d3->RegisterRandomEffect(model3->x);
      } else {
        d3->RegisterParameter(model3->x);
      }
    }

    model3->id = this->id;
    model3->mean = this->mean.value;
    if (this->mean.estimated) {
      if (this->mean.is_random_effect) {
        d3->RegisterRandomEffect(model3->mean);
      } else {
        d3->RegisterParameter(model3->mean);
      }
    }

    model3->id = this->id;
    model3->sd = this->sd.value;
    if (this->sd.estimated) {
      if (this->sd.is_random_effect) {
        d3->RegisterRandomEffect(model3->sd);
      } else {
        d3->RegisterParameter(model3->sd);
      }
    }
    d3->distribution_models[model3->id] = model3;

    return true;
  }
};

#endif
