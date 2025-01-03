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

#include "../../../common/def.hpp"
#include "../../../population_dynamics/fleet/fleet.hpp"
#include "rcpp_interface_base.hpp"

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
   * @brief The map associating the IDs of FleetInterfaceBase to the objects.
   * This is a live object, which is an object that has been created and lives
   * in memory.
   */
  static std::map<uint32_t, FleetInterfaceBase*> live_objects;

  /**
   * @brief The constructor.
   */
  FleetInterfaceBase() {
    this->id = FleetInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    FleetInterfaceBase */
    FleetInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  /**
   * @brief The destructor.
   */
  virtual ~FleetInterfaceBase() {}

  /**
   * @brief Get the ID for the child fleet interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;
};
// static id of the FleetInterfaceBase object
uint32_t FleetInterfaceBase::id_g = 1;
// local id of the FleetInterfaceBase object map relating the ID of the
// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, FleetInterfaceBase*> FleetInterfaceBase::live_objects;

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
   * @brief The ID of the selectivity object.
   */
  int interface_selectivity_id_m = -999;

 public:
  /**
   * @brief The name of the fleet.
   */
  std::string name = "NA";
  /**
   * @brief Is this fleet a survey, then true. If the fleet is a fishery, then
   * false, where false is the default. As of version 0.3.0, a fleet in FIMS
   * cannot accommodate both landings and index data, and thus must be
   * designated to be a fleet or a survey. This will be fixed in later
   * versions.
   */
  bool is_survey = false;
  /**
   * @brief The number of age bins in the fleet data.
   */
  int nages;
  /**
   * @brief The number of length bins in the fleet data.
   */
  int nlengths = 0;
  /**
   * @brief The number of years in the fleet data.
   */
  int nyears;
  /**
   * @brief The natural log of the catchability parameter for this fleet.
   */
  ParameterVector log_q;
  /**
   * @brief The vector of the natural log of fishing mortality rates for this
   * fleet.
   */
  ParameterVector log_Fmort;
  /**
   * @brief The vector of natural log of the expected index of abundance for the
   * fleet.
   */
  ParameterVector log_expected_index;
  /**
   * @brief The vector of expected catch-at-age in numbers for the fleet.
   */
  ParameterVector proportion_catch_numbers_at_age;
  /**
   * @brief The vector of expected catch-at-length in numbers for the fleet.
   */
  ParameterVector proportion_catch_numbers_at_length;
  /**
   * @brief The vector of conversions to go from age to length, i.e., the
   * age-to-length-conversion matrix.
   */
  ParameterVector age_length_conversion_matrix;
  /**
   * @brief Should catchability (q) be estimated? The default is false.
   */
  bool estimate_q = false;
  /**
   * @brief Is catchability (q) a random effect? The default is false.
   */
  bool random_q = false;
  // derived quantities
  /**
   * @brief Derived catch-at-age in numbers.
   */
  Rcpp::NumericVector derived_cnaa;
  /**
   * @brief Derived catch-at-length in numbers.
   */
  Rcpp::NumericVector derived_cnal;
  /**
   * @brief Derived catch-at-age in weight (mt).
   */
  Rcpp::NumericVector derived_cwaa;
  /**
   * @brief Derived index.
   */
  Rcpp::NumericVector derived_index;
  /**
   * @brief Derived age compositions.
   */
  Rcpp::NumericVector derived_age_composition;
  /**
   * @brief Derived length compositions.
   */
  Rcpp::NumericVector derived_length_composition;

  /**
   * @brief The constructor.
   */
  FleetInterface() : FleetInterfaceBase() {}

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
   * @brief Set the unique ID for the observed age-composition data object.
   * @param observed_agecomp_data_id Unique ID for the observed data object.
   */
  void SetObservedAgeCompData(int observed_agecomp_data_id) {
    interface_observed_agecomp_data_id_m = observed_agecomp_data_id;
  }

  /**
   * @brief Set the unique ID for the observed length-composition data object.
   * @param observed_lengthcomp_data_id Unique ID for the observed data object.
   */
  void SetObservedLengthCompData(int observed_lengthcomp_data_id) {
    interface_observed_lengthcomp_data_id_m = observed_lengthcomp_data_id;
  }

  /**
   * @brief Set the unique ID for the observed index data object.
   * @param observed_index_data_id Unique ID for the observed data object.
   */
  void SetObservedIndexData(int observed_index_data_id) {
    interface_observed_index_data_id_m = observed_index_data_id;
  }

  /**
   * @brief Set the unique ID for the selectivity object.
   * @param selectivity_id Unique ID for the observed object.
   */
  void SetSelectivity(int selectivity_id) {
    interface_selectivity_id_m = selectivity_id;
  }

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
  int GetObservedIndexDataID() { return interface_observed_index_data_id_m; }

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

    std::shared_ptr<fims_info::Information<double> > info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::fleet_iterator it;

    it = info->fleets.find(this->id);

    if (it == info->fleets.end()) {
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::Fleet<double> > fleet =
          std::dynamic_pointer_cast<fims_popdy::Fleet<double> >(it->second);

      for (size_t i = 0; i < this->log_Fmort.size(); i++) {
        if (this->log_Fmort[i].estimated_m) {
          this->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
        } else {
          this->log_Fmort[i].final_value_m = this->log_Fmort[i].initial_value_m;
        }
      }

      for (size_t i = 0; i < this->log_q.size(); i++) {
        if (this->log_q[i].estimated_m) {
          this->log_q[i].final_value_m = fleet->log_q[i];
        } else {
          this->log_q[i].final_value_m = this->log_q[i].initial_value_m;
        }
      }

      this->derived_cnaa =
          Rcpp::NumericVector(fleet->catch_numbers_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_cnaa.size(); i++) {
        this->derived_cnaa[i] = fleet->catch_numbers_at_age[i];
      }

      this->derived_cnal =
          Rcpp::NumericVector(fleet->catch_numbers_at_length.size());
      for (R_xlen_t i = 0; i < this->derived_cnal.size(); i++) {
        this->derived_cnal[i] = fleet->catch_numbers_at_length[i];
      }

      this->derived_cwaa =
          Rcpp::NumericVector(fleet->catch_weight_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_cwaa.size(); i++) {
        this->derived_cwaa[i] = fleet->catch_weight_at_age[i];
      }

      this->derived_age_composition =
          Rcpp::NumericVector(fleet->proportion_catch_numbers_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_age_composition.size(); i++) {
        this->derived_age_composition[i] =
            fleet->proportion_catch_numbers_at_age[i];
      }

      this->derived_length_composition =
          Rcpp::NumericVector(fleet->proportion_catch_numbers_at_length.size());
      for (R_xlen_t i = 0; i < this->derived_length_composition.size(); i++) {
        this->derived_length_composition[i] =
            fleet->proportion_catch_numbers_at_length[i];
      }

      this->derived_index = Rcpp::NumericVector(fleet->expected_index.size());
      for (R_xlen_t i = 0; i < this->derived_index.size(); i++) {
        this->derived_index[i] = fleet->expected_index[i];
      }
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * fleet interface. It returns the name and ID as well as all derived
   * quantities and parameter estimates. This string is formatted for a json
   * file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "\"module\" : {\n";
    ss << " \"name\" : \"Fleet\",\n";

    ss << " \"type\" : \"fleet\",\n";
    ss << " \"tag\" : \"" << this->name << "\",\n";
    ss << " \"id\": " << this->id << ",\n";

    ss << " \"parameter\": {\n";
    ss << " \"name\": \"log_Fmort\",\n";
    ss << " \"id\":" << this->log_Fmort.id_m << ",\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"values\": " << this->log_Fmort << "\n},\n";

    ss << " \"parameter\": {\n";
    ss << " \"name\": \"log_q\",\n";
    ss << " \"id\":" << this->log_q.id_m << ",\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"values\": " << this->log_q << "\n},\n";

    ss << " \"derived_quantity\": {\n";
    ss << "  \"name\": \"cnaa\",\n";
    ss << "  \"values\":[";
    if (this->derived_cnaa.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_cnaa.size() - 1; i++) {
        ss << this->derived_cnaa[i] << ", ";
      }
      ss << this->derived_cnaa[this->derived_cnaa.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << " \"derived_quantity\": {\n";
    ss << "  \"name\": \"cnal\",\n";
    ss << "  \"values\":[";
    if (this->derived_cnal.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_cnal.size() - 1; i++) {
        ss << this->derived_cnal[i] << ", ";
      }
      ss << this->derived_cnal[this->derived_cnal.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << " \"derived_quantity\": {\n";
    ss << "  \"name\": \"cwaa\",\n";
    ss << "  \"values\":[";
    if (this->derived_cwaa.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_cwaa.size() - 1; i++) {
        ss << this->derived_cwaa[i] << ", ";
      }
      ss << this->derived_cwaa[this->derived_cwaa.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << " \"derived_quantity\": {\n";
    ss << "  \"name\": \"age_composition \",\n";
    ss << "  \"values\":[";
    if (this->derived_age_composition.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_age_composition.size() - 1; i++) {
        ss << this->derived_age_composition[i] << ", ";
      }
      ss << this->derived_age_composition[this->derived_age_composition.size() -
                                          1]
         << "]\n";
    }
    ss << " },\n";

    ss << " \"derived_quantity\": {\n";
    ss << "  \"name\": \"length_composition \",\n";
    ss << "  \"values\":[";
    if (this->derived_length_composition.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_length_composition.size() - 1;
           i++) {
        ss << this->derived_length_composition[i] << ", ";
      }
      ss << this->derived_length_composition
                [this->derived_length_composition.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " \"derived_quantity\": {\n";
    ss << "  \"name\": \"index \",\n";
    ss << "  \"values\":[";
    if (this->derived_index.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_index.size() - 1; i++) {
        ss << this->derived_index[i] << ", ";
      }
      ss << this->derived_index[this->derived_index.size() - 1] << "]\n";
    }
    ss << " },\n";

    return ss.str();
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
    fleet->nlengths = this->nlengths;
    fleet->nyears = this->nyears;
    fleet->fleet_observed_agecomp_data_id_m =
        interface_observed_agecomp_data_id_m;
    fleet->fleet_observed_lengthcomp_data_id_m =
        interface_observed_lengthcomp_data_id_m;
    fleet->fleet_observed_index_data_id_m = interface_observed_index_data_id_m;
    fleet->fleet_selectivity_id_m = interface_selectivity_id_m;

    fleet->log_q.resize(this->log_q.size());
    for (size_t i = 0; i < this->log_q.size(); i++) {
      fleet->log_q[i] = this->log_q[i].initial_value_m;

      if (this->log_q[i].estimated_m) {
        info->RegisterParameterName("log_q");
        if (this->log_q[i].is_random_effect_m) {
          info->RegisterRandomEffect(fleet->log_q[i]);
        } else {
          info->RegisterParameter(fleet->log_q[i]);
        }
      }
    }

    fleet->log_Fmort.resize(this->log_Fmort.size());
    for (size_t i = 0; i < log_Fmort.size(); i++) {
      fleet->log_Fmort[i] = this->log_Fmort[i].initial_value_m;

      if (this->log_Fmort[i].estimated_m) {
        info->RegisterParameterName("log_Fmort");
        if (this->log_Fmort[i].is_random_effect_m) {
          info->RegisterRandomEffect(fleet->log_Fmort[i]);
        } else {
          info->RegisterParameter(fleet->log_Fmort[i]);
        }
      }
    }
    // add to variable_map
    info->variable_map[this->log_Fmort.id_m] = &(fleet)->log_Fmort;

    // exp_catch
    fleet->log_expected_index.resize(nyears);  // assume index is for all ages.
    info->variable_map[this->log_expected_index.id_m] =
        &(fleet)->log_expected_index;
    fleet->proportion_catch_numbers_at_age.resize(nyears * nages);
    info->variable_map[this->proportion_catch_numbers_at_age.id_m] =
        &(fleet)->proportion_catch_numbers_at_age;

    if (this->nlengths > 0) {
      fleet->proportion_catch_numbers_at_length.resize(nyears * nlengths);
      fleet->age_length_conversion_matrix.resize(nages * nlengths);
      for (size_t i = 0; i < fleet->age_length_conversion_matrix.size(); i++) {
        fleet->age_length_conversion_matrix[i] =
            this->age_length_conversion_matrix[i].initial_value_m;

        if (this->age_length_conversion_matrix[i].estimated_m) {
          info->RegisterParameterName("age_length_conversion_matrix");
          if (this->age_length_conversion_matrix[i].is_random_effect_m) {
            info->RegisterRandomEffect(fleet->age_length_conversion_matrix[i]);
          } else {
            info->RegisterParameter(fleet->age_length_conversion_matrix[i]);
          }
        }
      }
      info->variable_map[this->age_length_conversion_matrix.id_m] =
          &(fleet)->age_length_conversion_matrix;
      info->variable_map[this->proportion_catch_numbers_at_length.id_m] =
          &(fleet)->proportion_catch_numbers_at_length;
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
    FIMS_INFO_LOG("adding Fleet object to TMB");
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif
