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
  }

  /**
   * @brief Construct a new Fleet Interface Base object
   *
   * @param other
   */
  FleetInterfaceBase(const FleetInterfaceBase& other) :
  id(other.id) {}

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
  SharedInt interface_observed_agecomp_data_id_m = -999;
  /**
   * @brief The ID of the observed length-composition data object.
   */
  SharedInt interface_observed_lengthcomp_data_id_m = -999;
  /**
   * @brief The ID of the observed index data object.
   */
  SharedInt interface_observed_index_data_id_m = -999;
  /**
   * @brief The ID of the selectivity object.
   */
  SharedInt interface_selectivity_id_m = -999;

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
  SharedBoolean is_survey = false;
  /**
   * @brief The number of age bins in the fleet data.
   */
  SharedInt nages = 0;
  /**
   * @brief The number of length bins in the fleet data.
   */
  SharedInt nlengths = 0;
  /**
   * @brief The number of years in the fleet data.
   */
  SharedInt nyears = 0;
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
   * @brief The vector of natural log of the expected index of abundance for the fleet.
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
   * @brief The vector of conversions to go from age to length, i.e., the age-to-length-conversion matrix.
   */
  ParameterVector age_length_conversion_matrix;
  /**
   * @brief Should catchability (q) be estimated? The default is false.
   */
  SharedBoolean estimate_q = false;
  /**
   * @brief Is catchability (q) a random effect? The default is false.
   */
  SharedBoolean random_q = false;
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
  FleetInterface() : FleetInterfaceBase() {
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(std::make_shared<FleetInterface>(*this));
  }

  /**
   * @brief Construct a new Fleet Interface object
   *
   * @param other
   */
  FleetInterface(const FleetInterface& other) :
  FleetInterfaceBase(other),
  interface_observed_agecomp_data_id_m(other.interface_observed_agecomp_data_id_m),
  interface_observed_lengthcomp_data_id_m(other.interface_observed_lengthcomp_data_id_m), 
  interface_observed_index_data_id_m(other.interface_observed_index_data_id_m), 
  interface_selectivity_id_m(other.interface_selectivity_id_m), 
  name(other.name), is_survey(other.is_survey), 
  nages(other.nages), 
  nlengths(other.nlengths),
  nyears(other.nyears), 
  log_q(other.log_q), 
  log_Fmort(other.log_Fmort), 
  log_expected_index(other.log_expected_index),
  proportion_catch_numbers_at_age(other.proportion_catch_numbers_at_age), 
  proportion_catch_numbers_at_length(other.proportion_catch_numbers_at_length),
  age_length_conversion_matrix(other.age_length_conversion_matrix), 
  estimate_q(other.estimate_q), 
  random_q(other.random_q), 
  derived_cnaa(other.derived_cnaa), 
  derived_cnal(other.derived_cnal), 
  derived_cwaa(other.derived_cwaa), 
  derived_index(other.derived_index), 
  derived_age_composition(other.derived_age_composition), 
  derived_length_composition(other.derived_length_composition) {}

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
    interface_observed_agecomp_data_id_m.set(observed_agecomp_data_id);
  }

  /**
   * @brief Set the unique ID for the observed length-composition data object.
   * @param observed_lengthcomp_data_id Unique ID for the observed data object.
   */
  void SetObservedLengthCompData(int observed_lengthcomp_data_id) {
    interface_observed_lengthcomp_data_id_m.set(observed_lengthcomp_data_id);
  }

  /**
   * @brief Set the unique ID for the observed index data object.
   * @param observed_index_data_id Unique ID for the observed data object.
   */
  void SetObservedIndexData(int observed_index_data_id) {
    interface_observed_index_data_id_m.set(observed_index_data_id);
  }

  /**
   * @brief Set the unique ID for the selectivity object.
   * @param selectivity_id Unique ID for the observed object.
   */
  void SetSelectivity(int selectivity_id) {
    interface_selectivity_id_m.set(selectivity_id);
  }

  /**
  * @brief Get the unique ID for the observed age-composition data object.
  */
  int GetObservedAgeCompDataID() {
    return interface_observed_agecomp_data_id_m.get();
  }

  /**
  * @brief Get the unique ID for the observed length-composition data
  * object.
  */
  int GetObservedLengthCompDataID() {
    return interface_observed_lengthcomp_data_id_m.get();
  }

  /**
   * @brief Get the unique id for the observed index data object.
   */
  int GetObservedIndexDataID() {
    return interface_observed_index_data_id_m.get();
  }

  /** 
   * @brief Extracts the derived quantities from `Information` to the Rcpp
   * object. 
   */
  virtual void finalize() {
    if (this->finalized) {
      //log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) + " has been finalized already.");
    }

    this->finalized = true; //indicate this has been called already

    std::shared_ptr<fims_info::Information<double> > info =
            fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::fleet_iterator it;

    it = info->fleets.find(this->id);

    if (it == info->fleets.end()) {
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) + " not found in Information.");
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

      this->derived_cnaa = Rcpp::NumericVector(fleet->catch_numbers_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_cnaa.size(); i++) {
        this->derived_cnaa[i] = fleet->catch_numbers_at_age[i];
      }

      this->derived_cnal = Rcpp::NumericVector(fleet->catch_numbers_at_length.size());
      for (R_xlen_t i = 0; i < this->derived_cnal.size(); i++) {
        this->derived_cnal[i] = fleet->catch_numbers_at_length[i];
      }

      this->derived_cwaa = Rcpp::NumericVector(fleet->catch_weight_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_cwaa.size(); i++) {
        this->derived_cwaa[i] = fleet->catch_weight_at_age[i];
      }

      this->derived_age_composition = Rcpp::NumericVector(fleet->proportion_catch_numbers_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_age_composition.size(); i++) {
        this->derived_age_composition[i] = fleet->proportion_catch_numbers_at_age[i];
      }

      this->derived_length_composition = Rcpp::NumericVector(fleet->proportion_catch_numbers_at_length.size());
      for (R_xlen_t i = 0; i < this->derived_length_composition.size(); i++) {
        this->derived_length_composition[i] = fleet->proportion_catch_numbers_at_length[i];
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

    ss << "{\n";
    ss << " \"name\" : \"Fleet\",\n";

    ss << " \"type\" : \"fleet\",\n";
    ss << " \"tag\" : \"" << this->name << "\",\n";
    ss << " \"id\": " << this->id << ",\n";
    ss << " \"is_survey\": " << this->is_survey << ",\n";
    ss << " \"nlengths\": " << this->nlengths.get() << ",\n";
    ss << "\"parameters\": [\n";
    ss << "{\n";
    ss << " \"name\": \"log_Fmort\",\n";
    ss << " \"id\":" << this->log_Fmort.id_m << ",\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"values\": " << this->log_Fmort << "\n},\n";

    ss << " {\n";
    ss << " \"name\": \"log_q\",\n";
    ss << " \"id\":" << this->log_q.id_m << ",\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"values\": " << this->log_q << "\n}\n";
    if (this->nlengths > 0) {
      ss << " ,\n";
      ss << " {\n";
      ss << " \"name\": \"age_length_conversion_matrix\",\n";
      ss << " \"id\":" << this->age_length_conversion_matrix.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << this->age_length_conversion_matrix << "\n}\n";
    }
    ss << "], \"derived_quantities\":[\n";
    ss << "{\n";
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

    ss << " {\n";
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

    ss << " {\n";
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


    ss << "{\n";
    ss << "  \"name\": \"age_composition \",\n";
    ss << "  \"values\":[";
    if (this->derived_age_composition.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_age_composition.size() - 1; i++) {
        ss << this->derived_age_composition[i] << ", ";
      }
      ss << this->derived_age_composition[this->derived_age_composition.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"length_composition \",\n";
    ss << "  \"values\":[";
    if (this->derived_length_composition.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->derived_length_composition.size() - 1; i++) {
        ss << this->derived_length_composition[i] << ", ";
      }
      ss << this->derived_length_composition[this->derived_length_composition.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
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
    ss << " }\n]\n}";

    return ss.str();

  }




#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
      fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::Fleet<Type> > fleet =
      std::make_shared<fims_popdy::Fleet<Type> >();

    std::stringstream ss;

    // set relative info
    fleet->id = this->id;
    fleet->is_survey = this->is_survey.get();
    fleet->nages = this->nages.get();
    fleet->nlengths = this->nlengths.get();
    fleet->nyears = this->nyears.get();
    
    fleet->fleet_observed_agecomp_data_id_m =
      interface_observed_agecomp_data_id_m.get();
    
    fleet->fleet_observed_lengthcomp_data_id_m =
      interface_observed_lengthcomp_data_id_m.get();
    
    fleet->fleet_observed_index_data_id_m = interface_observed_index_data_id_m.get();
    
    fleet->fleet_selectivity_id_m = interface_selectivity_id_m.get();

    fleet->log_q.resize(this->log_q.size());
    for (size_t i = 0; i < this->log_q.size(); i++) {
      fleet->log_q[i] = this->log_q[i].initial_value_m;

      if (this->log_q[i].estimated_m) {
        ss.str("");
        ss << "Fleet.log_q." << this->id << "." << this->log_q[i].id_m;
          // register the parameter name
        info->RegisterParameterName(ss.str());
          if (this->log_q[i].is_random_effect_m) {
            info->RegisterRandomEffect(fleet->log_q[i]);
          } else {
            info->RegisterParameter(fleet->log_q[i]);
          }
      }
    }

    FIMS_INFO_LOG("adding Fleet fmort object to TMB");
    fleet->log_Fmort.resize(this->log_Fmort.size());
    for (size_t i = 0; i < log_Fmort.size(); i++) {
      fleet->log_Fmort[i] = this->log_Fmort[i].initial_value_m;

      if (this->log_Fmort[i].estimated_m) {
        ss.str("");
        ss << "Fleet.log_Fmort." << this->id << "." << this->log_Fmort[i].id_m;
        info->RegisterParameterName(ss.str());
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
    fleet->log_expected_index.resize(nyears.get()); // assume index is for all ages.
    info->variable_map[this->log_expected_index.id_m] = &(fleet)->log_expected_index;
    
    fleet->proportion_catch_numbers_at_age.resize(nyears.get() * nages.get());
    info->variable_map[this->proportion_catch_numbers_at_age.id_m] = &(fleet)->proportion_catch_numbers_at_age;
    FIMS_INFO_LOG(fims::to_string(this->nyears.get()) + " " + fims::to_string(this->nages.get()));
    FIMS_INFO_LOG(" adding Fleet length object to TMB");

    if (this->nlengths > 0) {
      fleet->proportion_catch_numbers_at_length.resize(this->nyears.get() * this->nlengths.get());
      fleet->age_length_conversion_matrix.resize(this->age_length_conversion_matrix.size());

      if (this->age_length_conversion_matrix.size() !=
              (this->nages.get()*this->nlengths.get())) {
          FIMS_ERROR_LOG("age_length_conversion_matrix don't match, " +
                  fims::to_string(this->age_length_conversion_matrix.size()) + " != " +
                  fims::to_string((this->nages.get()*this->nlengths.get())));
      }

      for (size_t i = 0; i < fleet->age_length_conversion_matrix.size(); i++) {
        fleet->age_length_conversion_matrix[i] =
          this->age_length_conversion_matrix[i].initial_value_m;
        FIMS_INFO_LOG(" adding Fleet length object to TMB in loop " + 
          fims::to_string(i) + " of " + 
          fims::to_string(fleet->age_length_conversion_matrix.size()));

        if (this->age_length_conversion_matrix[i].estimated_m) {
          ss.str("");
          ss << "Fleet.age_length_conversion_matrix." << this->id << "." << 
            this->age_length_conversion_matrix[i].id_m;
          info->RegisterParameterName(ss.str());
          if (this->age_length_conversion_matrix[i].is_random_effect_m) {
            info->RegisterRandomEffect(fleet->age_length_conversion_matrix[i]);
          } else {
            info->RegisterParameter(fleet->age_length_conversion_matrix[i]);
          }
        }
        FIMS_INFO_LOG(" adding Fleet length object to TMB in loop after if");
      }
      FIMS_INFO_LOG(" adding Fleet length object to TMB out loop");
      info->variable_map[this->age_length_conversion_matrix.id_m] = &(fleet)->age_length_conversion_matrix;
      info->variable_map[this->proportion_catch_numbers_at_length.id_m] = &(fleet)->proportion_catch_numbers_at_length;
  }

    // add to Information
    info->fleets[fleet->id] = fleet;
    FIMS_INFO_LOG("done adding Fleet object to TMB");
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
