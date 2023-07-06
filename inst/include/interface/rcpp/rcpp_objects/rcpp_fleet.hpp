/*
 * File:   rcpp_fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "../../../population_dynamics/fleet/fleet.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../common/def.hpp"

/**
 * @brief Rcpp interface for Fleet as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Fleet)
 *
 */
class FleetInterface : public FIMSRcppInterfaceBase {
  int agecomp_likelihood_id = -999;    /*!< id of agecomp likelihood component*/
  int index_likelihood_id = -999;      /*!< id of index likelihood component*/
  int observed_agecomp_data_id = -999; /*!< id of observed agecomp data object*/
  int observed_index_data_id = -999;   /*!< id of observed index data object*/
  int selectivity_id = -999;           /*!< id of selectivity component*/

 public:
  bool is_survey = false;     /*!< whether this is a survey fleet */
  int nages;                 /*!< number of ages in the fleet data*/
  int nyears;                /*!< number of years in the fleet data */
  double log_q; /*!< log of catchability for the fleet*/
  Rcpp::NumericVector
      log_Fmort;           /*!< log of fishing mortality rate for the fleet*/
  bool estimate_F = false; /*!< whether the parameter F should be estimated*/
  bool estimate_q = false; /*!< whether the parameter q should be estimated*/
  bool random_q = false;   /*!< whether q should be a random effect*/
  bool random_F = false;   /*!< whether F should be a random effect*/
  Parameter log_obs_error; /*!< the log of the observation error */

 public:
  static uint32_t id_g; /**< static id of the FleetInterface object */
  uint32_t id;          /**< local id of the FleetInterface object */

