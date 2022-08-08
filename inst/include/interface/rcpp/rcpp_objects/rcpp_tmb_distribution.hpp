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
#include "rcpp_interface_base.hpp"
#include "../../interface.hpp"

/**
 * @brief Distributions Rcpp Interface
 *
 */
class DistributionsInterfaceBase : public FIMSRcppInterfaceBase
{
public:
  static uint32_t id_g;                                                 /**< static id of the DistributionsInterfaceBase object */
  uint32_t id;                                                          /**< local id of the DistributionsInterfaceBase object */
  static std::map<uint32_t, DistributionsInterfaceBase *> live_objects; /**<
  map relating the ID of the DistributionsInterfaceBase to the DistributionsInterfaceBase
  objects */

  DistributionsInterfaceBase()
  {
    this->id = DistributionsInterfaceBase::id_g++;
    DistributionsInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~DistributionsInterfaceBase() {}

  /** @brief get_id method for child distribution interface objects to inherit **/
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child distribution interface objects to inherit **/
  virtual double evaluate(bool do_log) = 0;
};

uint32_t DistributionsInterfaceBase::id_g = 1;   /**< static id of the DistributionsInterfaceBase object */
std::map<uint32_t, DistributionsInterfaceBase *> /**< local id of the DistributionsInterfaceBase object */
    DistributionsInterfaceBase::live_objects;    /**<
     map relating the ID of the DistributionsInterfaceBase to the DistributionsInterfaceBase
     objects */

/**
 * @brief Rcpp interface for Dnorm as an S4 object. To instantiate
 * from R:
 * dnorm_ <- new(fims$TMBDlnormDistribution)
 *
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase
{
public:
  Parameter x;    /**< observed data */
  Parameter mean; /**< mean of x for the normal distribution **/
  Parameter sd;   /**< sd of x for the normal distribution **/

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
  double evaluate(bool do_log)
  {
    fims::Dnorm<double> dnorm;
    dnorm.x = this->x.value;
    dnorm.mean = this->mean.value;
    dnorm.sd = this->sd.value;
    return dnorm.evaluate(do_log);
  }

  /**
   * @brief adds the dnorm distribution and its parameters to the TMB model
   */
  bool add_to_fims_tmb()
  {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_REAL_TYPE>> model0 =
        std::make_shared<fims::Dnorm<TMB_FIMS_REAL_TYPE>>();

    // interface to data/parameter value
    model0->id = this->id;
    model0->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d0->RegisterRandomEffect(model0->x);
      }
      else
      {
        d0->RegisterParameter(model0->x);
      }
    }

    // set relative info
    model0->mean = this->mean.value;
    if (this->mean.estimated)
    {
      if (this->mean.is_random_effect)
      {
        d0->RegisterRandomEffect(model0->mean);
      }
      else
      {
        d0->RegisterParameter(model0->mean);
      }
    }

    model0->sd = this->sd.value;
    if (this->sd.estimated)
    {
      if (this->sd.is_random_effect)
      {
        d0->RegisterRandomEffect(model0->sd);
      }
      else
      {
        d0->RegisterParameter(model0->sd);
      }
    }
    d0->distribution_models[model0->id] = model0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_FIRST_ORDER>> model1 =
        std::make_shared<fims::Dnorm<TMB_FIMS_FIRST_ORDER>>();

    // interface to data/parameter first derivative
    model1->id = this->id;
    model1->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d1->RegisterRandomEffect(model1->x);
      }
      else
      {
        d1->RegisterParameter(model1->x);
      }
    }

    model1->mean = this->mean.value;
    if (this->mean.estimated)
    {
      if (this->mean.is_random_effect)
      {
        d1->RegisterRandomEffect(model1->mean);
      }
      else
      {
        d1->RegisterParameter(model1->mean);
      }
    }

    model1->sd = this->sd.value;
    if (this->sd.estimated)
    {
      if (this->sd.is_random_effect)
      {
        d1->RegisterRandomEffect(model1->sd);
      }
      else
      {
        d1->RegisterParameter(model1->sd);
      }
    }
    d1->distribution_models[model1->id] = model1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_SECOND_ORDER>> model2 =
        std::make_shared<fims::Dnorm<TMB_FIMS_SECOND_ORDER>>();

    // interface to data/parameter second derivative
    model2->id = this->id;
    model2->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d2->RegisterRandomEffect(model2->x);
      }
      else
      {
        d2->RegisterParameter(model2->x);
      }
    }

    model2->mean = this->mean.value;
    if (this->mean.estimated)
    {
      if (this->mean.is_random_effect)
      {
        d2->RegisterRandomEffect(model2->mean);
      }
      else
      {
        d2->RegisterParameter(model2->mean);
      }
    }

    model2->sd = this->sd.value;
    if (this->sd.estimated)
    {
      if (this->sd.is_random_effect)
      {
        d2->RegisterRandomEffect(model2->sd);
      }
      else
      {
        d2->RegisterParameter(model2->sd);
      }
    }
    d2->distribution_models[model2->id] = model2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_THIRD_ORDER>> model3 =
        std::make_shared<fims::Dnorm<TMB_FIMS_THIRD_ORDER>>();

    // interface to data/parameter third derivative
    model3->id = this->id;
    model3->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d3->RegisterRandomEffect(model3->x);
      }
      else
      {
        d3->RegisterParameter(model3->x);
      }
    }

    model3->mean = this->mean.value;
    if (this->mean.estimated)
    {
      if (this->mean.is_random_effect)
      {
        d3->RegisterRandomEffect(model3->mean);
      }
      else
      {
        d3->RegisterParameter(model3->mean);
      }
    }

    model3->sd = this->sd.value;
    if (this->sd.estimated)
    {
      if (this->sd.is_random_effect)
      {
        d3->RegisterRandomEffect(model3->sd);
      }
      else
      {
        d3->RegisterParameter(model3->sd);
      }
    }
    d3->distribution_models[model3->id] = model3;

    return true;
  }
};

