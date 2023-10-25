/*
 * File:   rcpp_fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "../../../common/def.hpp"
#include "../../../population_dynamics/fleet/fleet.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for
 * Rcpp fleet interfaces. This type should be inherited and not
 * called from R directly.
 *
 */
class FleetInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the FleetInterfaceBase object */
  uint32_t id;          /**< local id of the FleetInterfaceBase object */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, FleetInterfaceBase*> live_objects; /**<
map relating the ID of the FleetInterfaceBase to the FleetInterfaceBase
objects */

  FleetInterfaceBase() {
    this->id = FleetInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    FleetInterfaceBase */
    FleetInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~FleetInterfaceBase() {}

  /** @brief get_id method for child fleet interface objects to inherit **/
  virtual uint32_t get_id() = 0;
};

uint32_t FleetInterfaceBase::id_g = 1;
std::map<uint32_t, FleetInterfaceBase*> FleetInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Fleet as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Fleet)
 *
 */
class FleetInterface : public FleetInterfaceBase {
  int agecomp_likelihood_id = -999;    /*!< id of agecomp likelihood component*/
  int index_likelihood_id = -999;      /*!< id of index likelihood component*/
  int observed_agecomp_data_id = -999; /*!< id of observed agecomp data object*/
  int observed_index_data_id = -999;   /*!< id of observed index data object*/
  int selectivity_id = -999;           /*!< id of selectivity component*/

 public:
  bool is_survey = false; /*!< whether this is a survey fleet */
  int nages;              /*!< number of ages in the fleet data*/
  int nyears;             /*!< number of years in the fleet data */
  double log_q;           /*!< log of catchability for the fleet*/
  Rcpp::NumericVector
      log_Fmort;           /*!< log of fishing mortality rate for the fleet*/
  bool estimate_F = false; /*!< whether the parameter F should be estimated*/
  bool estimate_q = false; /*!< whether the parameter q should be estimated*/
  bool random_q = false;   /*!< whether q should be a random effect*/
  bool random_F = false;   /*!< whether F should be a random effect*/
  Parameter log_obs_error; /*!< the log of the observation error */

  FleetInterface() : FleetInterfaceBase() {}

  virtual ~FleetInterface() {}

  /** @brief returns the id for the fleet interface */
  virtual uint32_t get_id() { return this->id; }

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

  /**
 * @brief Evaluate index nll
 * 
*/
virtual double evaluate_index_nll(){
  fims::Fleet<double> fleet;
  fleet.log_obs_error = log_obs_error.value_m;
  return(fleet.evaluate_index_nll());
}


  /**
 * @brief Evaluate agecomp nll
 * 
*/
virtual double evaluate_age_comp_nll(){
  fims::Fleet<double> fleet;
//  fleet.catch_numbers_at_age = 0.0;
//  fleet.observed_agecomp_data = 0.0;

  return(fleet.evaluate_age_comp_nll());
}


#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims::Information<Type> > info =
        fims::Information<Type>::GetInstance();

    std::shared_ptr<fims::Fleet<Type> > fleet =
        std::make_shared<fims::Fleet<Type> >();

    // set relative info
    fleet->id = this->id;
    fleet->is_survey = this->is_survey;
    fleet->nages = this->nages;
    fleet->nyears = this->nyears;
    fleet->agecomp_likelihood_id = this->agecomp_likelihood_id;
    fleet->index_likelihood_id = this->index_likelihood_id;
    fleet->observed_agecomp_data_id = this->observed_agecomp_data_id;
    fleet->observed_index_data_id = this->observed_index_data_id;
    fleet->selectivity_id = this->selectivity_id;

    fleet->log_obs_error = this->log_obs_error.value_m;
    if (this->log_obs_error.estimated_m) {
      info->RegisterParameter(fleet->log_obs_error);
    }
    fleet->log_q = this->log_q;
    if (this->estimate_q) {
      if (this->random_q) {
        info->RegisterRandomEffect(fleet->log_q);
      } else {
        info->RegisterParameter(fleet->log_q);
      }
    }

    fleet->log_Fmort.resize(this->log_Fmort.size());
    for (int i = 0; i < log_Fmort.size(); i++) {
      fleet->log_Fmort[i] = this->log_Fmort[i];

      if (this->estimate_F) {
        if (this->random_F) {
          info->RegisterRandomEffect(fleet->log_Fmort[i]);
        } else {
          info->RegisterParameter(fleet->log_Fmort[i]);
        }
      }
    }
    // add to Information
    info->fleets[fleet->id] = fleet;

    return true;
  }

  /** @brief this adds the values to the TMB model object */
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
