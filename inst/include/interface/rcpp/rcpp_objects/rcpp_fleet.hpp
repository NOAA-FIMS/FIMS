/**
 * @file rcpp_fleet.hpp
 * @brief The Rcpp interface to declare fleets. Allows for the use of
 * methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "rcpp_interface_base.hpp"
#include "../../../population_dynamics/fleet/fleet.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp fleet
 * interfaces. This type should be inherited and not called from R directly.
 */
class FleetInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the FleetInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the FleetInterfaceBase object.
   */
  uint32_t id;

  /**
   * @brief The constructor.
   */
  FleetInterfaceBase() { this->id = FleetInterfaceBase::id_g++; }

  /**
   * @brief Interface objects are not copyable.
   */
  FleetInterfaceBase(const FleetInterfaceBase &) = delete;
  FleetInterfaceBase &operator=(const FleetInterfaceBase &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~FleetInterfaceBase() {}

};

/**
 * @brief The Rcpp interface for Fleet to instantiate from R:
 * fleet <- methods::new(Fleet)
 */
class FleetInterface : public FleetInterfaceBase {
  /**
   * @brief The ID of the observed age-composition data object.
   */
  int interface_observed_agecomp_data_id_m = -999;
  /**
   * @brief The ID of the observed length-composition data object.
   */
  int interface_observed_lengthcomp_data_id_m = -999;
  /**
   * @brief The ID of the observed index data object.
   */
  int interface_observed_index_data_id_m = -999;
  /**
   * @brief The ID of the observed catch data object.
   */
  int interface_observed_catch_data_id_m = -999;
  /**
   * @brief The ID of the selectivity object.
   */
  int interface_selectivity_id_m = -999;

 public:
  /**
   * @brief The name of the fleet.
   */
  std::string name = "NA";
  /**
   * @brief The number of age bins in the fleet data.
   */
  int n_ages = 0;
  /**
   * @brief The number of length bins in the fleet data.
   */
  int n_lengths = 0;
  /**
   * @brief The number of years in the fleet data.
   */
  int n_years = 0;
  /**
   * @brief What units are the observed catch for this fleet measured in.
   * Options are weight or numbers, default is weight.
   */
  std::string observed_catch_units = "weight";
  /**
   * @brief What units is the observed index of abundance for this fleet
   * measured in. Options are weight or numbers, default is weight.
   */
  std::string observed_index_units = "weight";
  /**
   * @brief The natural log of the index of abundance scaling parameter
   * for this fleet.
   */
  VariableVector log_q;
  /**
   * @brief The vector of the natural log of fishing mortality rates for this
   * fleet.
   */
  VariableVector log_Fmort;

  /**
   * @brief The vector of conversions to go from age to length, i.e., the
   * age-to-length-conversion matrix.
   */
  VariableVector age_to_length_conversion;

  // Fleet based derived quantities
  /**
   * @brief Annual catch at age in numbers for a specific fleet.
   */
  VariableVector catch_numbers_at_age;
  /**
   * @brief Annual catch at age in weight for a specific fleet.
   */
  VariableVector catch_weight_at_age;
  /**
   * @brief Annual catch at length in numbers for a specific fleet.
   */
  VariableVector catch_numbers_at_length;
  /**
   * @brief Total catch in weight for a specific fleet.
   */
  VariableVector catch_weight;
  /**
   * @brief Total catch in numbers for a specific fleet.
   */
  VariableVector catch_numbers;
  /**
   * @brief Total expected catch for a specific fleet, where the units of
   * the measurement depend on the specified units for that fleet.
   */
  VariableVector catch_expected;
  /**
   * @brief Log total expected catch for a specific fleet, where the units
   * of the measurement depend on the specified units for that fleet.
   */
  VariableVector log_catch_expected;
  /**
   * @brief Proportion of total catch by age used to specify the age
   * composition.
   */
  VariableVector agecomp_proportion;
  /**
   * @brief Proportion of total catch by length used to specify the length
   * composition.
   */
  VariableVector lengthcomp_proportion;
  /**
   * @brief Numbers at age for a fleet index such as survey or CPUE.
   */
  VariableVector index_numbers_at_age;
  /**
   * @brief Weight at age for a fleet index such as survey or CPUE.
   */
  VariableVector index_weight_at_age;
  /**
   * @brief Numbers at length for a fleet index such as survey or CPUE.
   */
  VariableVector index_numbers_at_length;
  /**
   * @brief Total weight for a fleet index such as survey or CPUE.
   */
  VariableVector index_weight;
  /**
   * @brief Total numbers for a fleet index such as survey or CPUE.
   */
  VariableVector index_numbers;
  /**
   * @brief Expected value for a fleet index, where the units of the
   * measurement depend on the specified units for that fleet.
   */
  VariableVector index_expected;
  /**
   * @brief Log expected value for a fleet index, where the units of the
   * measurement depend on the specified units for that fleet.
   */
  VariableVector log_index_expected;
  /**
   * @brief Expected age-composition proportions for the fleet.
   */
  VariableVector agecomp_expected;
  /**
   * @brief Expected length-composition proportions for the fleet.
   */
  VariableVector lengthcomp_expected;