  FleetInterface() { 
    this->id = FleetInterface::id_g++; 
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

  virtual ~FleetInterface() {}

  /**
   * @brief Set the unique id for the Age Comp Likelihood object
   *
   * @param agecomp_likelihood_id Unique id for the Age Comp Likelihood object
   */
  void SetAgeCompLikelihood(int agecomp_likelihood_id) {
    this->agecomp_likelihood_id = agecomp_likelihood_id;
  }

  /**
   * @brief Set the unique id for the Index Likelihood object
   *
   * @param index_likelihood_id Unique id for the Index Likelihood object
   */
  void SetIndexLikelihood(int index_likelihood_id) {

    this->index_likelihood_id = index_likelihood_id;
  }

  /**
   * @brief Set the unique id for the Observed Age Comp Data object
   *
   * @param observed_agecomp_data_id Unique id for the Observed Age Comp Data
   * object
   */
  void SetObservedAgeCompData(int observed_agecomp_data_id) {
    this->observed_agecomp_data_id = observed_agecomp_data_id;
  }

  /**
   * @brief Set the unique id for the Observed Index Data object
   *
   * @param observed_index_data_id Unique id for the Observed Index Data object
   */
  void SetObservedIndexData(int observed_index_data_id) {
    this->observed_index_data_id = observed_index_data_id;
  }

  /**
   * @brief Set the unique id for the Selectivity object
   *
   * @param selectivity_id Unique id for the Selectivity object
   */
  void SetSelectivity(int selectivity_id) {
    this->selectivity_id = selectivity_id;
  }

  /** @brief this adds the values to the TMB model object */
  virtual bool add_to_fims_tmb() {
    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Fleet<TMB_FIMS_REAL_TYPE>> f0 =
        std::make_shared<fims::Fleet<TMB_FIMS_REAL_TYPE>>();

    // set relative info
    f0->id = this->id;
    f0->is_survey = this->is_survey;
    f0->nages = this->nages;
    f0->nyears = this-> nyears;
    f0->agecomp_likelihood_id = this->agecomp_likelihood_id;
    f0->index_likelihood_id = this->index_likelihood_id;
    f0->observed_agecomp_data_id = this->observed_agecomp_data_id;
    f0->observed_index_data_id = this->observed_index_data_id;
    f0->selectivity_id = this->selectivity_id;
    
    f0->log_obs_error = this->log_obs_error.value;
    if(this->log_obs_error.estimated){
      d0->RegisterParameter(f0->log_obs_error);
    }
      f0->log_q = this->log_q;
      if (this->estimate_q) {
        if (this->random_q) {
          d0->RegisterRandomEffect(f0->log_q);
        } else {
          d0->RegisterParameter(f0->log_q);
        }
      }

    f0->log_Fmort.resize(this->log_Fmort.size());
    for (int i = 0; i < log_Fmort.size(); i++) {
      f0->log_Fmort[i] = this->log_Fmort[i];
      
      if (this->estimate_F) {
        if (this->random_F) {
          d0->RegisterRandomEffect(f0->log_Fmort[i]);
        } else {
          d0->RegisterParameter(f0->log_Fmort[i]);
        }
      }
    }
    // add to Information
    d0->fleets[f0->id] = f0;

    // 1st derivative model
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Fleet<TMB_FIMS_FIRST_ORDER>> f1 =
        std::make_shared<fims::Fleet<TMB_FIMS_FIRST_ORDER>>();

    f1->id = this->id;
    f1->is_survey = this->is_survey;
    f1->nages = this->nages;
    f1->nyears = this-> nyears;
    f1->agecomp_likelihood_id = this->agecomp_likelihood_id;
    f1->index_likelihood_id = this->index_likelihood_id;
    f1->observed_agecomp_data_id = this->observed_agecomp_data_id;
    f1->observed_index_data_id = this->observed_index_data_id;
    f1->log_obs_error = this->log_obs_error.value;
        if(this->log_obs_error.estimated){
    d1->RegisterParameter(f1->log_obs_error);
        }
    f1->selectivity_id = this->selectivity_id;
    
      f1->log_q = this->log_q;
      if (this->estimate_q) {
        if (this->random_q) {
          d1->RegisterRandomEffect(f1->log_q);
        } else {
          d1->RegisterParameter(f1->log_q);
        }
      }
    
    f1->log_Fmort.resize(this->log_Fmort.size());
    for (int i = 0; i < log_Fmort.size(); i++) {
      f1->log_Fmort[i] = this->log_Fmort[i];
      if (this->estimate_F) {
        if (this->random_F) {
          d1->RegisterRandomEffect(f1->log_Fmort[i]);
        } else {
          d1->RegisterParameter(f1->log_Fmort[i]);
        }
      }
    }


    // add to Information
    d1->fleets[f1->id] = f1;

    // 2nd derivative model
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Fleet<TMB_FIMS_SECOND_ORDER>> f2 =
        std::make_shared<fims::Fleet<TMB_FIMS_SECOND_ORDER>>();

    f2->id = this->id;
    f2->is_survey = this->is_survey;
    f2->nages = this->nages;
    f2->nyears = this-> nyears;
    f2->agecomp_likelihood_id = this->agecomp_likelihood_id;
    f2->index_likelihood_id = this->index_likelihood_id;
    f2->observed_agecomp_data_id = this->observed_agecomp_data_id;
    f2->observed_index_data_id = this->observed_index_data_id;
    f2->log_obs_error = this->log_obs_error.value;
        if(this->log_obs_error.estimated){
           d2->RegisterParameter(f2->log_obs_error);
    
        }
    f2->selectivity_id = this->selectivity_id;
      f2->log_q = this->log_q;
      if (this->estimate_q) {
        if (this->random_q) {
          d2->RegisterRandomEffect(f2->log_q);
        } else {
          d2->RegisterParameter(f2->log_q);
        }
      }

    f2->log_Fmort.resize(this->log_Fmort.size());
    for (int i = 0; i < log_Fmort.size(); i++) {
      f2->log_Fmort[i] = this->log_Fmort[i];
      if (this->estimate_F) {
        if (this->random_F) {
          d2->RegisterRandomEffect(f2->log_Fmort[i]);
        } else {
          d2->RegisterParameter(f2->log_Fmort[i]);
        }
      }
    }

    // add to Information
    d2->fleets[f2->id] = f2;

    // 3rd derivative model
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Fleet<TMB_FIMS_THIRD_ORDER>> f3 =
        std::make_shared<fims::Fleet<TMB_FIMS_THIRD_ORDER>>();

    f3->id = this->id;
    f3->is_survey = this->is_survey;
    f3->nages = this->nages;
    f3->nyears = this-> nyears;
    f3->agecomp_likelihood_id = this->agecomp_likelihood_id;
    f3->index_likelihood_id = this->index_likelihood_id;
    f3->observed_agecomp_data_id = this->observed_agecomp_data_id;
    f3->observed_index_data_id = this->observed_index_data_id;
    f3->selectivity_id = this->selectivity_id;
    f3->log_obs_error = this->log_obs_error.value;
    if(this->log_obs_error.estimated){
          d3->RegisterParameter(f3->log_obs_error);
    }
      f3->log_q = this->log_q;
      if (this->estimate_q) {
        if (this->random_q) {
          d3->RegisterRandomEffect(f3->log_q);
        } else {
          d3->RegisterParameter(f3->log_q);
        }
      }

    f3->log_Fmort.resize(this->log_Fmort.size());
    for (int i = 0; i < log_Fmort.size(); i++) {
    f3->log_Fmort[i] = this->log_Fmort[i];
      if (this->estimate_F) {
        if (this->random_F) {
          d3->RegisterRandomEffect(f3->log_Fmort[i]);
        } else {
          d3->RegisterParameter(f3->log_Fmort[i]);
        }
      }
    }

    // add to Information
    d3->fleets[f3->id] = f3;
    return true;
  }
};

uint32_t FleetInterface::id_g = 1;

#endif