/**
 * @brief Rcpp interface for Dlnorm as an S4 object. To instantiate
 * from R:
 * dlnorm_ <- new(fims$TMBDnormDistribution)
 *
 */
class DlnormDistributionsInterface : public DistributionsInterfaceBase
{
public:
  Parameter x;       /*!< observation */
  Parameter meanlog; /*!< mean of the distribution of log(x) */
  Parameter sdlog;   /*!< standard deviation of the distribution of log(x) */

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
  double evaluate(bool do_log)
  {
    fims::Dlnorm<double> dlnorm;
    dlnorm.x = this->x.value;
    dlnorm.meanlog = this->meanlog.value;
    dlnorm.sdlog = this->sdlog.value;
    return dlnorm.evaluate(do_log);
  }

  /**
   * @brief adds the dlnorm distribution and its parameters to the TMB model
   */
  virtual bool add_to_fims_tmb()
  {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_REAL_TYPE>> model0 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_REAL_TYPE>>();

    // set relative info
    model0->id = this->id;
    model0->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d0->RegisterRandomEffect(model0->x);
      }
      else
      {
        d0->RegisterParameter(model0->x);
      }
    }
    model0->meanlog = this->meanlog.value;
    if (this->meanlog.estimated)
    {
      if (this->meanlog.is_random_effect)
      {
        d0->RegisterRandomEffect(model0->meanlog);
      }
      else
      {
        d0->RegisterParameter(model0->meanlog);
      }
    }
    model0->sdlog = this->sdlog.value;
    if (this->sdlog.estimated)
    {
      if (this->sdlog.is_random_effect)
      {
        d0->RegisterRandomEffect(model0->sdlog);
      }
      else
      {
        d0->RegisterParameter(model0->sdlog);
      }
    }
    // add to Information
    d0->distribution_models[model0->id] = model0;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_FIRST_ORDER>> model1 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_FIRST_ORDER>>();

    // set relative info
    model1->id = this->id;
    model1->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d1->RegisterRandomEffect(model1->x);
      }
      else
      {
        d1->RegisterParameter(model1->x);
      }
    }
    model1->meanlog = this->meanlog.value;
    if (this->meanlog.estimated)
    {
      if (this->meanlog.is_random_effect)
      {
        d1->RegisterRandomEffect(model1->meanlog);
      }
      else
      {
        d1->RegisterParameter(model1->meanlog);
      }
    }
    model1->sdlog = this->sdlog.value;
    if (this->sdlog.estimated)
    {
      if (this->sdlog.is_random_effect)
      {
        d1->RegisterRandomEffect(model1->sdlog);
      }
      else
      {
        d1->RegisterParameter(model1->sdlog);
      }
    }
    // add to Information
    d1->distribution_models[model1->id] = model1;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_SECOND_ORDER>> model2 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_SECOND_ORDER>>();

    // set relative info
    model2->id = this->id;
    model2->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d2->RegisterRandomEffect(model2->x);
      }
      else
      {
        d2->RegisterParameter(model2->x);
      }
    }
    model2->meanlog = this->meanlog.value;
    if (this->meanlog.estimated)
    {
      if (this->meanlog.is_random_effect)
      {
        d2->RegisterRandomEffect(model2->meanlog);
      }
      else
      {
        d2->RegisterParameter(model2->meanlog);
      }
    }
    model2->sdlog = this->sdlog.value;
    if (this->sdlog.estimated)
    {
      if (this->sdlog.is_random_effect)
      {
        d2->RegisterRandomEffect(model2->sdlog);
      }
      else
      {
        d2->RegisterParameter(model2->sdlog);
      }
    }
    // add to Information
    d2->distribution_models[model2->id] = model2;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_THIRD_ORDER>> model3 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_THIRD_ORDER>>();

    // set relative info
    model3->id = this->id;
    model3->x = this->x.value;
    if (this->x.estimated)
    {
      if (this->x.is_random_effect)
      {
        d3->RegisterRandomEffect(model3->x);
      }
      else
      {
        d3->RegisterParameter(model3->x);
      }
    }
    model3->meanlog = this->meanlog.value;
    if (this->meanlog.estimated)
    {
      if (this->meanlog.is_random_effect)
      {
        d3->RegisterRandomEffect(model3->meanlog);
      }
      else
      {
        d3->RegisterParameter(model3->meanlog);
      }
    }
    model3->sdlog = this->sdlog.value;
    if (this->sdlog.estimated)
    {
      if (this->sdlog.is_random_effect)
      {
        d3->RegisterRandomEffect(model3->sdlog);
      }
      else
      {
        d3->RegisterParameter(model3->sdlog);
      }
    }
    // add to Information
    d3->distribution_models[model3->id] = model3;

    return true;
  }
};