  /**
   * @brief The constructor.
   */
  FleetInterface() : FleetInterfaceBase() {}

  /**
   * @brief Interface objects are not copyable.
   */
  FleetInterface(const FleetInterface &) = delete;
  FleetInterface &operator=(const FleetInterface &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~FleetInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @copydoc FIMSRcppInterfaceBase::get_variable_vector
   */
  virtual VariableVector *get_variable_vector(const std::string &name) {
    if (name == "log_q") return &this->log_q;
    if (name == "log_Fmort") return &this->log_Fmort;
    if (name == "age_to_length_conversion")
      return &this->age_to_length_conversion;
    if (name == "catch_numbers_at_age") return &this->catch_numbers_at_age;
    if (name == "catch_weight_at_age") return &this->catch_weight_at_age;
    if (name == "catch_numbers_at_length")
      return &this->catch_numbers_at_length;
    if (name == "catch_weight") return &this->catch_weight;
    if (name == "catch_numbers") return &this->catch_numbers;
    if (name == "catch_expected") return &this->catch_expected;
    if (name == "log_catch_expected") return &this->log_catch_expected;
    if (name == "agecomp_proportion") return &this->agecomp_proportion;
    if (name == "lengthcomp_proportion") return &this->lengthcomp_proportion;
    if (name == "index_numbers_at_age") return &this->index_numbers_at_age;
    if (name == "index_weight_at_age") return &this->index_weight_at_age;
    if (name == "index_numbers_at_length")
      return &this->index_numbers_at_length;
    if (name == "index_weight") return &this->index_weight;
    if (name == "index_numbers") return &this->index_numbers;
    if (name == "index_expected") return &this->index_expected;
    if (name == "log_index_expected") return &this->log_index_expected;
    if (name == "agecomp_expected") return &this->agecomp_expected;
    if (name == "lengthcomp_expected") return &this->lengthcomp_expected;
    return nullptr;
  }

  /**
   * @brief Sets the name of the fleet.
   * @param name The name to set.
   */
  void SetName(const std::string &name) { this->name = name; }

  /**
   * @brief Gets the name of the fleet.
   * @return The name.
   */
  std::string GetName() const { return this->name; }

  /**
   * @brief Set the unique ID for the observed age-composition data object.
   * @param observed_agecomp_data_id Unique ID for the observed data object.
   */
  void SetObservedAgeCompDataID(int observed_agecomp_data_id) {
    interface_observed_agecomp_data_id_m = observed_agecomp_data_id;
  }

  /**
   * @brief Set the unique ID for the observed length-composition data object.
   * @param observed_lengthcomp_data_id Unique ID for the observed data object.
   */
  void SetObservedLengthCompDataID(int observed_lengthcomp_data_id) {
    interface_observed_lengthcomp_data_id_m = observed_lengthcomp_data_id;
  }

  /**
   * @brief Set the unique ID for the observed index data object.
   * @param observed_index_data_id Unique ID for the observed data object.
   */
  void SetObservedIndexDataID(int observed_index_data_id) {
    interface_observed_index_data_id_m = observed_index_data_id;
  }

  /**
   * @brief Set the unique ID for the observed catch data object.
   * @param observed_catch_data_id Unique ID for the observed data object.
   */
  void SetObservedCatchDataID(int observed_catch_data_id) {
    interface_observed_catch_data_id_m = observed_catch_data_id;
  }
  /**
   * @brief Set the unique ID for the selectivity object.
   * @param selectivity_id Unique ID for the observed object.
   */
  void SetSelectivityID(int selectivity_id) {
    interface_selectivity_id_m = selectivity_id;
  }

  /**
   * @brief Get the unique ID for the selectivity object.
   *
   * @return uint32_t
   */
  int GetSelectivityID() { return interface_selectivity_id_m; }

  /**
   * @brief Get the unique ID for the observed age-composition data object.
   */
  int GetObservedAgeCompDataID() {
    return interface_observed_agecomp_data_id_m;
  }

  /**
   * @brief Get the unique ID for the observed length-composition data
   * object.
   */
  int GetObservedLengthCompDataID() {
    return interface_observed_lengthcomp_data_id_m;
  }

  /**
   * @brief Get the unique id for the observed index data object.
   */
  int GetObservedIndexDataID() {
    return interface_observed_index_data_id_m;
  }

  /**
   * @brief Get the unique id for the observed catch data object.
   */
  int GetObservedCatchDataID() {
    return interface_observed_catch_data_id_m;
  }
  /**
   * @brief Extracts the derived quantities from `Information` to the Rcpp
   * object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::fleet_iterator it;

    it = info->fleets.find(this->id);

    if (it == info->fleets.end()) {
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::Fleet<double>> fleet =
          std::dynamic_pointer_cast<fims_popdy::Fleet<double>>(it->second);

      for (size_t i = 0; i < this->log_Fmort.size(); i++) {
        set_final_value_by_estimation_status(this->log_Fmort[i],
                                             fleet->log_Fmort[i]);
      }

      for (size_t i = 0; i < this->log_q.size(); i++) {
        set_final_value_by_estimation_status(this->log_q[i], fleet->log_q[i]);
      }

      for (size_t i = 0; i < fleet->age_to_length_conversion.size(); i++) {
        set_final_value_by_estimation_status(this->age_to_length_conversion[i],
                                             fleet->age_to_length_conversion[i]);
      }
    }
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::Fleet<Type>> fleet =
        std::make_shared<fims_popdy::Fleet<Type>>();

    std::stringstream ss;

    // set relative info
    fleet->id = this->id;
    fleet->n_ages = this->n_ages;
    fleet->n_lengths = this->n_lengths;
    fleet->n_years = this->n_years;
    fleet->observed_catch_units = this->observed_catch_units;
    fleet->observed_index_units = this->observed_index_units;

    fleet->fleet_observed_agecomp_data_id_m =
        interface_observed_agecomp_data_id_m;

    fleet->fleet_observed_lengthcomp_data_id_m =
        interface_observed_lengthcomp_data_id_m;

    fleet->fleet_observed_index_data_id_m =
        interface_observed_index_data_id_m;
    fleet->fleet_observed_catch_data_id_m =
        interface_observed_catch_data_id_m;

    fleet->fleet_selectivity_id_m = interface_selectivity_id_m;

    fleet->log_q.resize(this->log_q.size());
    for (size_t i = 0; i < this->log_q.size(); i++) {
      fleet->log_q[i] = this->log_q[i].initial_value_m;
      ss.str("");
      ss << "Fleet." << this->id << ".log_q." << this->log_q[i].id_m;
      register_parameter_if_estimable(
          fleet->log_q[i], this->log_q[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->log_q.id_m] = &(fleet)->log_q;

    if (this->log_Fmort.size() != static_cast<size_t>(this->n_years)) {
      FIMS_ERROR_LOG("The size of `log_Fmort` does not match `n_years`: " +
                     fims::to_string(this->log_Fmort.size()) +
                     " != " + fims::to_string(this->n_years));
      throw std::invalid_argument(
          "Fleet log_Fmort size mismatch."
          "Fleet log_Fmort is of size " +
          fims::to_string(this->log_Fmort.size()) +
          " and the number of years is " +
          fims::to_string(this->n_years));
    }
    fleet->log_Fmort.resize(static_cast<size_t>(this->log_Fmort.size()));
    for (size_t i = 0; i < log_Fmort.size(); i++) {
      fleet->log_Fmort[i] = this->log_Fmort[i].initial_value_m;
      ss.str("");
      ss << "Fleet." << this->id << ".log_Fmort." << this->log_Fmort[i].id_m;
      register_parameter_if_estimable(
          fleet->log_Fmort[i], this->log_Fmort[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->log_Fmort.id_m] = &(fleet)->log_Fmort;

    if (this->n_lengths > 0) {
      fleet->age_to_length_conversion.resize(
          this->age_to_length_conversion.size());

      if (this->age_to_length_conversion.size() !=
          static_cast<size_t>(this->n_ages * this->n_lengths)) {
        FIMS_ERROR_LOG(
            "age_to_length_conversion don't match, " +
            fims::to_string(this->age_to_length_conversion.size()) + " != " +
            fims::to_string((this->n_ages * this->n_lengths)));
      }

      for (size_t i = 0; i < fleet->age_to_length_conversion.size(); i++) {
        fleet->age_to_length_conversion[i] =
            this->age_to_length_conversion[i].initial_value_m;

        ss.str("");
        ss << "Fleet." << this->id << ".age_to_length_conversion."
           << this->age_to_length_conversion[i].id_m;
        register_parameter_if_estimable(
            fleet->age_to_length_conversion[i],
            this->age_to_length_conversion[i].estimation_status_m,
            ss.str(), false);
      }
      info->variable_map[this->age_to_length_conversion.id_m] =
          &(fleet)->age_to_length_conversion;
    }

    // add to Information
    info->fleets[fleet->id] = fleet;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

#endif
