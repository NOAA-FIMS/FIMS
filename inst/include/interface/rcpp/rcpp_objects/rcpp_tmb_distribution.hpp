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
  static std::map<uint32_t, DistributionsInterfaceBase *> live_objects; /**<
  map relating the ID of the DistributionsInterfaceBase to the
  DistributionsInterfaceBase objects */

  DistributionsInterfaceBase() {
    this->id = DistributionsInterfaceBase::id_g++;
    DistributionsInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~DistributionsInterfaceBase() {}

  /** @brief get_id method for child distribution interface objects to inherit
   * **/
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child distribution interface objects to inherit
   * **/
  virtual double evaluate(bool do_log) = 0;
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
 * dnorm_ <- new(fims$TMBDnormDistribution)
 *
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  Parameter x;    /**< observed data */
  Parameter mean; /**< mean of x for the normal distribution **/
  Parameter sd;   /**< sd of x for the normal distribution **/

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
  virtual double evaluate(bool do_log) {
    fims::Dnorm<double> dnorm;
    dnorm.x = this->x.value;
    dnorm.mean = this->mean.value;
    dnorm.sd = this->sd.value;
    return dnorm.evaluate(do_log);
  }

  /**
   * @brief adds the dnorm distribution and its parameters to the TMB model
   */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_REAL_TYPE>> model0 =
        std::make_shared<fims::Dnorm<TMB_FIMS_REAL_TYPE>>();

    // interface to data/parameter value
    model0->id = this->id;
    model0->x = this->x.value;
    // set relative info
    model0->mean = this->mean.value;
    model0->sd = this->sd.value;

    d0->distribution_models[model0->id] = model0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_FIRST_ORDER>> model1 =
        std::make_shared<fims::Dnorm<TMB_FIMS_FIRST_ORDER>>();

    // interface to data/parameter first derivative
    model1->id = this->id;
    model1->x = this->x.value;
    model1->mean = this->mean.value;
    model1->sd = this->sd.value;

    d1->distribution_models[model1->id] = model1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_SECOND_ORDER>> model2 =
        std::make_shared<fims::Dnorm<TMB_FIMS_SECOND_ORDER>>();

    // interface to data/parameter second derivative
    model2->id = this->id;
    model2->x = this->x.value;
    model2->mean = this->mean.value;
    model2->sd = this->sd.value;

    d2->distribution_models[model2->id] = model2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dnorm<TMB_FIMS_THIRD_ORDER>> model3 =
        std::make_shared<fims::Dnorm<TMB_FIMS_THIRD_ORDER>>();

    // interface to data/parameter third derivative
    model3->id = this->id;
    model3->x = this->x.value;
    model3->mean = this->mean.value;
    model3->sd = this->sd.value;

    d3->distribution_models[model3->id] = model3;

    return true;
  }
};

/**
 * @brief Rcpp interface for Dlnorm as an S4 object. To instantiate
 * from R:
 * dlnorm_ <- new(fims$TMBDlnormDistribution)
 *
 */
class DlnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  Parameter x;       /*!< observation */
  Parameter meanlog; /*!< mean of the distribution of log(x) */
  Parameter sdlog;   /*!< standard deviation of the distribution of log(x) */
  bool do_bias_correction; /*!< true if the lognormal should be bias corrected,
                              default FALSE */

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
  virtual double evaluate(bool do_log) {
    fims::Dlnorm<double> dlnorm;
    dlnorm.x = this->x.value;
    dlnorm.meanlog = this->meanlog.value;
    dlnorm.sdlog = this->sdlog.value;
    dlnorm.do_bias_correction = this->do_bias_correction;
    return dlnorm.evaluate(do_log);
  }

  /**
   * @brief adds the dlnorm distribution and its parameters to the TMB model
   */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_REAL_TYPE>> model0 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_REAL_TYPE>>();

    // set relative info
    model0->id = this->id;
    model0->x = this->x.value;
    model0->meanlog = this->meanlog.value;
    model0->sdlog = this->sdlog.value;

    d0->distribution_models[model0->id] = model0;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_FIRST_ORDER>> model1 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_FIRST_ORDER>>();

    // set relative info
    model1->id = this->id;
    model1->x = this->x.value;
    model1->meanlog = this->meanlog.value;
    model1->sdlog = this->sdlog.value;

    d1->distribution_models[model1->id] = model1;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_SECOND_ORDER>> model2 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_SECOND_ORDER>>();

    // set relative info
    model2->id = this->id;
    model2->x = this->x.value;
    model2->meanlog = this->meanlog.value;
    model2->sdlog = this->sdlog.value;

    d2->distribution_models[model2->id] = model2;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dlnorm<TMB_FIMS_THIRD_ORDER>> model3 =
        std::make_shared<fims::Dlnorm<TMB_FIMS_THIRD_ORDER>>();

    // set relative info
    model3->id = this->id;
    model3->x = this->x.value;
    model3->meanlog = this->meanlog.value;
    model3->sdlog = this->sdlog.value;

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
// template <typename T>
class DmultinomDistributionsInterface : public DistributionsInterfaceBase {
 public:
  Rcpp::NumericVector x; /*!< Vector of length K of integers */
  Rcpp::NumericVector p; /*!< Vector of length K, specifying the probability
   for the K classes (note, unlike in R these must sum to 1). */

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
  virtual double evaluate(bool do_log) {
    fims::Dmultinom<double> dmultinom;
    // Decale TMBVector in this scope
    typedef
        typename fims::ModelTraits<TMB_FIMS_REAL_TYPE>::EigenVector TMBVector;
    dmultinom.x = TMBVector(x.size());  // Vector from TMB
    dmultinom.p = TMBVector(p.size());  // Vector from TMB
    for (int i = 0; i < x.size(); i++) {
      dmultinom.x[i] = x[i];
      dmultinom.p[i] = p[i];
    }
    return dmultinom.evaluate(do_log);
  }

  virtual bool add_to_fims_tmb() {
    typedef typename fims::ModelTraits<TMB_FIMS_REAL_TYPE>::EigenVector Vector0;
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_REAL_TYPE>> model0 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_REAL_TYPE>>();

    model0->id = this->id;
    model0->x = Vector0(x.size());
    model0->p = Vector0(p.size());

    for (int i = 0; i < x.size(); i++) {
      model0->x[i] = x[i];
      model0->p[i] = p[i];
    }

    d0->distribution_models[model0->id] = model0;

    typedef
        typename fims::ModelTraits<TMB_FIMS_FIRST_ORDER>::EigenVector Vector1;
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_FIRST_ORDER>> model1 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_FIRST_ORDER>>();

    model1->id = this->id;
    model1->x = Vector1(x.size());
    model1->p = Vector1(p.size());

    for (int i = 0; i < x.size(); i++) {
      model1->x[i] = x[i];
      model1->p[i] = p[i];
    }

    d1->distribution_models[model1->id] = model1;

    typedef
        typename fims::ModelTraits<TMB_FIMS_SECOND_ORDER>::EigenVector Vector2;
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_SECOND_ORDER>> model2 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_SECOND_ORDER>>();

    model2->id = this->id;
    model2->x = Vector2(x.size());
    model2->p = Vector2(p.size());

    for (int i = 0; i < x.size(); i++) {
      model2->x[i] = x[i];
      model2->p[i] = p[i];
    }

    d2->distribution_models[model2->id] = model2;

    typedef
        typename fims::ModelTraits<TMB_FIMS_THIRD_ORDER>::EigenVector Vector3;
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Dmultinom<TMB_FIMS_THIRD_ORDER>> model3 =
        std::make_shared<fims::Dmultinom<TMB_FIMS_THIRD_ORDER>>();

    model3->id = this->id;
    model3->x = Vector3(x.size());
    model3->p = Vector3(p.size());

    for (int i = 0; i < x.size(); i++) {
      model3->x[i] = x[i];
      model3->p[i] = p[i];
    }

    d3->distribution_models[model3->id] = model3;

    return true;
  }
};
#endif
