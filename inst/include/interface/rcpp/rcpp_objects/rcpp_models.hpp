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

#include <set>
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
 * @brief The CatchAtAgeInterface class is used to interface with the
 * CatchAtAge model. It inherits from the FisheryModelInterfaceBase class.
 */
class CatchAtAgeInterface : public FisheryModelInterfaceBase {
 public:
  /**
   * @brief The constructor.
   */
  CatchAtAgeInterface() : FisheryModelInterfaceBase() {
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
      : FisheryModelInterfaceBase(other) {}

  /**
   * Method to add a population id to the set of population ids.
   */
  void AddPopulation(uint32_t id) {
    this->population_ids->insert(id);

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
      ss << "}],\n";

      int ny = population_interface->n_years.get();
      int na = population_interface->n_ages.get();
      PopulationInterface *pi = population_interface_ptr.get();
      ss << " \"derived_quantities\": [\n";
      ss << dq_to_json_from_vv("total_landings_weight", "\"n_years\"", {ny}, pi->total_landings_weight) << ",\n";
      ss << dq_to_json_from_vv("total_landings_numbers", "\"n_years\"", {ny}, pi->total_landings_numbers) << ",\n";
      ss << dq_to_json_from_vv("mortality_F", "\"n_years\", \"n_ages\"", {ny, na}, pi->mortality_F) << ",\n";
      ss << dq_to_json_from_vv("mortality_M", "\"n_years\", \"n_ages\"", {ny, na}, pi->mortality_M) << ",\n";
      ss << dq_to_json_from_vv("mortality_Z", "\"n_years\", \"n_ages\"", {ny, na}, pi->mortality_Z) << ",\n";
      ss << dq_to_json_from_vv("numbers_at_age", "\"n_years+1\", \"n_ages\"", {ny + 1, na}, pi->numbers_at_age) << ",\n";
      ss << dq_to_json_from_vv("unfished_numbers_at_age", "\"n_years+1\", \"n_ages\"", {ny + 1, na}, pi->unfished_numbers_at_age) << ",\n";
      ss << dq_to_json_from_vv("biomass", "\"n_years+1\"", {ny + 1}, pi->biomass) << ",\n";
      ss << dq_to_json_from_vv("spawning_biomass", "\"n_years+1\"", {ny + 1}, pi->spawning_biomass) << ",\n";
      ss << dq_to_json_from_vv("unfished_biomass", "\"n_years+1\"", {ny + 1}, pi->unfished_biomass) << ",\n";
      ss << dq_to_json_from_vv("unfished_spawning_biomass", "\"n_years+1\"", {ny + 1}, pi->unfished_spawning_biomass) << ",\n";
      ss << dq_to_json_from_vv("proportion_mature_at_age", "\"n_years+1\", \"n_ages\"", {ny + 1, na}, pi->proportion_mature_at_age) << ",\n";
      ss << dq_to_json_from_vv("expected_recruitment", "\"n_years+1\"", {ny + 1}, pi->expected_recruitment) << ",\n";
      ss << dq_to_json_from_vv("sum_selectivity", "\"n_years\", \"n_ages\"", {ny, na}, pi->sum_selectivity) << "\n";
      ss << " ]}\n";
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
   * @brief Serialize a derived quantity VariableVector to a JSON DQ entry.
   *
   * @param name The name of the derived quantity.
   * @param dim_header Comma-separated quoted header strings (e.g. "\"n_years\"").
   * @param dims Sizes for each dimension.
   * @param vv The VariableVector holding the computed values (final_value_m).
   * @return std::string JSON fragment.
   */
  std::string dq_to_json_from_vv(const std::string &name,
                                  const std::string &dim_header,
                                  const std::vector<int> &dims,
                                  VariableVector &vv) {
    std::stringstream ss;
    ss << "{\n";
    ss << "\"name\":\"" << name << "\",\n";
    ss << "\"dimensionality\": {\n";
    ss << "  \"header\": [" << dim_header << "],\n";
    ss << "  \"dimensions\": [";
    for (size_t i = 0; i < dims.size(); i++) {
      if (i > 0) ss << ", ";
      ss << dims[i];
    }
    ss << "]\n},\n";
    ss << "\"value\":[";
    ss << std::fixed << std::setprecision(10);
    size_t n = vv.size();
    for (size_t i = 0; i < n; i++) {
      double v = vv[i].final_value_m;
      if (v != v) {  // NaN
        ss << "-999";
      } else {
        ss << v;
      }
      if (i < n - 1) ss << ", ";
    }
    ss << "]\n";
    ss << "}";
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

      int ny = fleet_interface->n_years.get();
      int na = fleet_interface->n_ages.get();
      int nl = fleet_interface->n_lengths.get();
      ss << "], \"derived_quantities\": [\n";
      ss << dq_to_json_from_vv("landings_numbers_at_age", "\"n_years\", \"n_ages\"", {ny, na}, fleet_interface->landings_numbers_at_age) << ",\n";
      ss << dq_to_json_from_vv("landings_weight_at_age", "\"n_years\", \"n_ages\"", {ny, na}, fleet_interface->landings_weight_at_age) << ",\n";
      ss << dq_to_json_from_vv("landings_numbers_at_length", "\"n_years\", \"n_lengths\"", {ny, nl}, fleet_interface->landings_numbers_at_length) << ",\n";
      ss << dq_to_json_from_vv("landings_weight", "\"n_years\"", {ny}, fleet_interface->landings_weight) << ",\n";
      ss << dq_to_json_from_vv("landings_numbers", "\"n_years\"", {ny}, fleet_interface->landings_numbers) << ",\n";
      ss << dq_to_json_from_vv("landings_expected", "\"n_years\"", {ny}, fleet_interface->landings_expected) << ",\n";
      ss << dq_to_json_from_vv("log_landings_expected", "\"n_years\"", {ny}, fleet_interface->log_landings_expected) << ",\n";
      ss << dq_to_json_from_vv("agecomp_proportion", "\"n_years\", \"n_ages\"", {ny, na}, fleet_interface->agecomp_proportion) << ",\n";
      ss << dq_to_json_from_vv("lengthcomp_proportion", "\"n_years\", \"n_lengths\"", {ny, nl}, fleet_interface->lengthcomp_proportion) << ",\n";
      ss << dq_to_json_from_vv("agecomp_expected", "\"n_years\", \"n_ages\"", {ny, na}, fleet_interface->agecomp_expected) << ",\n";
      ss << dq_to_json_from_vv("lengthcomp_expected", "\"n_years\", \"n_lengths\"", {ny, nl}, fleet_interface->lengthcomp_expected) << ",\n";
      ss << dq_to_json_from_vv("index_numbers_at_age", "\"n_years\", \"n_ages\"", {ny, na}, fleet_interface->index_numbers_at_age) << ",\n";
      ss << dq_to_json_from_vv("index_weight_at_age", "\"n_years\", \"n_ages\"", {ny, na}, fleet_interface->index_weight_at_age) << ",\n";
      ss << dq_to_json_from_vv("index_numbers_at_length", "\"n_years\", \"n_lengths\"", {ny, nl}, fleet_interface->index_numbers_at_length) << ",\n";
      ss << dq_to_json_from_vv("index_weight", "\"n_years\"", {ny}, fleet_interface->index_weight) << ",\n";
      ss << dq_to_json_from_vv("index_numbers", "\"n_years\"", {ny}, fleet_interface->index_numbers) << ",\n";
      ss << dq_to_json_from_vv("index_expected", "\"n_years\"", {ny}, fleet_interface->index_expected) << ",\n";
      ss << dq_to_json_from_vv("log_index_expected", "\"n_years\"", {ny}, fleet_interface->log_index_expected) << ",\n";
      ss << dq_to_json_from_vv("catch_index", "\"n_years\"", {ny}, fleet_interface->catch_index) << "\n";
      ss << "]}\n";
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

    // add to Information
    info->models_map[this->get_id()] = model;

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
