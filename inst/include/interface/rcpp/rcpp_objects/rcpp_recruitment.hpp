/*
 * File:   rcpp_recruitment.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP

#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * Recruitment Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief RecruitmentInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Recruitment function
 * */
class RecruitmentInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  static std::map<uint32_t, RecruitmentInterfaceBase*> live_objects;
  /**< map associating the ids of RecruitmentInterfaceBase to the objects */

  RecruitmentInterfaceBase() {
    this->id = RecruitmentInterfaceBase::id_g++;
    RecruitmentInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~RecruitmentInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() = 0;
};

uint32_t RecruitmentInterfaceBase::id_g = 1;
std::map<uint32_t, RecruitmentInterfaceBase*>
    RecruitmentInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Beverton-Holt as an S4 object. To instantiate
 * from R:
 * beverton_holt <- new(fims$beverton_holt)
 */
class BevertonHoltRecruitmentInterface : public RecruitmentInterfaceBase {
 public:
  Parameter steep;   /**< steepness or the productivity of the stock*/
  Parameter rzero;   /**< recruitment at unfished biomass */
  Parameter phizero; /**< unfished spawning biomass per recruit */

  BevertonHoltRecruitmentInterface() : RecruitmentInterfaceBase() {}

  virtual ~BevertonHoltRecruitmentInterface() {}

  virtual uint32_t get_id() { return this->id; }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_REAL_TYPE> > b0 =
        std::make_shared<fims::SRBevertonHolt<TMB_FIMS_REAL_TYPE> >();

    // set relative info
    b0->id = this->id;
    b0->steep = this->steep.value;
    if (this->steep.estimated) {
      if (this->steep.is_random_effect) {
        d0->RegisterRandomEffect(b0->steep);
      } else {
        d0->RegisterParameter(b0->steep);
      }
    }
    b0->rzero = this->rzero.value;
    if (this->rzero.estimated) {
      if (this->rzero.is_random_effect) {
        d0->RegisterRandomEffect(b0->rzero);
      } else {
        d0->RegisterParameter(b0->rzero);
      }
    }
    b0->phizero = this->phizero.value;
    if (this->phizero.estimated) {
      if (this->phizero.is_random_effect) {
        d0->RegisterRandomEffect(b0->phizero);
      } else {
        d0->RegisterParameter(b0->phizero);
      }
    }
    // add to Information
    d0->recruitment_models[b0->id] = b0;

    // first-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_FIRST_ORDER> > b1 =
        std::make_shared<fims::SRBevertonHolt<TMB_FIMS_FIRST_ORDER> >();

    // set relative info
    b1->id = this->id;
    b1->steep = this->steep.value;
    if (this->steep.estimated) {
      if (this->steep.is_random_effect) {
        d1->RegisterRandomEffect(b1->steep);
      } else {
        d1->RegisterParameter(b1->steep);
      }
    }
    b1->rzero = this->rzero.value;
    if (this->rzero.estimated) {
      if (this->rzero.is_random_effect) {
        d1->RegisterRandomEffect(b1->rzero);
      } else {
        d1->RegisterParameter(b1->rzero);
      }
    }
    b1->phizero = this->phizero.value;
    if (this->phizero.estimated) {
      if (this->phizero.is_random_effect) {
        d1->RegisterRandomEffect(b1->phizero);
      } else {
        d1->RegisterParameter(b1->phizero);
      }
    }
    // add to Information
    d1->recruitment_models[b1->id] = b1;

    // second-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> > b2 =
        std::make_shared<fims::SRBevertonHolt<TMB_FIMS_SECOND_ORDER> >();

    // set relative info
    b2->id = this->id;
    b2->steep = this->steep.value;
    if (this->steep.estimated) {
      if (this->steep.is_random_effect) {
        d2->RegisterRandomEffect(b2->steep);
      } else {
        d2->RegisterParameter(b2->steep);
      }
    }
    b2->rzero = this->rzero.value;
    if (this->rzero.estimated) {
      if (this->rzero.is_random_effect) {
        d2->RegisterRandomEffect(b2->rzero);
      } else {
        d2->RegisterParameter(b2->rzero);
      }
    }
    b2->phizero = this->phizero.value;
    if (this->phizero.estimated) {
      if (this->phizero.is_random_effect) {
        d2->RegisterRandomEffect(b2->phizero);
      } else {
        d2->RegisterParameter(b2->phizero);
      }
    }
    // add to Information
    d2->recruitment_models[b2->id] = b2;

    // third-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> > b3 =
        std::make_shared<fims::SRBevertonHolt<TMB_FIMS_THIRD_ORDER> >();

    // set relative info
    b3->id = this->id;
    b3->steep = this->steep.value;
    if (this->steep.estimated) {
      if (this->steep.is_random_effect) {
        d3->RegisterRandomEffect(b3->steep);
      } else {
        d3->RegisterParameter(b3->steep);
      }
    }
    b3->rzero = this->rzero.value;
    if (this->rzero.estimated) {
      if (this->rzero.is_random_effect) {
        d3->RegisterRandomEffect(b3->rzero);
      } else {
        d3->RegisterParameter(b3->rzero);
      }
    }
    b3->phizero = this->phizero.value;
    if (this->phizero.estimated) {
      if (this->phizero.is_random_effect) {
        d3->RegisterRandomEffect(b3->phizero);
      } else {
        d3->RegisterParameter(b3->phizero);
      }
    }
    // add to Information
    d3->recruitment_models[b3->id] = b3;

    return true;
  }
};

#endif
