/**
 * @file rcpp_models.hpp
 * @brief The Rcpp interface to declare different types of models. Allows
 * for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MODELS_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MODELS_HPP

#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include "common/def.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../models/fisheries_models.hpp"
#include "common/model.hpp"
#include "../../../utilities/fims_json.hpp"
#include "rcpp_population.hpp"
#include "rcpp_fleet.hpp"
#include "rcpp_growth.hpp"
#include "rcpp_distribution.hpp"
#include "rcpp_data.hpp"
#include "rcpp_maturity.hpp"
#include "rcpp_recruitment.hpp"
#include "rcpp_selectivity.hpp"
#include <valarray>
#include <cmath>
#include <mutex>

/**
 * @brief The FisheryModelInterfaceBase class is the base class for all fishery
 * models in the FIMS Rcpp interface. It inherits from the
 * FIMSRcppInterfaceBase.
 *
 */
class FisheryModelInterfaceBase : public FIMSRcppInterfaceBase {
 protected:
  /**
   * @brief The set of population ids that this fishery model operates on.
   */
  std::shared_ptr<std::set<uint32_t>> population_ids;
  /**
   * @brief Iterator for population ids.
   */
  typedef typename std::set<uint32_t>::iterator population_id_iterator;

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
  static std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>>
      live_objects;

  /**
   * @brief The constructor.
   */
  FisheryModelInterfaceBase() {
    this->id = FisheryModelInterfaceBase::id_g++;
    this->population_ids = std::make_shared<std::set<uint32_t>>();
    /* Create instance of map: key is id and value is pointer to
    FleetInterfaceBase */
    // FisheryModelInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Data Interface Base object
   *
   * @param other
   */
  FisheryModelInterfaceBase(const FisheryModelInterfaceBase &other)
      : population_ids(other.population_ids), id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~FisheryModelInterfaceBase() {}

  /**
   * @brief Serialize the fishery model to a JSON string.
   *
   * This method provides a standardized interface for converting the state of
   * a fishery model into a JSON-formatted string. The JSON output is intended
   * for use in reporting, diagnostics, or data exchange between C++ and R.
   * Derived classes should override this method to provide model-specific
   * serialization logic.
   *
   * @return A JSON string representing the current state of the model. The
   * base implementation returns a placeholder string indicating the method is
   * not yet implemented.
   */
  virtual std::string to_json() {
    return "std::string to_json() not yet implemented.";
  }

  /**
   * @brief Get the ID for the child fleet interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief Get the vector of fixed effect parameters for the model.
   *
   * @details Returns a numeric vector containing the fixed effect parameters
   * used in the model.
   * @return Rcpp::NumericVector of fixed effect parameters.
   */
  Rcpp::NumericVector get_fixed_parameters_vector() {
    std::shared_ptr<fims_info::Information<double>> info0 =
        fims_info::Information<double>::GetInstance();

    Rcpp::NumericVector p;

    for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++) {
      p.push_back(*info0->fixed_effects_parameters[i]);
    }

    return p;
  }

  /**
   * @brief Get the vector of random effect parameters for the model.
   *
   * @details Returns a numeric vector containing the random effect parameters
   * used in the model.
   * @return Rcpp::NumericVector of random effect parameters.
   */
  Rcpp::NumericVector get_random_parameters_vector() {
    std::shared_ptr<fims_info::Information<double>> d0 =
        fims_info::Information<double>::GetInstance();

    Rcpp::NumericVector p;

    for (size_t i = 0; i < d0->random_effects_parameters.size(); i++) {
      p.push_back(*d0->random_effects_parameters[i]);
    }

    return p;
  }

  /**
   * @brief Sum method to calculate the sum of an array or vector of doubles.
   *
   * @param v
   * @return double
   */
  double sum(const std::valarray<double> &v) {
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); i++) {
      sum += v[i];
    }
    return sum;
  }

  /**
   * @brief Sum method for a vector of doubles.
   *
   * @param v
   * @return double
   */
  double sum(const std::vector<double> &v) {
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); i++) {
      sum += v[i];
    }
    return sum;
  }

  /**
   * @brief Minimum method to calculate the minimum of an array or vector
   * of doubles.
   *
   * @param v
   * @return double
   */
  double min(const std::valarray<double> &v) {
    double min_value = v[0];
    for (size_t i = 1; i < v.size(); i++) {
      if (v[i] < min_value) {
        min_value = v[i];
      }
    }
    return min_value;
  }

  /**
   * @brief A function to compute the absolute value of a value array of
   * floating-point values. It is a wrapper around std::fabs.
   *
   * @param v A value array of floating-point values, where floating-point
   * values is anything with decimals.
   * @return std::valarray<double>
   */
  std::valarray<double> fabs(const std::valarray<double> &v) {
    std::valarray<double> result(v.size());
    for (size_t i = 0; i < v.size(); i++) {
      result[i] = std::fabs(v[i]);
    }
    return result;
  }
};

/**
 * @brief Rcpp interface for catch-at-age population derived quantities.
 *
 * @details This interface exposes derived quantities to R as explicit
 * VariableVector members.
 */
class CatchAtAgePopulationDerivedQuantitiesInterface {
 public:
  VariableVector total_landings_weight;
  VariableVector total_landings_numbers;
  VariableVector mortality_F;
  VariableVector mortality_M;
  VariableVector mortality_Z;
  VariableVector numbers_at_age;
  VariableVector unfished_numbers_at_age;
  VariableVector biomass;
  VariableVector spawning_biomass;
  VariableVector unfished_biomass;
  VariableVector unfished_spawning_biomass;
  VariableVector proportion_mature_at_age;
  VariableVector expected_recruitment;
  VariableVector sum_selectivity;

  /**
   * @brief Constructor.
   */
  CatchAtAgePopulationDerivedQuantitiesInterface() {}

  /**
   * @brief Resize all population derived quantities.
   *
   * @param n_years Number of model years.
   * @param n_ages Number of ages.
   */
  void Initialize(size_t n_years, size_t n_ages) {
    total_landings_weight.resize(n_years);
    total_landings_numbers.resize(n_years);
    mortality_F.resize(n_years * n_ages);
    mortality_M.resize(n_years * n_ages);
    mortality_Z.resize(n_years * n_ages);
    numbers_at_age.resize((n_years + 1) * n_ages);
    unfished_numbers_at_age.resize((n_years + 1) * n_ages);
    biomass.resize(n_years + 1);
    spawning_biomass.resize(n_years + 1);
    unfished_biomass.resize(n_years + 1);
    unfished_spawning_biomass.resize(n_years + 1);
    proportion_mature_at_age.resize((n_years + 1) * n_ages);
    expected_recruitment.resize(n_years + 1);
    sum_selectivity.resize(n_years * n_ages);
  }