/**
 * @brief Rcpp interface for Dmultinom as an S4 object. To instantiate
 * from R:
 * dmultinom_ <- new(fims$TMBDmultinomDistribution)
 *
 */
template <typename T>
class DmultinomDistributionsInterface : public DistributionsInterfaceBase
{
  using Vector = typename ModelTraits<T>::EigenVector;
public:
  // std::vector<double> x;
  // std::vector<double> p;
  Vector x; /*!< Vector of length K of integers */
  Vector p; /*!< Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1). */                  

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
  double evaluate(bool do_log)
  {
    fims::Dmultinom<double> dmultinom;

    // this->dmultinom_xp = make_map(this->x, this->p);
    dmultinom.x = this->x;
    dmultinom.p = this->p;
    // dmultinom.dmultinom_xp = this->dmultinom_xp;
    return dmultinom.evaluate(do_log);
  }

  virtual bool add_to_fims_tmb()
  {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_REAL_TYPE>> model0 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_REAL_TYPE>>();

    model0->id = this->id;
    model0->x = this->x;
    model0->p = this->p;
    // model0->dmultinom_xp = this->dmultinom_xp;
    d0->distribution_models[model0->id] = model0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_FIRST_ORDER>> model1 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_FIRST_ORDER>>();

    model1->id = this->id;
    model1->x = this->x;
    model1->p = this->p;
    // model1->dmultinom_xp = this->dmultinom_xp;
    d1->distribution_models[model1->id] = model1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_SECOND_ORDER>> model2 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_SECOND_ORDER>>();

    model2->id = this->id;
    model2->x = this->x;
    model2->p = this->p;
    // model2->dmultinom_xp = this->dmultinom_xp;
    d2->distribution_models[model2->id] = model2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_THIRD_ORDER>> model3 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_THIRD_ORDER>>();

    model3->id = this->id;
    model3->x = this->x;
    model3->p = this->p;
    // model3->dmultinom_xp = this->dmultinom_xp;
    d3->distribution_models[model3->id] = model3;

    return true;
  }
};
#endif
