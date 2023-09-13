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

  // static std::vector<double> recruit_deviations; /**< vector of recruitment
  // deviations*/
  /// static bool constrain_deviations; /**< whether or not the rec devs are
  /// constrained*/

  RecruitmentInterfaceBase() {
    this->id = RecruitmentInterfaceBase::id_g++;
    RecruitmentInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~RecruitmentInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child recruitment interface objects to inherit
   * **/
  virtual double evaluate(double spawners, double ssbzero) = 0;

  /**
   * @brief evaluate recruitment nll
   *
   * @return double
   */
  virtual double evaluate_nll() = 0;
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
  Parameter logit_steep;       /**< steepness or the productivity of the stock*/
  Parameter log_rzero;         /**< recruitment at unfished biomass */
  Parameter log_sigma_recruit; /**< the log of the stock recruit deviations */
  Rcpp::NumericVector deviations; /**< recruitment deviations*/
  bool estimate_deviations =
      false; /**< boolean describing whether to estimate */

  BevertonHoltRecruitmentInterface() : RecruitmentInterfaceBase() {}

  virtual ~BevertonHoltRecruitmentInterface() {}

  virtual uint32_t get_id() { return this->id; }

  virtual double evaluate(double spawners, double ssbzero) {
    fims::SRBevertonHolt<double> BevHolt;

    BevHolt.logit_steep = this->logit_steep.value_m;
    if (this->logit_steep.value_m == 1.0) {
      warning(
          "Steepness is subject to a logit transformation, so its value is "
          "0.7848469. Fixing it at 1.0 is not currently possible.");
    }

    BevHolt.log_rzero = this->log_rzero.value_m;

    return BevHolt.evaluate(spawners, ssbzero);
  }

  virtual double evaluate_nll() {
    fims::SRBevertonHolt<double> NLL;

    NLL.log_sigma_recruit = this->log_sigma_recruit.value_m;
    NLL.recruit_deviations.resize(deviations.size());  // Vector from TMB
    for (int i = 0; i < deviations.size(); i++) {
      NLL.recruit_deviations[i] = deviations[i];
    }
    FIMS_LOG << "Rec devs being passed to C++ are " << deviations << std::endl;

    NLL.estimate_recruit_deviations = this->estimate_deviations;
    return NLL.evaluate_nll();
  }

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
    b0->logit_steep = this->logit_steep.value_m;
    if (this->logit_steep.estimated_m) {
      if (this->logit_steep.is_random_effect_m) {
        d0->RegisterRandomEffect(b0->logit_steep);
      } else {
        d0->RegisterParameter(b0->logit_steep);
      }
    }
    b0->log_rzero = this->log_rzero.value_m;
    if (this->log_rzero.estimated_m) {
      if (this->log_rzero.is_random_effect_m) {
        d0->RegisterRandomEffect(b0->log_rzero);
      } else {
        d0->RegisterParameter(b0->log_rzero);
      }
    }
    b0->log_sigma_recruit = this->log_sigma_recruit.value_m;
    if (this->log_sigma_recruit.estimated_m) {
      if (this->log_sigma_recruit.is_random_effect_m) {
        d0->RegisterRandomEffect(b0->log_sigma_recruit);
      } else {
        d0->RegisterParameter(b0->log_sigma_recruit);
      }
    }

    b0->recruit_deviations.resize(this->deviations.size());
    if (this->estimate_deviations) {
      for (size_t i = 0; i < b0->recruit_deviations.size(); i++) {
        b0->recruit_deviations[i] = this->deviations[i];
        d0->RegisterParameter(b0->recruit_deviations[i]);
      }
    } else {
      for (size_t i = 0; i < b0->recruit_deviations.size(); i++) {
        b0->recruit_deviations[i] = this->deviations[i];
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
    b1->logit_steep = this->logit_steep.value_m;
    if (this->logit_steep.estimated_m) {
      if (this->logit_steep.is_random_effect_m) {
        d1->RegisterRandomEffect(b1->logit_steep);
      } else {
        d1->RegisterParameter(b1->logit_steep);
      }
    }
    b1->log_rzero = this->log_rzero.value_m;
    if (this->log_rzero.estimated_m) {
      if (this->log_rzero.is_random_effect_m) {
        d1->RegisterRandomEffect(b1->log_rzero);
      } else {
        d1->RegisterParameter(b1->log_rzero);
      }
    }
    b1->log_sigma_recruit = this->log_sigma_recruit.value_m;
    if (this->log_sigma_recruit.estimated_m) {
      if (this->log_sigma_recruit.is_random_effect_m) {
        d1->RegisterRandomEffect(b1->log_sigma_recruit);
      } else {
        d1->RegisterParameter(b1->log_sigma_recruit);
      }
    }

    b1->recruit_deviations.resize(this->deviations.size());
    if (this->estimate_deviations) {
      for (size_t i = 0; i < b1->recruit_deviations.size(); i++) {
        b1->recruit_deviations[i] = this->deviations[i];
        d1->RegisterParameter(b1->recruit_deviations[i]);
      }
    } else {
      for (size_t i = 0; i < b1->recruit_deviations.size(); i++) {
        b1->recruit_deviations[i] = this->deviations[i];
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
    b2->logit_steep = this->logit_steep.value_m;
    if (this->logit_steep.estimated_m) {
      if (this->logit_steep.is_random_effect_m) {
        d2->RegisterRandomEffect(b2->logit_steep);
      } else {
        d2->RegisterParameter(b2->logit_steep);
      }
    }
    b2->log_rzero = this->log_rzero.value_m;
    if (this->log_rzero.estimated_m) {
      if (this->log_rzero.is_random_effect_m) {
        d2->RegisterRandomEffect(b2->log_rzero);
      } else {
        d2->RegisterParameter(b2->log_rzero);
      }
    }
    b2->log_sigma_recruit = this->log_sigma_recruit.value_m;
    if (this->log_sigma_recruit.estimated_m) {
      if (this->log_sigma_recruit.is_random_effect_m) {
        d2->RegisterRandomEffect(b2->log_sigma_recruit);
      } else {
        d2->RegisterParameter(b2->log_sigma_recruit);
      }
    }

    b2->recruit_deviations.resize(this->deviations.size());
    if (this->estimate_deviations) {
      for (size_t i = 0; i < b2->recruit_deviations.size(); i++) {
        b2->recruit_deviations[i] = this->deviations[i];
        d2->RegisterParameter(b2->recruit_deviations[i]);
      }
    } else {
      for (size_t i = 0; i < b2->recruit_deviations.size(); i++) {
        b2->recruit_deviations[i] = this->deviations[i];
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
    b3->logit_steep = this->logit_steep.value_m;
    if (this->logit_steep.estimated_m) {
      if (this->logit_steep.is_random_effect_m) {
        d3->RegisterRandomEffect(b3->logit_steep);
      } else {
        d3->RegisterParameter(b3->logit_steep);
      }
    }
    b3->log_rzero = this->log_rzero.value_m;
    if (this->log_rzero.estimated_m) {
      if (this->log_rzero.is_random_effect_m) {
        d3->RegisterRandomEffect(b3->log_rzero);
      } else {
        d3->RegisterParameter(b3->log_rzero);
      }
    }
    b3->log_sigma_recruit = this->log_sigma_recruit.value_m;
    if (this->log_sigma_recruit.estimated_m) {
      if (this->log_sigma_recruit.is_random_effect_m) {
        d3->RegisterRandomEffect(b3->log_sigma_recruit);
      } else {
        d3->RegisterParameter(b3->log_sigma_recruit);
      }
    }

    b3->recruit_deviations.resize(this->deviations.size());
    if (this->estimate_deviations) {
      for (size_t i = 0; i < b3->recruit_deviations.size(); i++) {
        b3->recruit_deviations[i] = this->deviations[i];
        d3->RegisterParameter(b3->recruit_deviations[i]);
      }
    } else {
      for (size_t i = 0; i < b3->recruit_deviations.size(); i++) {
        b3->recruit_deviations[i] = this->deviations[i];
      }
    }

    // add to Information
    d3->recruitment_models[b3->id] = b3;

    return true;
  }
};

#endif
