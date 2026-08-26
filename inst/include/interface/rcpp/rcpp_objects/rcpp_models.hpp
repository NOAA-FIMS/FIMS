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
 * @brief The fishery model types FIMS can build.
 *
 * @details The create_fishery_model_() function takes one of these names from
 * R and builds the matching class: "catch_at_age" builds a
 * CatchAtAgeInterface. FisheryModelInterfaceBase is never built on its own; it
 * only holds what all model types have in common.
 *
 * These are an enum rather than plain strings so that every place in the C++
 * code that acts on a model type has to name one of these values, which makes
 * it harder to add a type and forget to handle it somewhere.
 */
enum class FisheryModelType : uint8_t { catch_at_age = 0 };

/**
 * @brief Convert a type name supplied from R to a FisheryModelType.
 */
inline FisheryModelType FisheryModelTypeFromString(const std::string &name) {
  if (name == "catch_at_age") return FisheryModelType::catch_at_age;
  throw std::invalid_argument(
      "Invalid type: '" + name + "'. Valid options are: catch_at_age.");
}

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
   * @brief The populations this model operates on.
   *
   * @details The model holds direct shared_ptrs to
   * the linked populations. Every fleet in the model is reached by walking
   * these populations.
   */
  std::vector<std::shared_ptr<PopulationInterface>> populations_m;

  /**
   * @brief The constructor.
   */
  FisheryModelInterfaceBase() {
    this->id = FisheryModelInterfaceBase::id_g++;
    this->population_ids = std::make_shared<std::set<uint32_t>>();
  }

  /**
   * @brief Interface objects are not copyable.
   */
  FisheryModelInterfaceBase(const FisheryModelInterfaceBase &) = delete;
  FisheryModelInterfaceBase &operator=(const FisheryModelInterfaceBase &) = delete;

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
   * @brief Set the populations this model operates on, replacing whatever was
   * there before.
   *
   * @details Records each population's unique ID and a direct link to the
   * interface object, rebuilding both together. population_ids is a std::set
   * and populations_m a std::vector, so appending to them separately could
   * de-duplicate in one and duplicate in the other; rebuilding both from the
   * same loop keeps them in step. A duplicate would otherwise be serialized
   * twice by to_json() and have its derived quantities initialized twice by
   * add_to_fims_tmb_internal().
   *
   * @param populations The population interface objects, in the order they
   * should be recorded.
   */
  void SetPopulations(
      const std::vector<std::shared_ptr<PopulationInterface>> &populations) {
    this->population_ids->clear();
    this->populations_m.clear();
    for (size_t i = 0; i < populations.size(); i++) {
      if (!populations[i]) {
        FIMS_ERROR_LOG("Cannot add a null population to model " +
                       fims::to_string(this->id) + ".");
        continue;
      }
      if (!this->population_ids->insert(populations[i]->get_id()).second) {
        FIMS_WARNING_LOG("Population " +
                         fims::to_string(populations[i]->get_id()) +
                         " appears more than once in the populations given to "
                         "model " + fims::to_string(this->id) +
                         "; ignoring the repeat.");
        continue;
      }
      this->populations_m.push_back(populations[i]);
    }
  }

  /**
   * @brief Collect the distinct fleets across every linked population, keyed
   * by fleet ID so a fleet operating on several populations is returned once.
   */
  std::map<uint32_t, std::shared_ptr<FleetInterface>> GetFleets() const {
    std::map<uint32_t, std::shared_ptr<FleetInterface>> fleets;
    for (size_t i = 0; i < this->populations_m.size(); i++) {
      const std::vector<std::shared_ptr<FleetInterface>> &population_fleets =
          this->populations_m[i]->fleets_m;
      for (size_t j = 0; j < population_fleets.size(); j++) {
        if (population_fleets[j]) {
          fleets[population_fleets[j]->get_id()] = population_fleets[j];
        }
      }
    }
    return fleets;
  }

  /**
   * @brief Enable or disable reporting for this model.
   *
   * @details Defaults to a no-op; model types that support reporting override
   * it.
   */
  virtual void DoReporting(bool report) {}

  /**
   * @brief Whether reporting is enabled for this model.
   */
  virtual bool IsReporting() { return false; }

  /**
   * @brief Serialize the assembled model to a JSON string.
   *
   * @details Overridden by each model type. See
   * CatchAtAgeInterface::to_json for what report_modules carries.
   */
  virtual std::string to_json(const std::vector<SharedBase> &report_modules) {
    return "std::string to_json(report_modules) not yet implemented.";
  }

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
  CatchAtAgeInterface() : FisheryModelInterfaceBase() {}

  /**
   * @brief Interface objects are not copyable.
   */
  CatchAtAgeInterface(const CatchAtAgeInterface &) = delete;
  CatchAtAgeInterface &operator=(const CatchAtAgeInterface &) = delete;

  /**
   * @brief Enable or disable reporting for the CatchAtAge model.
   *
   * @details This method is used to control whether reporting is performed for
   * the CatchAtAge model. The implementation may depend on TMB_MODEL.
   * @param report Boolean flag to enable (true) or disable (false) reporting.
   */
  virtual void DoReporting(bool report) {
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
  virtual bool IsReporting() {
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
  std::string population_to_json(
      std::shared_ptr<PopulationInterface> population_interface) {
    std::stringstream ss;

    if (!population_interface) {
      FIMS_ERROR_LOG("Population pointer is null; cannot convert to JSON.");
      return "{}";  // Return empty JSON
    }

    std::shared_ptr<PopulationInterface> population_interface_ptr =
        population_interface;

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
      ss << "  \"dimensions\": [" << population_interface->n_years << ", "
         << population_interface->n_ages << "]\n},\n";
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
      ss << "  \"dimensions\": [" << population_interface->n_years
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
      ss << "  \"dimensions\": [" << (population_interface->n_years + 1)
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
      ss << "  \"dimensions\": [" << population_interface->n_ages
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
  std::string fleet_to_json(std::shared_ptr<FleetInterface> fleet_interface) {
    std::stringstream ss;

    if (!fleet_interface) {
      FIMS_ERROR_LOG("Fleet pointer is null; cannot convert to JSON.");
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
      ss << " \"n_ages\": " << fleet_interface->n_ages << ",\n";
      ss << " \"n_years\": " << fleet_interface->n_years << ",\n";
      ss << " \"n_lengths\": " << fleet_interface->n_lengths << ",\n";
      ss << "\"data_ids\" : [\n";
      ss << "{\"agecomp\": " << fleet_interface->GetObservedAgeCompDataID()
         << "},\n";
      ss << "{\"lengthcomp\": "
         << fleet_interface->GetObservedLengthCompDataID() << "},\n";
      ss << "{\"index\": " << fleet_interface->GetObservedIndexDataID()
         << "},\n";
      ss << "{\"catch\": " << fleet_interface->GetObservedCatchDataID()
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
      ss << "  \"dimensions\": [" << fleet_interface->n_years
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
   * @brief If a module is of type T, store it in that type's map under its
   * module id.
   *
   * @param module The module to test, as a base-class pointer.
   * @param modules The map to store it in if the type matches.
   * @return true if the module was of type T and has been stored; false if it
   * is some other type and should be offered to the next map.
   */
  template <typename T>
  static bool try_store_module(
      const SharedBase &module,
      std::map<uint32_t, std::shared_ptr<T>> &modules) {
    if (std::shared_ptr<T> typed_module =
            std::dynamic_pointer_cast<T>(module)) {
      modules[typed_module->get_id()] = typed_module;
      return true;
    }
    return false;
  }

  // Keep the no-argument FisheryModelInterfaceBase::to_json() visible
  // alongside the overload below rather than hiding it.
  using FisheryModelInterfaceBase::to_json;

  /**
   * @brief Serialize the assembled model to a JSON string.
   *
   * @details The model reaches its populations and their fleets through direct
   * links, because add_to_fims_tmb_internal() needs those objects to size its
   * derived-quantity maps. Every other module -- growth, recruitment,
   * maturity, selectivity, the observed data modules, and the density
   * components -- is referenced from the model only by integer id and exists
   * purely to be reported. Rather than have the population and fleet
   * interfaces carry pointers that serve no purpose outside this function,
   * the reporting path is handed those modules directly.
   *
   * @param report_modules The modules to serialize, as base-class pointers.
   * They are sorted by type and keyed by module id, so ordering is
   * deterministic and a module used by several populations or fleets is
   * emitted once.
   * @return A JSON string representing the current state of the model.
   */
  virtual std::string to_json(const std::vector<SharedBase> &report_modules) {
    std::map<uint32_t, std::shared_ptr<RecruitmentInterfaceBase>>
        recruitment_modules;
    std::map<uint32_t, std::shared_ptr<GrowthInterfaceBase>> growth_modules;
    std::map<uint32_t, std::shared_ptr<MaturityInterfaceBase>>
        maturity_modules;
    std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>
        selectivity_modules;
    std::map<uint32_t, std::shared_ptr<DataInterfaceBase>> data_modules;
    std::map<uint32_t, std::shared_ptr<DistributionsInterfaceBase>>
        density_components;

    // Sort each supplied module into the output section it belongs to. The
    // types are disjoint, so the order of these tests does not matter.
    // Populations, fleets, and models match none of them and fall through:
    // the model reaches those through its own links.
    for (size_t i = 0; i < report_modules.size(); i++) {
      const SharedBase &module = report_modules[i];
      if (!module) continue;
      if (try_store_module(module, recruitment_modules)) continue;
      if (try_store_module(module, growth_modules)) continue;
      if (try_store_module(module, maturity_modules)) continue;
      if (try_store_module(module, selectivity_modules)) continue;
      if (try_store_module(module, data_modules)) continue;
      if (try_store_module(module, density_components)) continue;
    }

    // Fleets are reached through the populations, de-duplicated by fleet id so
    // a fleet operating on several populations is emitted once.
    std::map<uint32_t, std::shared_ptr<FleetInterface>> fleets =
        this->GetFleets();
    typename std::map<uint32_t, std::shared_ptr<FleetInterface>>::iterator
        fleet_map_it;

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
    typename std::map<uint32_t, std::shared_ptr<GrowthInterfaceBase>>::iterator
        growth_it;
    for (growth_it = growth_modules.begin(); growth_it != growth_modules.end();
         growth_it++) {
      std::shared_ptr<GrowthInterfaceBase> growth_interface = growth_it->second;
      if (growth_interface) {
        growth_interface->finalize();
        ss << growth_interface->to_json();
        if (std::next(growth_it) != growth_modules.end()) {
          ss << ", ";
        }
      }
    }

    ss << "],\n";

    ss << "\"recruitment\": [\n";
    typename std::map<uint32_t, std::shared_ptr<RecruitmentInterfaceBase>>::iterator
        recruitment_it;
    for (recruitment_it = recruitment_modules.begin(); recruitment_it != recruitment_modules.end();
         recruitment_it++) {
      std::shared_ptr<RecruitmentInterfaceBase> recruitment_interface = recruitment_it->second;
      if (recruitment_interface) {
        recruitment_interface->finalize();
        ss << recruitment_interface->to_json();
        if (std::next(recruitment_it) != recruitment_modules.end()) {
          ss << ", ";
        }
      }
    }
    ss << "],\n";

    ss << "\"maturity\": [\n";
    typename std::map<uint32_t, std::shared_ptr<MaturityInterfaceBase>>::iterator
        maturity_it;
    for (maturity_it = maturity_modules.begin(); maturity_it != maturity_modules.end();
         maturity_it++) {
      std::shared_ptr<MaturityInterfaceBase> maturity_interface = maturity_it->second;
      if (maturity_interface) {
        maturity_interface->finalize();
        ss << maturity_interface->to_json();
        if (std::next(maturity_it) != maturity_modules.end()) {
          ss << ", ";
        }
      }
    }
    ss << "],\n";

    ss << "\"selectivity\": [\n";
    typename std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>::iterator
        selectivity_it;
    for (selectivity_it = selectivity_modules.begin(); selectivity_it != selectivity_modules.end();
         selectivity_it++) {
      std::shared_ptr<SelectivityInterfaceBase> selectivity_interface = selectivity_it->second;
      if (selectivity_interface) {
        selectivity_interface->finalize();
        ss << selectivity_interface->to_json();
        if (std::next(selectivity_it) != selectivity_modules.end()) {
          ss << ", ";
        }
      }
    }
    ss << "],\n";

    ss << " \"population_ids\": [";
    typename std::set<uint32_t>::iterator pit;
    for (pit = this->population_ids->begin();
         pit != this->population_ids->end(); pit++) {
      ss << *pit;
      if (std::next(pit) != this->population_ids->end()) {
        ss << ", ";
      }
    }
    ss << "],\n";
    ss << " \"fleet_ids\": [";

    for (fleet_map_it = fleets.begin(); fleet_map_it != fleets.end();
         fleet_map_it++) {
      ss << fleet_map_it->first;
      if (std::next(fleet_map_it) != fleets.end()) {
        ss << ", ";
      }
    }
    ss << "],\n";
    ss << "\"populations\": [\n";
    for (size_t i = 0; i < this->populations_m.size(); i++) {
      std::shared_ptr<PopulationInterface> population_interface =
          this->populations_m[i];
      if (population_interface) {
        population_interface->finalize();
        ss << this->population_to_json(population_interface);
      } else {
        FIMS_ERROR_LOG("Population " + fims::to_string(i) +
                       " linked to CatchAtAge " + fims::to_string(this->id) +
                       " is null.");
        ss << "{}";  // Return empty JSON for this population
      }
      if (i + 1 < this->populations_m.size()) {
        ss << ",";
      }
    }

    ss << "]";
    ss << ",\n";
    ss << "\"fleets\": [\n";

    for (fleet_map_it = fleets.begin(); fleet_map_it != fleets.end();
         fleet_map_it++) {
      std::shared_ptr<FleetInterface> fleet_interface = fleet_map_it->second;
      fleet_interface->finalize();
      ss << this->fleet_to_json(fleet_interface);
      if (std::next(fleet_map_it) != fleets.end()) {
        ss << ",";
      }
    }

    ss << "],\n";

    ss << "\"density_components\": [\n";

    typename std::map<uint32_t,
                      std::shared_ptr<DistributionsInterfaceBase>>::iterator
        density_it;
    for (density_it = density_components.begin();
         density_it != density_components.end(); ++density_it) {
      std::shared_ptr<DistributionsInterfaceBase> dist_interface =
          density_it->second;
      if (dist_interface) {
        dist_interface->finalize();
        ss << dist_interface->to_json();
        if (std::next(density_it) != density_components.end()) {
          ss << ",\n";
        }
      }
    }
    ss << "\n],\n";
    ss << "\"data\": [\n";
    typename std::map<uint32_t, std::shared_ptr<DataInterfaceBase>>::iterator
        d_it;
    for (d_it = data_modules.begin(); d_it != data_modules.end(); ++d_it) {
      std::shared_ptr<DataInterfaceBase> data_interface = (*d_it).second;
      if (data_interface) {
        data_interface->finalize();
        ss << data_interface->to_json();
        if (std::next(d_it) != data_modules.end()) {
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

    // all fleets in the model, keyed and therefore ordered by fleet ID
    std::map<uint32_t, std::shared_ptr<FleetInterface>> fleets =
        this->GetFleets();

    // add to Information
    info->models_map[this->get_id()] = model;

    for (size_t population_index = 0;
         population_index < this->populations_m.size(); population_index++) {
      std::shared_ptr<PopulationInterface> population_interface =
          this->populations_m[population_index];
      if (!population_interface) {
        FIMS_ERROR_LOG("Population " + fims::to_string(population_index) +
                       " linked to CatchAtAge " + fims::to_string(this->id) +
                       " is null.");
        continue;
      }

      model->InitializePopulationDerivedQuantities(population_interface->id);
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->GetPopulationDerivedQuantities(population_interface->id);

      std::map<std::string, fims_popdy::DimensionInfo>
          &derived_quantities_dim_info =
              model->GetPopulationDimensionInfo(population_interface->id);

      std::stringstream ss;

      derived_quantities["total_catch_weight"] =
          fims::Vector<Type>(population_interface->n_years);

      derived_quantities_dim_info["total_catch_weight"] =
          fims_popdy::DimensionInfo(
              "total_catch_weight",
              fims::Vector<int>{(int)population_interface->n_years},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[population_interface->total_catch_weight.id_m] =
          &derived_quantities["total_catch_weight"];

      derived_quantities["total_catch_numbers"] =
          fims::Vector<Type>(population_interface->n_years);

      derived_quantities_dim_info["total_catch_numbers"] =
          fims_popdy::DimensionInfo(
              "total_catch_numbers",
              fims::Vector<int>{population_interface->n_years},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[population_interface->total_catch_numbers.id_m] =
          &derived_quantities["total_catch_numbers"];

      derived_quantities["mortality_F"] =
          fims::Vector<Type>(population_interface->n_years *
                             population_interface->n_ages);
      derived_quantities_dim_info["mortality_F"] = fims_popdy::DimensionInfo(
          "mortality_F",
          fims::Vector<int>{population_interface->n_years,
                            population_interface->n_ages},
          fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[population_interface->mortality_F.id_m] =
          &derived_quantities["mortality_F"];

      derived_quantities["mortality_M"] =
          fims::Vector<Type>(population_interface->n_years *
                             population_interface->n_ages);
      derived_quantities_dim_info["mortality_M"] = fims_popdy::DimensionInfo(
          "mortality_M",
          fims::Vector<int>{population_interface->n_years,
                            population_interface->n_ages},
          fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[population_interface->mortality_M.id_m] =
          &derived_quantities["mortality_M"];

      derived_quantities["mortality_Z"] =
          fims::Vector<Type>(population_interface->n_years *
                             population_interface->n_ages);
      derived_quantities_dim_info["mortality_Z"] = fims_popdy::DimensionInfo(
          "mortality_Z",
          fims::Vector<int>{population_interface->n_years,
                            population_interface->n_ages},
          fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[population_interface->mortality_Z.id_m] =
          &derived_quantities["mortality_Z"];

      derived_quantities["numbers_at_age"] =
          fims::Vector<Type>((population_interface->n_years + 1) *
                             population_interface->n_ages);
      derived_quantities_dim_info["numbers_at_age"] = fims_popdy::DimensionInfo(
          "numbers_at_age",
          fims::Vector<int>{(population_interface->n_years + 1),
                            population_interface->n_ages},
          fims::Vector<std::string>{"n_years+1", "n_ages"});
      info->variable_map[population_interface->numbers_at_age.id_m] =
          &derived_quantities["numbers_at_age"];

      derived_quantities["unfished_numbers_at_age"] =
          fims::Vector<Type>((population_interface->n_years + 1) *
                             population_interface->n_ages);
      derived_quantities_dim_info["unfished_numbers_at_age"] =
          fims_popdy::DimensionInfo(
              "unfished_numbers_at_age",
              fims::Vector<int>{(population_interface->n_years + 1),
                                population_interface->n_ages},
              fims::Vector<std::string>{"n_years+1", "n_ages"});
      info->variable_map[population_interface->unfished_numbers_at_age.id_m] =
          &derived_quantities["unfished_numbers_at_age"];

      derived_quantities["biomass"] =
          fims::Vector<Type>((population_interface->n_years + 1));
      derived_quantities_dim_info["biomass"] = fims_popdy::DimensionInfo(
          "biomass",
          fims::Vector<int>{(population_interface->n_years + 1)},
          fims::Vector<std::string>{"n_years+1"});
      info->variable_map[population_interface->biomass.id_m] =
          &derived_quantities["biomass"];

      derived_quantities["spawning_biomass"] =
          fims::Vector<Type>((population_interface->n_years + 1));
      derived_quantities_dim_info["spawning_biomass"] =
          fims_popdy::DimensionInfo(
              "spawning_biomass",
              fims::Vector<int>{(population_interface->n_years + 1)},
              fims::Vector<std::string>{"n_years+1"});
      info->variable_map[population_interface->spawning_biomass.id_m] =
          &derived_quantities["spawning_biomass"];

      derived_quantities["unfished_biomass"] =
          fims::Vector<Type>((population_interface->n_years + 1));
      derived_quantities_dim_info["unfished_biomass"] =
          fims_popdy::DimensionInfo(
              "unfished_biomass",
              fims::Vector<int>{(population_interface->n_years + 1)},
              fims::Vector<std::string>{"n_years+1"});
      info->variable_map[population_interface->unfished_biomass.id_m] =
          &derived_quantities["unfished_biomass"];

      derived_quantities["unfished_spawning_biomass"] =
          fims::Vector<Type>((population_interface->n_years + 1));
      derived_quantities_dim_info["unfished_spawning_biomass"] =
          fims_popdy::DimensionInfo(
              "unfished_spawning_biomass",
              fims::Vector<int>{(population_interface->n_years + 1)},
              fims::Vector<std::string>{"n_years+1"});
      info->variable_map[population_interface->unfished_spawning_biomass.id_m] =
          &derived_quantities["unfished_spawning_biomass"];

      derived_quantities["proportion_mature_at_age"] =
          fims::Vector<Type>((population_interface->n_years + 1) *
                             population_interface->n_ages);
      derived_quantities_dim_info["proportion_mature_at_age"] =
          fims_popdy::DimensionInfo(
              "proportion_mature_at_age",
              fims::Vector<int>{(population_interface->n_years + 1),
                                population_interface->n_ages},
              fims::Vector<std::string>{"n_years+1", "n_ages"});
      info->variable_map[population_interface->proportion_mature_at_age.id_m] =
          &derived_quantities["proportion_mature_at_age"];

      derived_quantities["expected_recruitment"] =
          fims::Vector<Type>((population_interface->n_years + 1));
      derived_quantities_dim_info["expected_recruitment"] =
          fims_popdy::DimensionInfo(
              "expected_recruitment",
              fims::Vector<int>{(population_interface->n_years + 1)},
              fims::Vector<std::string>{"n_years+1"});
      info->variable_map[population_interface->expected_recruitment.id_m] =
          &derived_quantities["expected_recruitment"];

      derived_quantities["sum_selectivity"] =
          fims::Vector<Type>(population_interface->n_years *
                             population_interface->n_ages);
      derived_quantities_dim_info["sum_selectivity"] =
          fims_popdy::DimensionInfo(
              "sum_selectivity",
              fims::Vector<int>{population_interface->n_years,
                                population_interface->n_ages},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[population_interface->sum_selectivity.id_m] =
          &derived_quantities["sum_selectivity"];

      // replace elements in the variable map
    }

    typename std::map<uint32_t, std::shared_ptr<FleetInterface>>::iterator
        fleet_map_it;
    for (fleet_map_it = fleets.begin(); fleet_map_it != fleets.end();
         ++fleet_map_it) {
      std::shared_ptr<FleetInterface> fleet_interface = fleet_map_it->second;
      model->InitializeFleetDerivedQuantities(fleet_interface->id);
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->GetFleetDerivedQuantities(fleet_interface->id);

      std::map<std::string, fims_popdy::DimensionInfo>
          &derived_quantities_dim_info =
              model->GetFleetDimensionInfo(fleet_interface->id);

      // initialize derive quantities
      // catch
      derived_quantities["catch_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_ages);
      derived_quantities_dim_info["catch_numbers_at_age"] =
          fims_popdy::DimensionInfo(
              "catch_numbers_at_age",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_ages},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[fleet_interface->catch_numbers_at_age.id_m] =
          &derived_quantities["catch_numbers_at_age"];

      derived_quantities["catch_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_ages);
      derived_quantities_dim_info["catch_weight_at_age"] =
          fims_popdy::DimensionInfo(
              "catch_weight_at_age",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_ages},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[fleet_interface->catch_weight_at_age.id_m] =
          &derived_quantities["catch_weight_at_age"];

      derived_quantities["catch_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_lengths);
      derived_quantities_dim_info["catch_numbers_at_length"] =
          fims_popdy::DimensionInfo(
              "catch_numbers_at_length",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_lengths},
              fims::Vector<std::string>{"n_years", "n_lengths"});
      info->variable_map[fleet_interface->catch_numbers_at_length.id_m] =
          &derived_quantities["catch_numbers_at_length"];

      derived_quantities["catch_weight"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["catch_weight"] =
          fims_popdy::DimensionInfo(
              "catch_weight",
              fims::Vector<int>{(fleet_interface->n_years)},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->catch_weight.id_m] =
          &derived_quantities["catch_weight"];

      derived_quantities["catch_numbers"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["catch_numbers"] =
          fims_popdy::DimensionInfo(
              "catch_numbers",
              fims::Vector<int>{(fleet_interface->n_years)},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->catch_numbers.id_m] =
          &derived_quantities["catch_numbers"];

      derived_quantities["catch_expected"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["catch_expected"] =
          fims_popdy::DimensionInfo(
              "catch_expected",
              fims::Vector<int>{(fleet_interface->n_years)},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->catch_expected.id_m] =
          &derived_quantities["catch_expected"];

      derived_quantities["log_catch_expected"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["log_catch_expected"] =
          fims_popdy::DimensionInfo(
              "log_catch_expected",
              fims::Vector<int>{(fleet_interface->n_years)},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->log_catch_expected.id_m] =
          &derived_quantities["log_catch_expected"];

      derived_quantities["agecomp_proportion"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_ages);
      derived_quantities_dim_info["agecomp_proportion"] =
          fims_popdy::DimensionInfo(
              "agecomp_proportion",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_ages},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[fleet_interface->agecomp_proportion.id_m] =
          &derived_quantities["agecomp_proportion"];

      derived_quantities["lengthcomp_proportion"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_lengths);
      derived_quantities_dim_info["lengthcomp_proportion"] =
          fims_popdy::DimensionInfo(
              "lengthcomp_proportion",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_lengths},
              fims::Vector<std::string>{"n_years", "n_lengths"});
      info->variable_map[fleet_interface->lengthcomp_proportion.id_m] =
          &derived_quantities["lengthcomp_proportion"];

      // index
      derived_quantities["index_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_ages);
      derived_quantities_dim_info["index_numbers_at_age"] =
          fims_popdy::DimensionInfo(
              "index_numbers_at_age",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_ages},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[fleet_interface->index_numbers_at_age.id_m] =
          &derived_quantities["index_numbers_at_age"];

      derived_quantities["index_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_ages);
      derived_quantities_dim_info["index_weight_at_age"] =
          fims_popdy::DimensionInfo(
              "index_weight_at_age",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_ages},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[fleet_interface->index_weight_at_age.id_m] =
          &derived_quantities["index_weight_at_age"];

      derived_quantities["index_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_lengths);
      derived_quantities_dim_info["index_numbers_at_length"] =
          fims_popdy::DimensionInfo(
              "index_numbers_at_length",
              fims::Vector<int>{(fleet_interface->n_years),
                                fleet_interface->n_lengths},
              fims::Vector<std::string>{"n_years", "n_lengths"});
      info->variable_map[fleet_interface->index_numbers_at_length.id_m] =
          &derived_quantities["index_numbers_at_length"];

      derived_quantities["index_weight"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["index_weight"] = fims_popdy::DimensionInfo(
          "index_weight", fims::Vector<int>{(fleet_interface->n_years)},
          fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->index_weight.id_m] =
          &derived_quantities["index_weight"];

      derived_quantities["index_numbers"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["index_numbers"] = fims_popdy::DimensionInfo(
          "index_numbers", fims::Vector<int>{(fleet_interface->n_years)},
          fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->index_numbers.id_m] =
          &derived_quantities["index_numbers"];

      derived_quantities["index_expected"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["index_expected"] = fims_popdy::DimensionInfo(
          "index_expected", fims::Vector<int>{(fleet_interface->n_years)},
          fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->index_expected.id_m] =
          &derived_quantities["index_expected"];

      derived_quantities["log_index_expected"] =
          fims::Vector<Type>(fleet_interface->n_years);
      derived_quantities_dim_info["log_index_expected"] =
          fims_popdy::DimensionInfo(
              "log_index_expected",
              fims::Vector<int>{(fleet_interface->n_years)},
              fims::Vector<std::string>{"n_years"});
      info->variable_map[fleet_interface->log_index_expected.id_m] =
          &derived_quantities["log_index_expected"];

      derived_quantities["agecomp_expected"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_ages);
      derived_quantities_dim_info["agecomp_expected"] =
          fims_popdy::DimensionInfo(
              "agecomp_expected",
              fims::Vector<int>{(fleet_interface->n_years),
                                (fleet_interface->n_ages)},
              fims::Vector<std::string>{"n_years", "n_ages"});
      info->variable_map[fleet_interface->agecomp_expected.id_m] =
          &derived_quantities["agecomp_expected"];

      derived_quantities["lengthcomp_expected"] = fims::Vector<Type>(
          fleet_interface->n_years * fleet_interface->n_lengths);
      derived_quantities_dim_info["lengthcomp_expected"] =
          fims_popdy::DimensionInfo(
              "lengthcomp_expected",
              fims::Vector<int>{(fleet_interface->n_years),
                                (fleet_interface->n_lengths)},
              fims::Vector<std::string>{"n_years", "n_lengths"});
      info->variable_map[fleet_interface->lengthcomp_expected.id_m] =
          &derived_quantities["lengthcomp_expected"];
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
