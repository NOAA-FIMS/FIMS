/*
 * File:   rcpp_growth.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_EWAA_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_EWAA_HPP

#include "../../../population_dynamics/growth/growth.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * EWAA Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief Rcpp interface for EWAAgrowth as an S4 object. To instantiate
 * from R:
 * ewaa <- new(fims$EWAAgrowth)
 *
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
 public:
  std::vector<double> weights;   /**< weights for each age class */
  std::vector<double> ages;      /**< ages for each age class */
  std::map<double, double> ewaa; /**< map of ewaa values */

  bool initialized = false; /**< boolean tracking if weights and ages
  vectors have been set */

  EWAAGrowthInterface() : GrowthInterfaceBase() {}

  virtual ~EWAAGrowthInterface() {}
  /** @brief get the id of the GrowthInterfaceBase object */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Create a map of input numeric vectors
   * @param weights T vector of weights
   * @param ages T vector of ages
   * @return std::map<T, T>
   *
   * */
  inline std::map<double, double> make_map(std::vector<double> ages,
                                           std::vector<double> weights) {
    std::map<double, double> mymap;
    for (uint32_t i = 0; i < ages.size(); i++) {
      mymap.insert(std::pair<double, double>(ages[i], weights[i]));
    }
    return mymap;
  }

  /** @brief Rcpp interface to the EWAAgrowth evaluate method
   * you can call from R using
   * ewaagrowth.evaluate(age)
   * */
  double evaluate(double age) {
    fims::EWAAgrowth<double> EWAAGrowth;

    if (initialized == false) {
      this->ewaa = make_map(this->ages, this->weights);
      // Check that ages and weights vector are the same length
      if (this->ages.size() != this->weights.size()) {
        Rcpp::stop("ages and weights must be the same length");
      }
      initialized = true;
    } else {
      Rcpp::stop("this empirical weight at age object is already initialized");
    }
    EWAAGrowth.ewaa = this->ewaa;
    return EWAAGrowth.evaluate(age);
  }

  /** @brief this adds the values to the TMB model object */
  virtual bool add_to_fims_tmb() {
    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_REAL_TYPE> > b0 =
        std::make_shared<fims::EWAAgrowth<TMB_FIMS_REAL_TYPE> >();

    // set relative info
    b0->id = this->id;
    b0->ewaa = this->ewaa;

    // add to Information
    d0->growth_models[b0->id] = b0;

    // add to Information
    d0->growth_models[b0->id] = b0;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_FIRST_ORDER> > b1 =
        std::make_shared<fims::EWAAgrowth<TMB_FIMS_FIRST_ORDER> >();

    // set relative info
    b1->id = this->id;
    b1->ewaa = this->ewaa;

    // add to Information
    d1->growth_models[b0->id] = b1;

    // add to Information
    d1->growth_models[b0->id] = b1;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_SECOND_ORDER> > b2 =
        std::make_shared<fims::EWAAgrowth<TMB_FIMS_SECOND_ORDER> >();

    // set relative info
    b2->id = this->id;
    b2->ewaa = this->ewaa;

    // add to Information
    d2->growth_models[b2->id] = b2;

    // add to Information
    d2->growth_models[b2->id] = b2;

    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::EWAAgrowth<TMB_FIMS_THIRD_ORDER> > b3 =
        std::make_shared<fims::EWAAgrowth<TMB_FIMS_THIRD_ORDER> >();

    // set relative info
    b3->id = this->id;
    b3->ewaa = this->ewaa;

    // add to Information
    d3->growth_models[b3->id] = b3;

    // add to Information
    d3->growth_models[b3->id] = b3;

    return true;
  }
};

#endif