  /**
   * AMH: doesn't this step need to happen in catch_at_age.hpp?
   * @brief Reset all population derived quantities to a value.
   *
   * @param value Value to assign to all entries.
   */
  void Fill(double value = 0.0) {
    total_landings_weight.fill(value);
    total_landings_numbers.fill(value);
    mortality_F.fill(value);
    mortality_M.fill(value);
    mortality_Z.fill(value);
    numbers_at_age.fill(value);
    unfished_numbers_at_age.fill(value);
    biomass.fill(value);
    spawning_biomass.fill(value);
    unfished_biomass.fill(value);
    unfished_spawning_biomass.fill(value);
    proportion_mature_at_age.fill(value);
    expected_recruitment.fill(value);
    sum_selectivity.fill(value);
  }

#ifdef TMB_MODEL  // AMH: why does this need to be in a TMB_MODEL wrapper?
  /**
   * @brief Pass derived quantities to variable_map.
   *
   * @param derived_quantities Backend population derived quantities.
   */
  template <typename Type>
  void LinkVariableMap(
      std::map<std::string, fims::Vector<Type>> &derived_quantities) {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->variable_map[total_landings_weight.id_m] =
        &(derived_quantities.at("total_landings_weight"));
    info->variable_map[total_landings_numbers.id_m] =
        &(derived_quantities.at("total_landings_numbers"));
    info->variable_map[mortality_F.id_m] =
        &(derived_quantities.at("mortality_F"));
    info->variable_map[mortality_M.id_m] =
        &(derived_quantities.at("mortality_M"));
    info->variable_map[mortality_Z.id_m] =
        &(derived_quantities.at("mortality_Z"));
    info->variable_map[numbers_at_age.id_m] =
        &(derived_quantities.at("numbers_at_age"));
    info->variable_map[unfished_numbers_at_age.id_m] =
        &(derived_quantities.at("unfished_numbers_at_age"));
    info->variable_map[biomass.id_m] = &(derived_quantities.at("biomass"));
    info->variable_map[spawning_biomass.id_m] =
        &(derived_quantities.at("spawning_biomass"));
    info->variable_map[unfished_biomass.id_m] =
        &(derived_quantities.at("unfished_biomass"));
    info->variable_map[unfished_spawning_biomass.id_m] =
        &(derived_quantities.at("unfished_spawning_biomass"));
    info->variable_map[proportion_mature_at_age.id_m] =
        &(derived_quantities.at("proportion_mature_at_age"));
    info->variable_map[expected_recruitment.id_m] =
        &(derived_quantities.at("expected_recruitment"));
    info->variable_map[sum_selectivity.id_m] =
        &(derived_quantities.at("sum_selectivity"));
  }
#endif
};

/**
 * @brief Rcpp interface for catch-at-age fleet derived quantities.
 *
 * @details This interface exposes derived quantities to R as explicit
 * VariableVector members.
 */
class CatchAtAgeFleetDerivedQuantitiesInterface {
 public:
  VariableVector landings_numbers_at_age;
  VariableVector landings_weight_at_age;
  VariableVector landings_numbers_at_length;
  VariableVector landings_weight;
  VariableVector landings_numbers;
  VariableVector landings_expected;
  VariableVector log_landings_expected;
  VariableVector agecomp_proportion;
  VariableVector lengthcomp_proportion;
  VariableVector index_numbers_at_age;
  VariableVector index_weight_at_age;
  VariableVector index_numbers_at_length;
  VariableVector index_weight;
  VariableVector index_numbers;
  VariableVector index_expected;
  VariableVector log_index_expected;
  VariableVector catch_index;
  VariableVector agecomp_expected;
  VariableVector lengthcomp_expected;

  /**
   * @brief Constructor.
   */
  CatchAtAgeFleetDerivedQuantitiesInterface() {}

  /**
   * @brief Resize all fleet derived quantities.
   *
   * @param n_years Number of model years.
   * @param n_ages Number of ages.
   * @param n_lengths Number of lengths.
   */
  void Initialize(size_t n_years, size_t n_ages, size_t n_lengths) {
    landings_numbers_at_age.resize(n_years * n_ages);
    landings_weight_at_age.resize(n_years * n_ages);
    landings_numbers_at_length.resize(n_years * n_lengths);
    landings_weight.resize(n_years);
    landings_numbers.resize(n_years);
    landings_expected.resize(n_years);
    log_landings_expected.resize(n_years);
    agecomp_proportion.resize(n_years * n_ages);
    lengthcomp_proportion.resize(n_years * n_lengths);
    index_numbers_at_age.resize(n_years * n_ages);
    index_weight_at_age.resize(n_years * n_ages);
    index_numbers_at_length.resize(n_years * n_lengths);
    index_weight.resize(n_years);
    index_numbers.resize(n_years);
    index_expected.resize(n_years);
    log_index_expected.resize(n_years);
    catch_index.resize(n_years);
    agecomp_expected.resize(n_years * n_ages);
    lengthcomp_expected.resize(n_years * n_lengths);
  }

  /**
   * AMH: doesn't this step need to happen in catch_at_age.hpp?
   * @brief Reset all fleet derived quantities to a value.
   *
   * @param value Value to assign to all entries.
   */
  void Fill(double value = 0.0) {
    landings_numbers_at_age.fill(value);
    landings_weight_at_age.fill(value);
    landings_numbers_at_length.fill(value);
    landings_weight.fill(value);
    landings_numbers.fill(value);
    landings_expected.fill(value);
    log_landings_expected.fill(value);
    agecomp_proportion.fill(value);
    lengthcomp_proportion.fill(value);
    index_numbers_at_age.fill(value);
    index_weight_at_age.fill(value);
    index_numbers_at_length.fill(value);
    index_weight.fill(value);
    index_numbers.fill(value);
    index_expected.fill(value);
    log_index_expected.fill(value);
    catch_index.fill(value);
    agecomp_expected.fill(value);
    lengthcomp_expected.fill(value);
  }

#ifdef TMB_MODEL  // AMH: why does this need to be in a TMB_MODEL wrapper?
  /**
   * @brief Pass derived quantities to variable_map.
   *
   * @param derived_quantities Backend fleet derived quantities.
   */
  template <typename Type>
  void LinkVariableMap(
      std::map<std::string, fims::Vector<Type>> &derived_quantities) {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->variable_map[landings_numbers_at_age.id_m] =
        &(derived_quantities.at("landings_numbers_at_age"));
    info->variable_map[landings_weight_at_age.id_m] =
        &(derived_quantities.at("landings_weight_at_age"));
    info->variable_map[landings_numbers_at_length.id_m] =
        &(derived_quantities.at("landings_numbers_at_length"));
    info->variable_map[landings_weight.id_m] =
        &(derived_quantities.at("landings_weight"));
    info->variable_map[landings_numbers.id_m] =
        &(derived_quantities.at("landings_numbers"));
    info->variable_map[landings_expected.id_m] =
        &(derived_quantities.at("landings_expected"));
    info->variable_map[log_landings_expected.id_m] =
        &(derived_quantities.at("log_landings_expected"));
    info->variable_map[agecomp_proportion.id_m] =
        &(derived_quantities.at("agecomp_proportion"));
    info->variable_map[lengthcomp_proportion.id_m] =
        &(derived_quantities.at("lengthcomp_proportion"));
    info->variable_map[index_numbers_at_age.id_m] =
        &(derived_quantities.at("index_numbers_at_age"));
    info->variable_map[index_weight_at_age.id_m] =
        &(derived_quantities.at("index_weight_at_age"));
    info->variable_map[index_numbers_at_length.id_m] =
        &(derived_quantities.at("index_numbers_at_length"));
    info->variable_map[index_weight.id_m] =
        &(derived_quantities.at("index_weight"));
    info->variable_map[index_numbers.id_m] =
        &(derived_quantities.at("index_numbers"));
    info->variable_map[index_expected.id_m] =
        &(derived_quantities.at("index_expected"));
    info->variable_map[log_index_expected.id_m] =
        &(derived_quantities.at("log_index_expected"));
    info->variable_map[catch_index.id_m] =
        &(derived_quantities.at("catch_index"));
    info->variable_map[agecomp_expected.id_m] =
        &(derived_quantities.at("agecomp_expected"));
    info->variable_map[lengthcomp_expected.id_m] =
        &(derived_quantities.at("lengthcomp_expected"));
  }
#endif
};

/**
 * @brief The CatchAtAgeInterface class is used to interface with the
 * CatchAtAge model. It inherits from the FisheryModelInterfaceBase class.
 */
class CatchAtAgeInterface : public FisheryModelInterfaceBase {
 public:
  typedef std::map<uint32_t, CatchAtAgePopulationDerivedQuantitiesInterface>
      PopulationDerivedQuantitiesMap;
  typedef std::map<uint32_t, CatchAtAgeFleetDerivedQuantitiesInterface>
      FleetDerivedQuantitiesMap;

  std::shared_ptr<PopulationDerivedQuantitiesMap> population_derived_quantities;
  std::shared_ptr<FleetDerivedQuantitiesMap> fleet_derived_quantities;

  /**
   * @brief The constructor.
   */
  CatchAtAgeInterface() : FisheryModelInterfaceBase() {
    this->population_derived_quantities =
        std::make_shared<PopulationDerivedQuantitiesMap>();
    this->fleet_derived_quantities =
        std::make_shared<FleetDerivedQuantitiesMap>();
    std::shared_ptr<CatchAtAgeInterface> caa =
        std::make_shared<CatchAtAgeInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(caa);
    FisheryModelInterfaceBase::live_objects[this->id] = caa;
  }

  /**
   * @brief Construct a new Catch At Age Interface object
   *
   * @param other
   */
  CatchAtAgeInterface(const CatchAtAgeInterface &other)
      : FisheryModelInterfaceBase(other),
        population_derived_quantities(other.population_derived_quantities),
        fleet_derived_quantities(other.fleet_derived_quantities) {}

  /**
   * Method to add a population id to the set of population ids.
   */
  void AddPopulation(uint32_t id) {
    this->population_ids->insert(id);
    (*this->population_derived_quantities)[id];

    std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
    pit = PopulationInterfaceBase::live_objects.find(id);
    if (pit != PopulationInterfaceBase::live_objects.end()) {
      std::shared_ptr<PopulationInterfaceBase> &pop = (*pit).second;
      pop->initialize_catch_at_age.set(true);
    } else {
      FIMS_ERROR_LOG("Population with id " + fims::to_string(id) +
                     " not found.");
    }
  }

  /**
   * @brief Initialize Rcpp population derived quantities.
   *
   * @param population_id The population ID.
   * @param n_years Number of model years.
   * @param n_ages Number of ages.
   */
  void InitializePopulationDerivedQuantities(uint32_t population_id,
                                             size_t n_years, size_t n_ages) {
    (*population_derived_quantities)[population_id].Initialize(n_years, n_ages);
  }

  /**
   * @brief Get Rcpp population derived quantities for a population ID.
   *
   * @param population_id The population ID.
   * @return CatchAtAgePopulationDerivedQuantitiesInterface&
   */
  CatchAtAgePopulationDerivedQuantitiesInterface *
  GetPopulationDerivedQuantities(uint32_t population_id) {
    std::map<uint32_t, CatchAtAgePopulationDerivedQuantitiesInterface>::iterator
        it = population_derived_quantities->find(population_id);
    if (it == population_derived_quantities->end()) {
      throw std::out_of_range(
          "CatchAtAgeInterface::GetPopulationDerivedQuantities: population_id "
          "not found");
    }
    return &(it->second);
  }

  /**
   * @brief Initialize Rcpp fleet derived quantities.
   *
   * @param fleet_id The fleet ID.
   * @param n_years Number of model years.
   * @param n_ages Number of ages.
   * @param n_lengths Number of lengths.
   */
  void InitializeFleetDerivedQuantities(uint32_t fleet_id, size_t n_years,
                                        size_t n_ages, size_t n_lengths) {
    (*fleet_derived_quantities)[fleet_id].Initialize(n_years, n_ages,
                                                     n_lengths);
  }

  /**
   * @brief Get Rcpp fleet derived quantities for a fleet ID.
   *
   * @param fleet_id The fleet ID.
   * @return CatchAtAgeFleetDerivedQuantitiesInterface&
   */
  CatchAtAgeFleetDerivedQuantitiesInterface *GetFleetDerivedQuantities(
      uint32_t fleet_id) {
    std::map<uint32_t, CatchAtAgeFleetDerivedQuantitiesInterface>::iterator it =
        fleet_derived_quantities->find(fleet_id);
    if (it == fleet_derived_quantities->end()) {
      throw std::out_of_range(
          "CatchAtAgeInterface::GetFleetDerivedQuantities: fleet_id not "
          "found");
    }
    return &(it->second);
  }

  /**
   * @brief Enable or disable reporting for the CatchAtAge model.
   *
   * @details This method is used to control whether reporting is performed for
   * the CatchAtAge model. The implementation may depend on TMB_MODEL.
   * @param report Boolean flag to enable (true) or disable (false) reporting.
   */
  void DoReporting(bool report) {
#ifdef TMB_MODEL
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();
    typename fims_info::Information<double>::model_map_iterator model_it;
    model_it = info->models_map.find(this->get_id());
    if (model_it != info->models_map.end()) {
      std::shared_ptr<fims_popdy::CatchAtAge<double>> model_ptr =
          std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
              (*model_it).second);
      model_ptr->do_reporting = report;
    }
#endif
  }

  /**
   * @brief Check if reporting is enabled for the CatchAtAge model.
   *
   * @details Returns true if reporting is enabled, false otherwise. The
   * implementation may depend on TMB_MODEL.
   * @return Boolean indicating reporting status.
   */
  bool IsReporting() {
#ifdef TMB_MODEL
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();
    typename fims_info::Information<double>::model_map_iterator model_it;
    model_it = info->models_map.find(this->get_id());
    if (model_it != info->models_map.end()) {
      std::shared_ptr<fims_popdy::CatchAtAge<double>> model_ptr =
          std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
              (*model_it).second);
      return model_ptr->do_reporting;
    }
    return false;
#else
    return false;
