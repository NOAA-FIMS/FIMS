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
  static std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  FleetInterfaceBase() {
    this->id = FleetInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    FleetInterfaceBase */
    // FleetInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Fleet Interface Base object
   *
   * @param other
   */
  FleetInterfaceBase(const FleetInterfaceBase &other) : id(other.id) {}

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
std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>>
    FleetInterfaceBase::live_objects;
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
   * @brief The ID of the observed landings data object.
   */
  SharedInt interface_observed_landings_data_id_m = -999;
  /**
   * @brief The ID of the selectivity object.
   */
  SharedInt interface_selectivity_id_m = -999;
  /**
   * @brief What units is selectivity for this fleet modeled in.
   * Options are age or length, default is age.
   */
  SharedString  selectivity_units = fims::to_string("age");

 public:
  /**
   * @brief The name of the fleet.
   */
  SharedString name = fims::to_string("NA");
  /**
   * @brief The number of age bins in the fleet data.
   */
  SharedInt n_ages = 0;
  /**
   * @brief The number of length bins in the fleet data.
   */
  SharedInt n_lengths = 0;
  /**
   * @brief The number of years in the fleet data.
   */
  SharedInt n_years = 0;
  /**
   * @brief What units are the observed landings for this fleet measured in.
   * Options are weight or numbers, default is weight.
   */
  SharedString observed_landings_units = fims::to_string("weight");
  /**
   * @brief What units is the observed index of abundance for this fleet
   * measured in. Options are weight or numbers, default is weight.
   */
  SharedString observed_index_units = fims::to_string("weight");
  /**
   * @brief The natural log of the index of abundance scaling parameter
   * for this fleet.
   */
  ParameterVector log_q;
  /**
   * @brief The vector of the natural log of fishing mortality rates for this
   * fleet.
   */
  ParameterVector log_Fmort;
  /**
   * @brief The vector of natural log of the expected total landings for
   * the fleet.
   */
  ParameterVector log_landings_expected;
  /**
   * @brief The vector of natural log of the expected index of abundance
   * for the fleet.
   */
  ParameterVector log_index_expected;
  /**
   * @brief The vector of expected landings-at-age in numbers for the fleet.
   */
  ParameterVector agecomp_expected;
  /**
   * @brief The vector of expected landings-at-length in numbers for the fleet.
   */
  ParameterVector lengthcomp_expected;
  /**
   * @brief The vector of expected landings-at-age in numbers for the fleet.
   */
  ParameterVector agecomp_proportion;
  /**
   * @brief The vector of expected landings-at-length in numbers for the fleet.
   */
  ParameterVector lengthcomp_proportion;
  /**
   * @brief The vector of conversions to go from age to length, i.e., the
   * age-to-length-conversion matrix.
   */
  ParameterVector age_to_length_conversion;

  // derived quantities
  /**
   * @brief Derived landings-at-age in numbers.
   */
  Rcpp::NumericVector derived_landings_naa;
  /**
   * @brief Derived landings-at-length in numbers.
   */
  Rcpp::NumericVector derived_landings_nal;
  /**
   * @brief Derived landings-at-age in weight (mt).
   */
  Rcpp::NumericVector derived_landings_waa;
  /**
   * @brief Derived landings in observed units.
   */
  Rcpp::NumericVector derived_landings_expected;
  /**
   * @brief Derived landings in weight.
   */
  Rcpp::NumericVector derived_landings_w;
  /**
   * @brief Derived landings in numbers.
   */
  Rcpp::NumericVector derived_landings_n;
  /**
   * @brief Derived landings-at-age in numbers.
   */
  Rcpp::NumericVector derived_index_naa;
  /**
   * @brief Derived landings-at-length in numbers.
   */
  Rcpp::NumericVector derived_index_nal;
  /**
   * @brief Derived landings-at-age in weight (mt).
   */
  Rcpp::NumericVector derived_index_waa;
  /**
   * @brief Derived index in observed units.
   */
  Rcpp::NumericVector derived_index_expected;
  /**
   * @brief Derived index in weight.
   */
  Rcpp::NumericVector derived_index_w;
  /**
   * @brief Derived index in numbers.
   */
  Rcpp::NumericVector derived_index_n;
  /**
   * @brief Derived age composition proportions.
   */
  Rcpp::NumericVector derived_agecomp_proportion;
  /**
   * @brief Derived length composition proportions.
   */
  Rcpp::NumericVector derived_lengthcomp_proportion;
  /**
   * @brief Derived age compositions.
   */
  Rcpp::NumericVector derived_agecomp_expected;
  /**
   * @brief Derived length compositions.
   */
  Rcpp::NumericVector derived_lengthcomp_expected;

  /**
   * @brief The constructor.
   */
  FleetInterface() : FleetInterfaceBase() {
    std::shared_ptr<FleetInterface> fleet =
        std::make_shared<FleetInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(fleet);
    /* Create instance of map: key is id and value is pointer to
     FleetInterfaceBase */
    FleetInterfaceBase::live_objects[this->id] = fleet;
  }

  /**
   * @brief Construct a new Fleet Interface object
   *
   * @param other
   */
  FleetInterface(const FleetInterface &other)
      : FleetInterfaceBase(other),
        interface_observed_agecomp_data_id_m(
            other.interface_observed_agecomp_data_id_m),
        interface_observed_lengthcomp_data_id_m(
            other.interface_observed_lengthcomp_data_id_m),
        interface_observed_index_data_id_m(
            other.interface_observed_index_data_id_m),
        interface_observed_landings_data_id_m(
            other.interface_observed_landings_data_id_m),
        interface_selectivity_id_m(other.interface_selectivity_id_m),
        name(other.name),
        n_ages(other.n_ages),
        n_lengths(other.n_lengths),
        n_years(other.n_years),
        log_q(other.log_q),
        log_Fmort(other.log_Fmort),
        log_index_expected(other.log_index_expected),
        log_landings_expected(other.log_landings_expected),
        agecomp_proportion(other.agecomp_proportion),
        lengthcomp_proportion(other.lengthcomp_proportion),
        agecomp_expected(other.agecomp_expected),
        lengthcomp_expected(other.lengthcomp_expected),
        age_to_length_conversion(other.age_to_length_conversion),
        observed_landings_units(other.observed_landings_units),
        observed_index_units(other.observed_index_units),
        derived_landings_naa(other.derived_landings_naa),
        derived_landings_nal(other.derived_landings_nal),
        derived_landings_waa(other.derived_landings_waa),
        derived_index_expected(other.derived_index_expected),
        derived_index_w(other.derived_index_w),
        derived_index_n(other.derived_index_n),
        derived_landings_expected(other.derived_landings_expected),
        derived_landings_w(other.derived_landings_w),
        derived_landings_n(other.derived_landings_n),
        derived_agecomp_proportion(other.derived_agecomp_proportion),
        derived_lengthcomp_proportion(other.derived_lengthcomp_proportion),
        derived_agecomp_expected(other.derived_agecomp_expected),
        derived_lengthcomp_expected(other.derived_lengthcomp_expected) {}

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
   * @brief Sets the name of the fleet.
   * @param name The name to set.
   */
  void SetName(const std::string &name) { this->name.set(name); }

  /**
   * @brief Gets the name of the fleet.
   * @return The name.
   */
  std::string GetName() const { return this->name.get(); }

  /**
   * @brief Set the unique ID for the observed age-composition data object.
   * @param observed_agecomp_data_id Unique ID for the observed data object.
   */
  void SetObservedAgeCompDataID(int observed_agecomp_data_id) {
    interface_observed_agecomp_data_id_m.set(observed_agecomp_data_id);
  }

  /**
   * @brief Set the unique ID for the observed length-composition data object.
   * @param observed_lengthcomp_data_id Unique ID for the observed data object.
   */
  void SetObservedLengthCompDataID(int observed_lengthcomp_data_id) {
    interface_observed_lengthcomp_data_id_m.set(observed_lengthcomp_data_id);
  }

  /**
   * @brief Set the unique ID for the observed index data object.
   * @param observed_index_data_id Unique ID for the observed data object.
   */
  void SetObservedIndexDataID(int observed_index_data_id) {
    interface_observed_index_data_id_m.set(observed_index_data_id);
  }

  /**
   * @brief Set the unique ID for the observed landings data object.
   * @param observed_landings_data_id Unique ID for the observed data object.
   */
  void SetObservedLandingsDataID(int observed_landings_data_id) {
    interface_observed_landings_data_id_m.set(observed_landings_data_id);
  }
  /**
   * @brief Set the unique ID for selectivity object and set units to age.
   * @param selectivity_id Unique ID for the observed object.
   */
  void SetSelectivityAgeID(int selectivity_id) {
    interface_selectivity_id_m.set(selectivity_id);
    selectivity_units.set(fims::to_string("age"));
    // TODO: We should as a warning/notification that this is setting selectivity
    // units to age.
  }

  /**
   * @brief Set the unique ID for the selectivity object and set units to length.
   * @param selectivity_id Unique ID for the observed object.
   */
  void SetSelectivityLengthID(int selectivity_id) {
    interface_selectivity_id_m.set(selectivity_id);
    selectivity_units.set(fims::to_string("length"));
    // TODO: We should as a warning/notification that this is setting selectivity
    // units to length.
  }

  /**
   * @brief Get the unique ID for the selectivity object.
   */
  int GetSelectivityID() {
    return interface_selectivity_id_m.get();
  }

  /**
   * @brief Get the units for the selectivity object.
   */
  std::string GetSelectivityUnits() {
    return selectivity_units.get();
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
   * @brief Get the unique id for the observed landings data object.
   */
  int GetObservedLandingsDataID() {
    return interface_observed_landings_data_id_m.get();
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
        if (this->log_Fmort[i].estimation_type_m.get() == "constant") {
          this->log_Fmort[i].final_value_m = this->log_Fmort[i].initial_value_m;
        } else {
          this->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
        }
      }

      for (size_t i = 0; i < this->log_q.size(); i++) {
        if (this->log_q[i].estimation_type_m.get() == "constant") {
          this->log_q[i].final_value_m = this->log_q[i].initial_value_m;
        } else {
          this->log_q[i].final_value_m = fleet->log_q[i];
        }
      }

      for (size_t i = 0; i < fleet->age_to_length_conversion.size(); i++) {
        if (this->age_to_length_conversion[i].estimation_type_m.get() ==
            "constant") {
          this->age_to_length_conversion[i].final_value_m =
              this->age_to_length_conversion[i].initial_value_m;
        } else {
          this->age_to_length_conversion[i].final_value_m =
              fleet->age_to_length_conversion[i];
        }
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
    fleet->n_ages = this->n_ages.get();
    fleet->n_lengths = this->n_lengths.get();
    fleet->n_years = this->n_years.get();
    fleet->observed_landings_units = this->observed_landings_units;
    fleet->observed_index_units = this->observed_index_units;

    fleet->fleet_observed_agecomp_data_id_m =
        interface_observed_agecomp_data_id_m.get();

    fleet->fleet_observed_lengthcomp_data_id_m =
        interface_observed_lengthcomp_data_id_m.get();

    fleet->fleet_observed_index_data_id_m =
        interface_observed_index_data_id_m.get();
    fleet->fleet_observed_landings_data_id_m =
        interface_observed_landings_data_id_m.get();

    fleet->fleet_selectivity_id_m = interface_selectivity_id_m.get();

    fleet->log_q.resize(this->log_q.size());
    for (size_t i = 0; i < this->log_q.size(); i++) {
      fleet->log_q[i] = this->log_q[i].initial_value_m;

      if (this->log_q[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Fleet." << this->id << ".log_q." << this->log_q[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(fleet->log_q[i]);
      }
      if (this->log_q[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Fleet." << this->id << ".log_q." << this->log_q[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(fleet->log_q[i]);
      }
    }

    FIMS_INFO_LOG("adding Fleet fmort object to TMB");
    fleet->log_Fmort.resize(this->log_Fmort.size());
    for (size_t i = 0; i < log_Fmort.size(); i++) {
      fleet->log_Fmort[i] = this->log_Fmort[i].initial_value_m;

      if (this->log_Fmort[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Fleet." << this->id << ".log_Fmort." << this->log_Fmort[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(fleet->log_Fmort[i]);
      }
      if (this->log_Fmort[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Fleet." << this->id << ".log_Fmort." << this->log_Fmort[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(fleet->log_Fmort[i]);
      }
    }
    // add to variable_map
    info->variable_map[this->log_Fmort.id_m] = &(fleet)->log_Fmort;

    if (this->n_lengths.get() > 0) {
      fleet->age_to_length_conversion.resize(
          this->age_to_length_conversion.size());

      if (this->age_to_length_conversion.size() !=
          (this->n_ages.get() * this->n_lengths.get())) {
        FIMS_ERROR_LOG(
            "age_to_length_conversion don't match, " +
            fims::to_string(this->age_to_length_conversion.size()) + " != " +
            fims::to_string((this->n_ages.get() * this->n_lengths.get())));
      }

      for (size_t i = 0; i < fleet->age_to_length_conversion.size(); i++) {
        fleet->age_to_length_conversion[i] =
            this->age_to_length_conversion[i].initial_value_m;
        FIMS_INFO_LOG(" adding Fleet length object to TMB in loop " +
                      fims::to_string(i) + " of " +
                      fims::to_string(fleet->age_to_length_conversion.size()));

        if (this->age_to_length_conversion[i].estimation_type_m.get() ==
            "fixed_effects") {
          ss.str("");
          ss << "Fleet." << this->id << ".age_to_length_conversion."
             << this->age_to_length_conversion[i].id_m;
          info->RegisterParameterName(ss.str());
          info->RegisterParameter(fleet->age_to_length_conversion[i]);
        }
        if (this->age_to_length_conversion[i].estimation_type_m.get() ==
            "random_effects") {
          FIMS_ERROR_LOG(
              "age_to_length_conversion cannot be set to random effects");
        }
      }

      info->variable_map[this->age_to_length_conversion.id_m] =
          &(fleet)->age_to_length_conversion;
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
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
    FIMS_INFO_LOG("adding Fleet object to TMB");
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
#endif

    return true;
  }

#endif
};

#endif
