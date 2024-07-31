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

  /** @brief get_id method for child fleet interface objects to inherit */
  virtual uint32_t get_id() = 0;
};

uint32_t FleetInterfaceBase::id_g = 1;
std::map<uint32_t, FleetInterfaceBase*> FleetInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Fleet as an S4 object. To instantiate
 * from R:
 * fleet <- new(Fleet)
 *
 */
class FleetInterface : public FleetInterfaceBase {
  int interface_selectivity_id_m = -999; /**< id of selectivity component*/

 public:
  bool is_survey = false; /**< whether this is a survey fleet */
  int nages;              /**< number of ages in the fleet data*/
  int nyears;             /**< number of years in the fleet data */
  double log_q;           /**< log of catchability for the fleet*/
  ParameterVector
      log_Fmort;           /**< log of fishing mortality rate for the fleet*/
  ParameterVector log_expected_index; /**< expected index of abundance for the survey */
  ParameterVector proportion_catch_numbers_at_age; /**< expected catch numbers at age for the fleet */
  bool estimate_q = false; /**< whether the parameter q should be estimated*/
  bool random_q = false;             /**< whether q should be a random effect*/

  FleetInterface() : FleetInterfaceBase() {}

  virtual ~FleetInterface() {}

  /** @brief returns the id for the fleet interface */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Set the unique id for the Selectivity object
   *
   * @param selectivity_id Unique id for the Selectivity object
   */
  void SetSelectivity(int selectivity_id) {
    interface_selectivity_id_m = selectivity_id;
  }

#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::Fleet<Type> > fleet =
        std::make_shared<fims_popdy::Fleet<Type> >();

    // set relative info
    fleet->id = this->id;
    fleet->is_survey = this->is_survey;
    fleet->nages = this->nages;
    fleet->nyears = this->nyears;
    fleet->fleet_selectivity_id_m = interface_selectivity_id_m;

    fleet->log_q = this->log_q;
    if (this->estimate_q) {
      info->RegisterParameterName("log_q");
      if (this->random_q) {
        info->RegisterRandomEffect(fleet->log_q);
      } else {
        info->RegisterParameter(fleet->log_q);
      }
    }

    fleet->log_Fmort.resize(this->log_Fmort.size());
    for (size_t i = 0; i < log_Fmort.size(); i++) {
      fleet->log_Fmort[i] = this->log_Fmort[i].value_m;

      if (this->log_Fmort[i].estimated_m) {
        info->RegisterParameterName("log_Fmort");
        if (this->log_Fmort[i].is_random_effect_m) {
          info->RegisterRandomEffect(fleet->log_Fmort[i]);
        } else {
          info->RegisterParameter(fleet->log_Fmort[i]);
        }
      }
    }
    //add to variable_map
    info->variable_map[this->log_Fmort.id_m] = &(fleet)->log_Fmort;

    //exp_catch
    fleet->log_expected_index.resize(nyears);  // assume index is for all ages.
    info->variable_map[this->log_expected_index.id_m] = &(fleet)->log_expected_index;
    fleet->proportion_catch_numbers_at_age.resize(nyears * nages);
    info->variable_map[this->proportion_catch_numbers_at_age.id_m] = &(fleet)->proportion_catch_numbers_at_age;


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