#endif
  }

  /**
   * @brief Method to get this id.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   *
   */
  virtual void finalize() {}

  /**
   * @brief Method to convert a population to a JSON string.
   */
  std::string population_to_json(PopulationInterface *population_interface) {
    std::stringstream ss;

    typename std::map<uint32_t,
                      std::shared_ptr<PopulationInterfaceBase>>::iterator
        pi_it;  // population interface iterator
    pi_it = PopulationInterfaceBase::live_objects.find(
        population_interface->get_id());
    if (pi_it == PopulationInterfaceBase::live_objects.end()) {
      FIMS_ERROR_LOG("Population with id " +
                     fims::to_string(population_interface->get_id()) +
                     " not found in live objects.");
      return "{}";  // Return empty JSON
    }

    std::shared_ptr<PopulationInterface> population_interface_ptr =
        std::dynamic_pointer_cast<PopulationInterface>((*pi_it).second);

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    typename fims_info::Information<double>::model_map_iterator model_it;
    model_it = info->models_map.find(this->get_id());
    std::shared_ptr<fims_popdy::CatchAtAge<double>> model_ptr =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            (*model_it).second);

    typename fims_info::Information<double>::population_iterator pit;

    pit = info->populations.find(population_interface->get_id());

    if (pit != info->populations.end()) {
      std::shared_ptr<fims_popdy::Population<double>> &pop = (*pit).second;
      ss << "{\n";

      ss << " \"module_name\": \"Population\",\n";
      ss << " \"population\": \"" << population_interface->name << "\",\n";
      ss << " \"module_id\": " << population_interface->id << ",\n";
      ss << " \"recruitment_id\": " << population_interface->recruitment_id
         << ",\n";
      ss << " \"growth_id\": " << population_interface->growth_id << ",\n";
      ss << " \"maturity_id\": " << population_interface->maturity_id << ",\n";

      ss << " \"parameters\": [\n";
      for (size_t i = 0; i < pop->log_M.size(); i++) {
        population_interface_ptr->log_M[i].final_value_m = pop->log_M[i];
      }

      ss << "{\n \"name\": \"log_M\",\n";
      ss << " \"id\":" << population_interface->log_M.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [" << "\"n_years\", \"n_ages\"" << "],\n";
      ss << "  \"dimensions\": [" << population_interface->n_years.get() << ", "
         << population_interface->n_ages.get() << "]\n},\n";
      ss << " \"values\": " << population_interface->log_M << "\n\n";
      ss << "},\n";

      for (size_t i = 0; i < pop->log_f_multiplier.size(); i++) {
        population_interface_ptr->log_f_multiplier[i].final_value_m =
            pop->log_f_multiplier[i];
      }

      ss << "{\n \"name\": \"log_f_multiplier\",\n";
      ss << " \"id\":" << population_interface->log_f_multiplier.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [" << "\"n_years\"" << "],\n";
      ss << "  \"dimensions\": [" << population_interface->n_years.get()
         << "]\n},\n";
      ss << " \"values\": " << population_interface->log_f_multiplier << "\n\n";
      ss << "},\n";

      for (size_t i = 0; i < pop->spawning_biomass_ratio.size(); i++) {
        population_interface_ptr->spawning_biomass_ratio[i].final_value_m =
            pop->spawning_biomass_ratio[i];
      }

      ss << "{\n \"name\": \"spawning_biomass_ratio\",\n";
      ss << " \"id\":" << population_interface->spawning_biomass_ratio.id_m
         << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [" << "\"n_years\"" << "],\n";
      ss << "  \"dimensions\": [" << (population_interface->n_years.get() + 1)
         << "]\n},\n";
      ss << " \"values\": " << population_interface->spawning_biomass_ratio
         << "\n\n";
      ss << "},\n";

      for (size_t i = 0; i < pop->log_init_naa.size(); i++) {
        population_interface_ptr->log_init_naa[i].final_value_m =
            pop->log_init_naa[i];
      }
      ss << " {\n\"name\": \"log_init_naa\",\n";
      ss << "  \"id\":" << population_interface->log_init_naa.id_m << ",\n";
      ss << "  \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [" << "\"n_ages\"" << "],\n";
      ss << "  \"dimensions\": [" << population_interface->n_ages.get()
         << "]\n},\n";

      ss << "  \"values\":" << population_interface->log_init_naa << "\n";
      ss << "},\n";

      for (size_t i = 0; i < population_interface->proportion_female.size();
           i++) {
        population_interface_ptr->proportion_female[i].final_value_m =
            pop->proportion_female.get_force_scalar(i);
      }
      ss << " {\n\"name\": \"proportion_female\",\n";
      ss << "  \"id\":" << population_interface->proportion_female.id_m
         << ",\n";
      ss << "  \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [" << "\"n_ages\"" << "],\n";
      ss << "  \"dimensions\": ["
         << population_interface->proportion_female.size() << "]\n},\n";

      ss << "  \"values\":" << population_interface->proportion_female << "\n";
      ss << "}],\n";

      ss << " \"derived_quantities\": [\n";

      std::map<std::string, fims::Vector<double>> dqs =
          model_ptr->GetPopulationDerivedQuantities(
              population_interface->get_id());

      std::map<std::string, fims_popdy::DimensionInfo> dim_info =
          model_ptr->GetPopulationDimensionInfo(population_interface->get_id());
      ss << this->derived_quantities_component_to_json(dqs, dim_info)
         << " ]}\n";
    } else {
      ss << "{\n";
      ss << " \"name\": \"Population\",\n";

      ss << " \"type\": \"population\",\n";
      ss << " \"tag\": \"" << population_interface->get_id()
         << " not found in Information.\",\n";
      ss << " \"id\": " << population_interface->get_id() << ",\n";
      ss << " \"recruitment_id\": " << population_interface->recruitment_id
         << ",\n";
      ss << " \"growth_id\": " << population_interface->growth_id << ",\n";
      ss << " \"maturity_id\": " << population_interface->maturity_id << ",\n";
      ss << " \"derived_quantities\": []}\n";
    }

    return ss.str();
  }

  /**
   * This function is used to convert the derived quantities of a population or
   * fleet to a JSON string. This function is used to create the JSON output for
   * the CatchAtAge model.
   */
  std::string derived_quantity_to_json(
      std::map<std::string, fims::Vector<double>>::iterator it,
      const fims_popdy::DimensionInfo &dim_info) {
    std::stringstream ss;
    fims::Vector<double> &dq = (*it).second;
    std::stringstream dim_entry;
    // gather dimension information
    switch (dim_info.ndims) {
      case 1:
        dim_entry << "\"dimensionality\": {\n";
        dim_entry << "  \"header\": [\"" << dim_info.dim_names[0] << "\"],\n";
        dim_entry << "  \"dimensions\": [";
        for (size_t i = 0; i < dim_info.dims.size(); ++i) {
          if (i > 0) dim_entry << ", ";
          dim_entry << dim_info.dims[i];
        }
        dim_entry << "]\n";
        dim_entry << "}";
        break;
      case 2:
        dim_entry << "\"dimensionality\": {\n";
        dim_entry << "  \"header\": [\"" << dim_info.dim_names[0] << "\", \""
                  << dim_info.dim_names[1] << "\"],\n";
        dim_entry << "  \"dimensions\": [";
        for (size_t i = 0; i < dim_info.dims.size(); ++i) {
          if (i > 0) dim_entry << ", ";
          dim_entry << dim_info.dims[i];
        }
        dim_entry << "]\n";
        dim_entry << "}";
        break;
      case 3:
        dim_entry << "\"dimensionality\": {\n";
        dim_entry << "  \"header\": [\"" << dim_info.dim_names[0] << "\", \""
                  << dim_info.dim_names[1] << "\", \"" << dim_info.dim_names[2]
                  << "\"],\n";
        dim_entry << "  \"dimensions\": [";
        for (size_t i = 0; i < dim_info.dims.size(); ++i) {
          if (i > 0) dim_entry << ", ";
          dim_entry << dim_info.dims[i];
        }
        dim_entry << "]\n";
        dim_entry << "}";
        break;
      default:
        dim_entry << "\"dimensionality\": {\n";
        dim_entry << "  \"header\": [],\n";
        dim_entry << "  \"dimensions\": []\n";
        dim_entry << "}";
        break;
    }

    // build JSON string
    ss << "{\n";
    ss << "\"name\":\"" << (*it).first << "\",\n";
    ss << dim_entry.str() << ",\n";
    ss << "\"value\":[";
    ss << std::fixed << std::setprecision(10);
    if (dq.size() > 0) {
      for (size_t i = 0; i < dq.size() - 1; i++) {
        if (dq[i] != dq[i])  // check for NaN
        {
          ss << "-999" << ", ";
        } else {
          ss << dq[i] << ", ";
        }
      }
      if (dq[dq.size() - 1] != dq[dq.size() - 1])  // check for NaN
      {
        ss << "-999]" << "\n";
      } else {
        ss << dq[dq.size() - 1] << "]\n";
      }
    } else {
      ss << "]\n";
    }
    ss << "}";

    return ss.str();
  }

  /**
   * @brief Send the fleet-based derived quantities to the json file.
   * @return std::string
   */
  std::string derived_quantities_component_to_json(
      std::map<std::string, fims::Vector<double>> &dqs,
      std::map<std::string, fims_popdy::DimensionInfo> &dim_info) {
    std::stringstream ss;
    std::map<std::string, fims_popdy::DimensionInfo>::iterator dim_info_it;
    std::map<std::string, fims::Vector<double>>::iterator it;
    std::map<std::string, fims::Vector<double>>::iterator end_it;
    end_it = dqs.end();
    typename std::map<std::string, fims::Vector<double>>::iterator
        second_to_last;
    second_to_last = dqs.end();
    if (it != end_it) {
      second_to_last--;
    }

    it = dqs.begin();
    for (; it != second_to_last; ++it) {
      dim_info_it = dim_info.find(it->first);
      ss << this->derived_quantity_to_json(it, dim_info_it->second) << ",\n";
    }

    dim_info_it = dim_info.find(second_to_last->first);
    if (dim_info_it != dim_info.end()) {
      ss << this->derived_quantity_to_json(second_to_last, dim_info_it->second)
         << "\n";
    } else {
      ss << "{}";
      // Handle case where dimension info is not found
    }
    return ss.str();
  }

  /**
   * @brief Method to convert a fleet to a JSON string.
   */
  std::string fleet_to_json(FleetInterface *fleet_interface) {
    std::stringstream ss;

    if (!fleet_interface) {
      FIMS_ERROR_LOG(
          "Fleet pointer is null; cannot get id. Not found in live objects.");
      return "{}";  // Return empty JSON
    }

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    typename fims_info::Information<double>::model_map_iterator model_it;
    model_it = info->models_map.find(this->get_id());
    std::shared_ptr<fims_popdy::CatchAtAge<double>> model_ptr =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            (*model_it).second);

    typename fims_info::Information<double>::fleet_iterator fit;

    fit = info->fleets.find(fleet_interface->get_id());

    if (fit != info->fleets.end()) {
      std::shared_ptr<fims_popdy::Fleet<double>> &fleet = (*fit).second;

      ss << "{\n";
      ss << " \"module_name\": \"Fleet\",\n";
      ss << " \"fleet\": \"" << fleet_interface->name << "\",\n";
      ss << " \"module_id\": " << fleet_interface->id << ",\n";
      ss << " \"n_ages\": " << fleet_interface->n_ages.get() << ",\n";
      ss << " \"n_years\": " << fleet_interface->n_years.get() << ",\n";
      ss << " \"n_lengths\": " << fleet_interface->n_lengths.get() << ",\n";
      ss << "\"data_ids\" : [\n";
      ss << "{\"agecomp\": " << fleet_interface->GetObservedAgeCompDataID()
         << "},\n";
      ss << "{\"lengthcomp\": "
         << fleet_interface->GetObservedLengthCompDataID() << "},\n";
      ss << "{\"index\": " << fleet_interface->GetObservedIndexDataID()
         << "},\n";
      ss << "{\"landings\": " << fleet_interface->GetObservedLandingsDataID()
         << "}\n";
      ss << "],\n";
      ss << "\"parameters\": [\n";
      ss << "{\n";
      for (size_t i = 0; i < fleet_interface->log_Fmort.size(); i++) {
        fleet_interface->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
      }

      ss << " \"name\": \"log_Fmort\",\n";
      ss << " \"id\":" << fleet_interface->log_Fmort.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [\"" << "n_years" << "\"],\n";
      ss << "  \"dimensions\": [" << fleet_interface->n_years.get()
         << "]\n},\n";
      ss << " \"values\": " << fleet_interface->log_Fmort << "},\n";

      ss << " {\n";
      for (size_t i = 0; i < fleet->log_q.size(); i++) {
        fleet_interface->log_q[i].final_value_m = fleet->log_q[i];
      }
      ss << " \"name\": \"log_q\",\n";
      ss << " \"id\":" << fleet_interface->log_q.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [\"" << "na" << "\"],\n";
      ss << "  \"dimensions\": [" << fleet->log_q.size() << "]\n},\n";

      ss << " \"values\": " << fleet_interface->log_q << "}\n";

      ss << "], \"derived_quantities\": [";

      std::map<std::string, fims::Vector<double>> dqs =
          model_ptr->GetFleetDerivedQuantities(fleet_interface->get_id());
      std::map<std::string, fims_popdy::DimensionInfo> dim_info =
          model_ptr->GetFleetDimensionInfo(fleet_interface->get_id());
      ss << this->derived_quantities_component_to_json(dqs, dim_info) << "]}\n";
    } else {
      ss << "{\n";
      ss << " \"name\": \"Fleet\",\n";
      ss << " \"type\": \"fleet\",\n";
      ss << " \"tag\": \"" << fleet_interface->get_id()
         << " not found in Information.\",\n";
      ss << " \"derived_quantities\": []}\n";
    }
    return ss.str();
  }

  /**
   * @copydoc FisheryModelInterfaceBase::to_json
   */
  virtual std::string to_json() {
    std::set<uint32_t> recruitment_ids;
    std::set<uint32_t> growth_ids;
    std::set<uint32_t> maturity_ids;
    std::set<uint32_t> selectivity_ids;
    std::set<uint32_t> fleet_ids;
    // gather sub-module info from population and fleets
    typename std::set<uint32_t>::iterator module_id_it;  // generic
    typename std::set<uint32_t>::iterator pit;
    typename std::set<uint32_t>::iterator fids;
    for (pit = this->population_ids->begin();
         pit != this->population_ids->end(); pit++) {
      std::shared_ptr<PopulationInterface> population_interface =
          std::dynamic_pointer_cast<PopulationInterface>(
              PopulationInterfaceBase::live_objects[*pit]);
      if (population_interface) {
        recruitment_ids.insert(population_interface->recruitment_id.get());
        growth_ids.insert(population_interface->growth_id.get());
        maturity_ids.insert(population_interface->maturity_id.get());

        for (fids = population_interface->fleet_ids->begin();
             fids != population_interface->fleet_ids->end(); fids++) {
          fleet_ids.insert(*fids);
        }
      }
    }

    for (fids = fleet_ids.begin(); fids != fleet_ids.end(); fids++) {
      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[*fids]);
      if (fleet_interface) {
        selectivity_ids.insert(fleet_interface->GetSelectivityID());
      }
    }

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    std::shared_ptr<fims_popdy::CatchAtAge<double>> model =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            info->models_map[this->get_id()]);

    std::shared_ptr<fims_model::Model<double>> model_internal =
        fims_model::Model<double>::GetInstance();

#ifdef TMB_MODEL
    model->do_reporting = false;
#endif

    double value = model_internal->Evaluate();

    std::stringstream ss;

    ss.str("");

    ss << "{\n";
    ss << " \"name\": \"CatchAtAge\",\n";
    ss << " \"type\": \"model\",\n";
    ss << " \"estimation_framework\": ";
#ifdef TMB_MODEL
    ss << "\"Template_Model_Builder (TMB)\",";
#else
    ss << "\"FIMS\",";
#endif
    ss << " \"id\": " << this->get_id() << ",\n";
    ss << " \"objective_function_value\": " << sanitize_val(value) << ",\n";
    ss << "\"growth\":[\n";
    for (module_id_it = growth_ids.begin(); module_id_it != growth_ids.end();
         module_id_it++) {
      std::shared_ptr<GrowthInterfaceBase> growth_interface =
          GrowthInterfaceBase::live_objects[*module_id_it];

      if (growth_interface != NULL) {
        growth_interface->finalize();
        ss << growth_interface->to_json();
        if (std::next(module_id_it) != growth_ids.end()) {
          ss << ", ";
        }
      }
    }

    ss << "],\n";

    ss << "\"recruitment\": [\n";
    for (module_id_it = recruitment_ids.begin();
         module_id_it != recruitment_ids.end(); module_id_it++) {
      std::shared_ptr<RecruitmentInterfaceBase> recruitment_interface =
          RecruitmentInterfaceBase::live_objects[*module_id_it];
      if (recruitment_interface) {
        recruitment_interface->finalize();
        ss << recruitment_interface->to_json();
        if (std::next(module_id_it) != recruitment_ids.end()) {
          ss << ", ";
        }
      }
    }
    ss << "],\n";

    ss << "\"maturity\": [\n";
    for (module_id_it = maturity_ids.begin();
         module_id_it != maturity_ids.end(); module_id_it++) {
      std::shared_ptr<MaturityInterfaceBase> maturity_interface =
          MaturityInterfaceBase::live_objects[*module_id_it];
      if (maturity_interface) {
        maturity_interface->finalize();
        ss << maturity_interface->to_json();
        if (std::next(module_id_it) != maturity_ids.end()) {
          ss << ", ";
        }
      }
    }
    ss << "],\n";

    ss << "\"selectivity\": [\n";
    for (module_id_it = selectivity_ids.begin();
         module_id_it != selectivity_ids.end(); module_id_it++) {
      std::shared_ptr<SelectivityInterfaceBase> selectivity_interface =
          SelectivityInterfaceBase::live_objects[*module_id_it];
      if (selectivity_interface) {
        selectivity_interface->finalize();
        ss << selectivity_interface->to_json();
        if (std::next(module_id_it) != selectivity_ids.end()) {
          ss << ", ";
        }
      }
    }
    ss << "],\n";

    ss << " \"population_ids\": [";
    for (pit = this->population_ids->begin();
         pit != this->population_ids->end(); pit++) {
      ss << *pit;
      if (std::next(pit) != this->population_ids->end()) {
        ss << ", ";
      }
    }
    ss << "],\n";
    ss << " \"fleet_ids\": [";

    for (fids = fleet_ids.begin(); fids != fleet_ids.end(); fids++) {
      ss << *fids;
      if (std::next(fids) != fleet_ids.end()) {
        ss << ", ";
      }
    }
    ss << "],\n";
    ss << "\"populations\": [\n";
    typename std::set<uint32_t>::iterator pop_it;
    typename std::set<uint32_t>::iterator pop_end_it;
    pop_end_it = this->population_ids->end();
    typename std::set<uint32_t>::iterator pop_second_to_last_it;
    if (pop_end_it != this->population_ids->begin()) {
      pop_second_to_last_it = std::prev(pop_end_it);
    } else {
      pop_second_to_last_it = pop_end_it;
    }
    for (pop_it = this->population_ids->begin();
         pop_it != pop_second_to_last_it; pop_it++) {
      std::shared_ptr<PopulationInterface> population_interface =
          std::dynamic_pointer_cast<PopulationInterface>(
              PopulationInterfaceBase::live_objects[*pop_it]);
      if (population_interface) {
        std::set<uint32_t>::iterator fids;
        for (fids = population_interface->fleet_ids->begin();
             fids != population_interface->fleet_ids->end(); fids++) {
          fleet_ids.insert(*fids);
        }
        population_interface->finalize();
        ss << this->population_to_json(population_interface.get()) << ",";
      } else {
        FIMS_ERROR_LOG("Population with id " + fims::to_string(*pop_it) +
                       " not found in live objects.");
        ss << "{}";  // Return empty JSON for this population
      }
    }

    std::shared_ptr<PopulationInterface> population_interface =
        std::dynamic_pointer_cast<PopulationInterface>(
            PopulationInterfaceBase::live_objects[*pop_second_to_last_it]);
    if (population_interface) {
      std::set<uint32_t>::iterator fids;
      for (fids = population_interface->fleet_ids->begin();
           fids != population_interface->fleet_ids->end(); fids++) {
        fleet_ids.insert(*fids);
      }
      ss << this->population_to_json(population_interface.get());
    } else {
      FIMS_ERROR_LOG("Population with id " + fims::to_string(*pop_it) +
                     " not found in live objects.");
      ss << "{}";  // Return empty JSON for this population
    }

    ss << "]";
    ss << ",\n";
    ss << "\"fleets\": [\n";

    typename std::set<uint32_t>::iterator fleet_it;
    typename std::set<uint32_t>::iterator fleet_end_it;
    fleet_end_it = fleet_ids.end();
    typename std::set<uint32_t>::iterator fleet_second_to_last_it;

    if (fleet_end_it != fleet_ids.begin()) {
      fleet_second_to_last_it = std::prev(fleet_end_it);
    }
    for (fleet_it = fleet_ids.begin(); fleet_it != fleet_second_to_last_it;
         fleet_it++) {
      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[*fleet_it]);
      if (fleet_interface) {
        fleet_interface->finalize();
        ss << this->fleet_to_json(fleet_interface.get()) << ",";
      } else {
        FIMS_ERROR_LOG("Fleet with id " + fims::to_string(*fleet_it) +
                       " not found in live objects.");
        ss << "{}";  // Return empty JSON for this fleet
      }
    }
    std::shared_ptr<FleetInterface> fleet_interface =
        std::dynamic_pointer_cast<FleetInterface>(
            FleetInterfaceBase::live_objects[*fleet_second_to_last_it]);
    if (fleet_interface) {
      ss << this->fleet_to_json(fleet_interface.get());
    } else {
      FIMS_ERROR_LOG("Fleet with id " + fims::to_string(*fleet_it) +
                     " not found in live objects.");
      ss << "{}";  // Return empty JSON for this fleet
    }

    ss << "],\n";

    ss << "\"density_components\": [\n";

    typename std::map<
        uint32_t, std::shared_ptr<DistributionsInterfaceBase>>::iterator dit;
    for (dit = DistributionsInterfaceBase::live_objects.begin();
         dit != DistributionsInterfaceBase::live_objects.end(); ++dit) {
      std::shared_ptr<DistributionsInterfaceBase> dist_interface =
          (*dit).second;
      if (dist_interface) {
        dist_interface->finalize();
        ss << dist_interface->to_json();
        if (std::next(dit) != DistributionsInterfaceBase::live_objects.end()) {
          ss << ",\n";
        }
      }
    }
    ss << "\n],\n";
    ss << "\"data\": [\n";
    typename std::map<uint32_t, std::shared_ptr<DataInterfaceBase>>::iterator
        d_it;
    for (d_it = DataInterfaceBase::live_objects.begin();
         d_it != DataInterfaceBase::live_objects.end(); ++d_it) {
      std::shared_ptr<DataInterfaceBase> data_interface = (*d_it).second;
      if (data_interface) {
        data_interface->finalize();
        ss << data_interface->to_json();
        if (std::next(d_it) != DataInterfaceBase::live_objects.end()) {
          ss << ",\n";
        }
      }
    }
    ss << "\n]\n";
    ss << "}\n";
#ifdef TMB_MODEL
    model->do_reporting = true;
#endif
    return fims::JsonParser::PrettyFormatJSON(ss.str());
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::CatchAtAge<Type>> model =
        std::make_shared<fims_popdy::CatchAtAge<Type>>();

    population_id_iterator it;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it) {
      model->AddPopulation((*it));
    }

    std::set<uint32_t> fleet_ids;  // all fleets in the model
    typedef typename std::set<uint32_t>::iterator fleet_ids_iterator;

    // add to Information
    info->models_map[this->get_id()] = model;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it) {
      auto it2 = PopulationInterfaceBase::live_objects.find(*it);
      if (it2 == PopulationInterfaceBase::live_objects.end()) {
        throw std::runtime_error("Population ID " + std::to_string(*it) +
                                 " not found in live_objects");
      }
      auto population =
          std::dynamic_pointer_cast<PopulationInterface>(it2->second);
      model->InitializePopulationDerivedQuantities(population->id);
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->GetPopulationDerivedQuantities(population->id);

      std::map<std::string, fims_popdy::DimensionInfo>
          &derived_quantities_dim_info =
              model->GetPopulationDimensionInfo(population->id);

      std::stringstream ss;

      derived_quantities["total_landings_weight"] =
          fims::Vector<Type>(population->n_years.get());

      derived_quantities_dim_info["total_landings_weight"] =
          fims_popdy::DimensionInfo(
              "total_landings_weight",
              fims::Vector<int>{(int)population->n_years.get()},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["total_landings_numbers"] =
          fims::Vector<Type>(population->n_years.get());

      derived_quantities_dim_info["total_landings_numbers"] =
          fims_popdy::DimensionInfo(
              "total_landings_numbers",
              fims::Vector<int>{population->n_years.get()},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["mortality_F"] = fims::Vector<Type>(
          population->n_years.get() * population->n_ages.get());
      derived_quantities_dim_info["mortality_F"] = fims_popdy::DimensionInfo(
          "mortality_F",
          fims::Vector<int>{population->n_years.get(),
                            population->n_ages.get()},
          fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["mortality_M"] = fims::Vector<Type>(
          population->n_years.get() * population->n_ages.get());
      derived_quantities_dim_info["mortality_M"] = fims_popdy::DimensionInfo(
          "mortality_M",
          fims::Vector<int>{population->n_years.get(),
                            population->n_ages.get()},
          fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["mortality_Z"] = fims::Vector<Type>(
          population->n_years.get() * population->n_ages.get());
      derived_quantities_dim_info["mortality_Z"] = fims_popdy::DimensionInfo(
          "mortality_Z",
          fims::Vector<int>{population->n_years.get(),
                            population->n_ages.get()},
          fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["numbers_at_age"] = fims::Vector<Type>(
          (population->n_years.get() + 1) * population->n_ages.get());
      derived_quantities_dim_info["numbers_at_age"] = fims_popdy::DimensionInfo(
          "numbers_at_age",
          fims::Vector<int>{(population->n_years.get() + 1),
                            population->n_ages.get()},
          fims::Vector<std::string>{"n_years+1", "n_ages"});

      derived_quantities["unfished_numbers_at_age"] = fims::Vector<Type>(
          (population->n_years.get() + 1) * population->n_ages.get());
      derived_quantities_dim_info["unfished_numbers_at_age"] =
          fims_popdy::DimensionInfo(
              "unfished_numbers_at_age",
              fims::Vector<int>{(population->n_years.get() + 1),
                                population->n_ages.get()},
              fims::Vector<std::string>{"n_years+1", "n_ages"});

      derived_quantities["biomass"] =
          fims::Vector<Type>((population->n_years.get() + 1));
      derived_quantities_dim_info["biomass"] = fims_popdy::DimensionInfo(
          "biomass", fims::Vector<int>{(population->n_years.get() + 1)},
          fims::Vector<std::string>{"n_years+1"});

      derived_quantities["spawning_biomass"] =
          fims::Vector<Type>((population->n_years.get() + 1));
      derived_quantities_dim_info["spawning_biomass"] =
          fims_popdy::DimensionInfo(
              "spawning_biomass",
              fims::Vector<int>{(population->n_years.get() + 1)},
              fims::Vector<std::string>{"n_years+1"});

      derived_quantities["unfished_biomass"] =
          fims::Vector<Type>((population->n_years.get() + 1));
      derived_quantities_dim_info["unfished_biomass"] =
          fims_popdy::DimensionInfo(
              "unfished_biomass",
              fims::Vector<int>{(population->n_years.get() + 1)},
              fims::Vector<std::string>{"n_years+1"});

      derived_quantities["unfished_spawning_biomass"] =
          fims::Vector<Type>((population->n_years.get() + 1));
      derived_quantities_dim_info["unfished_spawning_biomass"] =
          fims_popdy::DimensionInfo(
              "unfished_spawning_biomass",
              fims::Vector<int>{(population->n_years.get() + 1)},
              fims::Vector<std::string>{"n_years+1"});

      derived_quantities["proportion_mature_at_age"] = fims::Vector<Type>(
          (population->n_years.get() + 1) * population->n_ages.get());
      derived_quantities_dim_info["proportion_mature_at_age"] =
          fims_popdy::DimensionInfo(
              "proportion_mature_at_age",
              fims::Vector<int>{(population->n_years.get() + 1),
                                population->n_ages.get()},
              fims::Vector<std::string>{"n_years+1", "n_ages"});

      derived_quantities["expected_recruitment"] =
          fims::Vector<Type>((population->n_years.get() + 1));
      derived_quantities_dim_info["expected_recruitment"] =
          fims_popdy::DimensionInfo(
              "expected_recruitment",
              fims::Vector<int>{(population->n_years.get() + 1)},
              fims::Vector<std::string>{"n_years+1"});

      derived_quantities["sum_selectivity"] = fims::Vector<Type>(
          population->n_years.get() * population->n_ages.get());
      derived_quantities_dim_info["sum_selectivity"] =
          fims_popdy::DimensionInfo(
              "sum_selectivity",
              fims::Vector<int>{population->n_years.get(),
                                population->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      (*this->population_derived_quantities)[population->id].LinkVariableMap(
          derived_quantities);

      for (fleet_ids_iterator fit = population->fleet_ids->begin();
           fit != population->fleet_ids->end(); ++fit) {
        fleet_ids.insert(*fit);
      }
    }

    for (fleet_ids_iterator it = fleet_ids.begin(); it != fleet_ids.end();
         ++it) {
      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[(*it)]);
      model->InitializeFleetDerivedQuantities(fleet_interface->id);
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->GetFleetDerivedQuantities(fleet_interface->id);

      std::map<std::string, fims_popdy::DimensionInfo>
          &derived_quantities_dim_info =
              model->GetFleetDimensionInfo(fleet_interface->id);

      // initialize derive quantities
      // landings
      derived_quantities["landings_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["landings_numbers_at_age"] =
          fims_popdy::DimensionInfo(
              "landings_numbers_at_age",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["landings_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["landings_weight_at_age"] =
          fims_popdy::DimensionInfo(
              "landings_weight_at_age",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["landings_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_lengths.get());
      derived_quantities_dim_info["landings_numbers_at_length"] =
          fims_popdy::DimensionInfo(
              "landings_numbers_at_length",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_lengths.get()},
              fims::Vector<std::string>{"n_years", "n_lengths"});

      derived_quantities["landings_weight"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["landings_weight"] =
          fims_popdy::DimensionInfo(
              "landings_weight",
              fims::Vector<int>{(fleet_interface->n_years.get())},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["landings_numbers"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["landings_numbers"] =
          fims_popdy::DimensionInfo(
              "landings_numbers",
              fims::Vector<int>{(fleet_interface->n_years.get())},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["landings_expected"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["landings_expected"] =
          fims_popdy::DimensionInfo(
              "landings_expected",
              fims::Vector<int>{(fleet_interface->n_years.get())},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["log_landings_expected"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["log_landings_expected"] =
          fims_popdy::DimensionInfo(
              "log_landings_expected",
              fims::Vector<int>{(fleet_interface->n_years.get())},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["agecomp_proportion"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["agecomp_proportion"] =
          fims_popdy::DimensionInfo(
              "agecomp_proportion",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["lengthcomp_proportion"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_lengths.get());
      derived_quantities_dim_info["lengthcomp_proportion"] =
          fims_popdy::DimensionInfo(
              "lengthcomp_proportion",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_lengths.get()},
              fims::Vector<std::string>{"n_years", "n_lengths"});

      // index
      derived_quantities["index_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["index_numbers_at_age"] =
          fims_popdy::DimensionInfo(
              "index_numbers_at_age",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["index_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["index_weight_at_age"] =
          fims_popdy::DimensionInfo(
              "index_weight_at_age",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["index_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["index_weight_at_age"] =
          fims_popdy::DimensionInfo(
              "index_weight_at_age",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_ages.get()},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["index_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_lengths.get());
      derived_quantities_dim_info["index_numbers_at_length"] =
          fims_popdy::DimensionInfo(
              "index_numbers_at_length",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                fleet_interface->n_lengths.get()},
              fims::Vector<std::string>{"n_years", "n_lengths"});
      derived_quantities["index_weight"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["index_weight"] = fims_popdy::DimensionInfo(
          "index_weight", fims::Vector<int>{(fleet_interface->n_years.get())},
          fims::Vector<std::string>{"n_years"});

      derived_quantities["index_numbers"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["index_numbers"] = fims_popdy::DimensionInfo(
          "index_numbers", fims::Vector<int>{(fleet_interface->n_years.get())},
          fims::Vector<std::string>{"n_years"});

      derived_quantities["index_expected"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["index_expected"] = fims_popdy::DimensionInfo(
          "index_expected", fims::Vector<int>{(fleet_interface->n_years.get())},
          fims::Vector<std::string>{"n_years"});

      derived_quantities["log_index_expected"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["log_index_expected"] =
          fims_popdy::DimensionInfo(
              "log_index_expected",
              fims::Vector<int>{(fleet_interface->n_years.get())},
              fims::Vector<std::string>{"n_years"});

      derived_quantities["catch_index"] =
          fims::Vector<Type>(fleet_interface->n_years.get());
      derived_quantities_dim_info["catch_index"] = fims_popdy::DimensionInfo(
          "catch_index", fims::Vector<int>{(fleet_interface->n_years.get())},
          fims::Vector<std::string>{"n_years"});

      derived_quantities["agecomp_expected"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_ages.get());
      derived_quantities_dim_info["agecomp_expected"] =
          fims_popdy::DimensionInfo(
              "agecomp_expected",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                (fleet_interface->n_ages.get())},
              fims::Vector<std::string>{"n_years", "n_ages"});

      derived_quantities["lengthcomp_expected"] = fims::Vector<Type>(
          fleet_interface->n_years.get() * fleet_interface->n_lengths.get());
      derived_quantities_dim_info["lengthcomp_expected"] =
          fims_popdy::DimensionInfo(
              "lengthcomp_expected",
              fims::Vector<int>{(fleet_interface->n_years.get()),
                                (fleet_interface->n_lengths.get())},
              fims::Vector<std::string>{"n_years", "n_lengths"});

      (*this->fleet_derived_quantities)[fleet_interface->id].LinkVariableMap(
          derived_quantities);

      // replace elements in the variable map
      info->variable_map[fleet_interface->log_landings_expected.id_m] =
          &(derived_quantities["log_landings_expected"]);
      info->variable_map[fleet_interface->log_index_expected.id_m] =
          &(derived_quantities["log_index_expected"]);
      info->variable_map[fleet_interface->agecomp_expected.id_m] =
          &(derived_quantities["agecomp_expected"]);
      info->variable_map[fleet_interface->agecomp_proportion.id_m] =
          &(derived_quantities["agecomp_proportion"]);
      info->variable_map[fleet_interface->lengthcomp_expected.id_m] =
          &(derived_quantities["lengthcomp_expected"]);
      info->variable_map[fleet_interface->lengthcomp_proportion.id_m] =
          &(derived_quantities["lengthcomp_proportion"]);
    }

    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }

#endif
};

#endif
