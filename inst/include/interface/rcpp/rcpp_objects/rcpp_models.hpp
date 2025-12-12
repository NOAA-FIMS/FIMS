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
#include "../../../common/def.hpp"
#include "../../../models/fisheries_models.hpp"
#include "../../../utilities/fims_json.hpp"
#include "rcpp_population.hpp"

#include "rcpp_interface_base.hpp"
#include "rcpp_population.hpp"
#include "rcpp_fleet.hpp"
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
      : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~FisheryModelInterfaceBase() {}

  virtual std::string to_json() {
    return "std::string to_json() not yet implemented.";
  }

  /**
   * @brief A function to calculate reference points for the fishery model.
   *
   * @return Rcpp::List
   */
  virtual Rcpp::List calculate_reference_points() {
    Rcpp::List result;
    return result;
  }

  /**
   * @brief Get the ID for the child fleet interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;
};
// static id of the FleetInterfaceBase object
uint32_t FisheryModelInterfaceBase::id_g = 1;

// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>>
    FisheryModelInterfaceBase::live_objects;

/**
 * @brief The CatchAtAgeInterface class is used to interface with the
 * CatchAtAge model. It inherits from the FisheryModelInterfaceBase class.
 */
class CatchAtAgeInterface : public FisheryModelInterfaceBase {
  /**
   * @brief The set of population ids that this catch at age model operates on.
   */
  std::shared_ptr<std::set<uint32_t>> population_ids;
  /**
   * @brief Iterator for population ids.
   */
  typedef typename std::set<uint32_t>::iterator population_id_iterator;

  /**
   * @brief A private working map of standard error values for all
   * concatenated derived quantities. Elements are extracted in the
   * to_json method.
   */
  std::map<std::string, std::vector<double>> se_values;

 public:
  /**
   * @brief The constructor.
   */
  CatchAtAgeInterface() : FisheryModelInterfaceBase() {
    this->population_ids = std::make_shared<std::set<uint32_t>>();
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
        population_ids(other.population_ids) {}

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
      fims::Vector<double> log_M_uncertainty(pop->log_M.size(), -999);
      this->get_se_values("log_M", this->se_values, log_M_uncertainty);
      for (size_t i = 0; i < pop->log_M.size(); i++) {
        population_interface_ptr->log_M[i].final_value_m = pop->log_M[i];
        population_interface_ptr->log_M[i].uncertainty_m = log_M_uncertainty[i];
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

      fims::Vector<double> log_init_naa_uncertainty(pop->log_init_naa.size(),
                                                    -999);
      this->get_se_values("log_init_naa", this->se_values,
                          log_init_naa_uncertainty);
      for (size_t i = 0; i < pop->log_init_naa.size(); i++) {
        population_interface_ptr->log_init_naa[i].final_value_m =
            pop->log_init_naa[i];
        population_interface_ptr->log_init_naa[i].uncertainty_m =
            log_init_naa_uncertainty[i];
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
    std::string name = (*it).first;
    fims::Vector<double> &dq = (*it).second;
    std::stringstream dim_entry;
    bool has_se = false;
    typename std::map<std::string, std::vector<double>>::iterator se_vals =
        this->se_values.find(name);

    if (se_vals != this->se_values.end()) {
      has_se = true;
    }
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
        ss << "-999]" << ",\n";
      } else {
        ss << dq[dq.size() - 1] << "],\n";
      }
    } else {
      ss << "],\n";
    }
    if (has_se) {
      try {
        std::vector<double> &se_vals_vector = (*se_vals).second;
        if (se_vals_vector.size() < dq.size()) {
          throw std::runtime_error(
              "Standard error vector size is smaller than derived quantity "
              "size for derived quantity " +
              name);
        }
        std::vector<double> uncertainty_std(se_vals_vector.begin(),
                                            se_vals_vector.begin() + dq.size());
        std::vector<double> temp(se_vals_vector.begin() + dq.size(),
                                 se_vals_vector.end());
        se_vals_vector = temp;
        fims::Vector<double> uncertainty(uncertainty_std);
        ss << "\"uncertainty\": " << uncertainty << "\n";
      } catch (const std::exception &e) {
        throw std::runtime_error(
            "Error processing uncertainty for derived quantity " + name + ": " +
            e.what());
      }
    } else {
      ss << "\"uncertainty\": [";
      for (size_t i = 0; i < dq.size(); ++i) {
        ss << "-999.0";  // Placeholder for uncertainty values
        if (i < dq.size() - 1) {
          ss << ", ";
        }
      }
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
      FIMS_ERROR_LOG("Fleet with id " +
                     fims::to_string(fleet_interface->get_id()) +
                     " not found in live objects.");
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
      fims::Vector<double> log_Fmort_uncertainty(fleet->log_Fmort.size(), -999);
      this->get_se_values("log_Fmort", this->se_values, log_Fmort_uncertainty);
      for (size_t i = 0; i < fleet_interface->log_Fmort.size(); i++) {
        fleet_interface->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
        fleet_interface->log_Fmort[i].uncertainty_m = log_Fmort_uncertainty[i];
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
      fims::Vector<double> log_q_uncertainty(fleet->log_q.size(), -999);
      this->get_se_values("log_q", this->se_values, log_q_uncertainty);
      for (size_t i = 0; i < fleet->log_q.size(); i++) {
        fleet_interface->log_q[i].final_value_m = fleet->log_q[i];
        fleet_interface->log_q[i].uncertainty_m = log_q_uncertainty[i];
      }
      ss << " \"name\": \"log_q\",\n";
      ss << " \"id\":" << fleet_interface->log_q.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"dimensionality\": {\n";
      ss << "  \"header\": [\"" << "na" << "\"],\n";
      ss << "  \"dimensions\": [" << fleet->log_q.size() << "]\n},\n";

      ss << " \"values\": " << fleet_interface->log_q << "}\n";

      if (fleet_interface->n_lengths > 0) {
        ss << " ,{\n";
        fims::Vector<double> age_to_length_conversion_uncertainty(
            fleet->age_to_length_conversion.size(), -999);
        this->get_se_values("age_to_length_conversion", this->se_values,
                            age_to_length_conversion_uncertainty);
        for (size_t i = 0; i < fleet_interface->age_to_length_conversion.size();
             i++) {
          fleet_interface->age_to_length_conversion[i].final_value_m =
              fleet->age_to_length_conversion[i];
          fleet_interface->age_to_length_conversion[i].uncertainty_m =
              age_to_length_conversion_uncertainty[i];
        }
        ss << " \"name\": \"age_to_length_conversion\",\n";
        ss << " \"id\":" << fleet_interface->age_to_length_conversion.id_m
           << ",\n";
        ss << " \"type\": \"vector\",\n";
        ss << " \"dimensionality\": {\n";
        ss << "  \"header\": [" << "\"n_ages\", \"n_lengths\"" << "],\n";
        ss << "  \"dimensions\": [" << fleet_interface->n_ages.get() << ", "
           << fleet_interface->n_lengths.get() << "]\n},\n";

        ss << " \"values\": " << fleet_interface->age_to_length_conversion
           << "\n";

        ss << "\n}\n";
      }

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
   * @brief Get the vector of fixed effect parameters for the CatchAtAge model.
   *
   * @details Returns a numeric vector containing the fixed effect parameters
   * used in the model.
   * @return Rcpp::NumericVector of fixed effect parameters.
   */
  Rcpp::NumericVector get_fixed_parameters_vector() {
    // base model
    std::shared_ptr<fims_info::Information<double>> info0 =
        fims_info::Information<double>::GetInstance();

    Rcpp::NumericVector p;

    for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++) {
      p.push_back(*info0->fixed_effects_parameters[i]);
    }

    return p;
  }

  /**
   * @brief Get the vector of random effect parameters for the CatchAtAge model.
   *
   * @details Returns a numeric vector containing the random effect parameters
   * used in the model.
   * @return Rcpp::NumericVector of random effect parameters.
   */
  Rcpp::NumericVector get_random_parameters_vector() {
    // base model
    std::shared_ptr<fims_info::Information<double>> d0 =
        fims_info::Information<double>::GetInstance();

    Rcpp::NumericVector p;

    for (size_t i = 0; i < d0->random_effects_parameters.size(); i++) {
      p.push_back(*d0->random_effects_parameters[i]);
    }

    return p;
  }

  /**
   * @brief Get the report output for the CatchAtAge model.
   *
   * @details Returns a list containing the report results for the CatchAtAge
   * model, including derived quantities and diagnostics.
   * @return Rcpp::List containing the report output.
   */
  Rcpp::List get_report() {
    Rcpp::Environment base = Rcpp::Environment::base_env();
    Rcpp::Function summary = base["summary"];

    // Grab needed R functions
    Rcpp::Environment TMB = Rcpp::Environment::namespace_env("TMB");
    Rcpp::Function MakeADFun = TMB["MakeADFun"];
    Rcpp::Function sdreport = TMB["sdreport"];
    // Grab your helpers from R global environment
    Rcpp::Environment global = Rcpp::Environment::global_env();
    // Build parameters list
    Rcpp::List parameters = Rcpp::List::create(
        Rcpp::Named("p") = this->get_fixed_parameters_vector(),
        Rcpp::Named("re") = this->get_random_parameters_vector());
    // Call MakeADFun with map = NULL
    Rcpp::List obj = MakeADFun(
        Rcpp::Named("data") = Rcpp::List::create(),
        Rcpp::Named("parameters") = parameters, Rcpp::Named("DLL") = "FIMS",
        Rcpp::Named("silent") = true, Rcpp::Named("map") = R_NilValue,
        Rcpp::Named("random") = "re");
    // Call obj$report()
    Rcpp::Function report = obj["report"];
    Rcpp::Function func = obj["fn"];
    Rcpp::Function gradient = obj["gr"];
    Rcpp::NumericVector grad = gradient(this->get_fixed_parameters_vector());
    double maxgc = -999;
    for (R_xlen_t i = 0; i < grad.size(); i++) {
      if (std::fabs(grad[i]) > maxgc) {
        maxgc = std::fabs(grad[i]);
      }
    }
    double of_value =
        Rcpp::as<double>(func(this->get_fixed_parameters_vector()));
    Rcpp::List rep = report();
    SEXP sdr = sdreport(obj);
    Rcpp::RObject sdr_summary = summary(sdr, "report");

    Rcpp::NumericMatrix mat(sdr_summary);
    Rcpp::List dimnames = mat.attr("dimnames");
    Rcpp::CharacterVector rownames = dimnames[0];
    Rcpp::CharacterVector colnames = dimnames[1];

    // ---- Group into map ----
    std::map<std::string, std::vector<double>> grouped;
    int nrow = mat.nrow();
    for (int i = 0; i < nrow; i++) {
      std::string key = Rcpp::as<std::string>(rownames[i]);
      double val = mat(i, 1);  // col 1 = "Std. Error"
      grouped[key].push_back(val);
    }

    // ---- Convert map -> R list ----
    Rcpp::List grouped_out;
    for (auto const &kv : grouped) {
      grouped_out[kv.first] = Rcpp::wrap(kv.second);
    }

    // Example: grab "Estimate" for first row
    double first_est = mat(0, 0);

    return Rcpp::List::create(
        Rcpp::Named("objective_function_value") = of_value,
        Rcpp::Named("gradient") = grad,
        Rcpp::Named("max_gradient_component") = maxgc,
        Rcpp::Named("report") = rep, Rcpp::Named("sdr_summary") = sdr_summary,
        Rcpp::Named("sdr_summary_matrix") = mat,
        Rcpp::Named("first_est") = first_est,
        Rcpp::Named("rownames") = rownames, Rcpp::Named("colnames") = colnames,
        Rcpp::Named("grouped_se") = grouped_out);
  }
  /**
   * @brief Method to convert the model to a JSON string.
   */
  virtual std::string to_json() {
    Rcpp::List report = get_report();
    Rcpp::List grouped_out = report["grouped_se"];
    double max_gc = Rcpp::as<double>(report["max_gradient_component"]);
    Rcpp::NumericVector grad = report["gradient"];
    double of_value = Rcpp::as<double>(report["objective_function_value"]);

    fims::Vector<double> gradient(grad.size());
    for (int i = 0; i < grad.size(); i++) {
      gradient[i] = grad[i];
    }
    // Assume grouped_out is an Rcpp::List
    std::map<std::string, std::vector<double>> grouped_cpp;
    Rcpp::CharacterVector names = grouped_out.names();
    for (int i = 0; i < grouped_out.size(); i++) {
      std::string key = Rcpp::as<std::string>(names[i]);
      Rcpp::NumericVector vec =
          grouped_out[i];  // each element is a numeric vector
      std::vector<double> vec_std(vec.size());
      for (int j = 0; j < vec.size(); j++) {
        vec_std[j] = vec[j];
      }
      grouped_cpp[key] = vec_std;
    }
    this->se_values = grouped_cpp;

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
    ss << " \"objective_function_value\": " << value << ",\n";
    ss << "\"growth\":[\n";
    for (module_id_it = growth_ids.begin(); module_id_it != growth_ids.end();
         module_id_it++) {
      std::shared_ptr<GrowthInterfaceBase> growth_interface =
          GrowthInterfaceBase::live_objects[*module_id_it];

      if (growth_interface != NULL) {
        growth_interface->set_uncertainty(this->se_values);
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
        recruitment_interface->set_uncertainty(this->se_values);
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
        maturity_interface->set_uncertainty(this->se_values);
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
        selectivity_interface->set_uncertainty(this->se_values);
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
    ss << "\n],\n";
    // add log
    ss << " \"log\": {\n";
    ss << "\"info\": " << fims::FIMSLog::fims_log->get_info() << ","
       << "\"warnings\": " << fims::FIMSLog::fims_log->get_warnings() << ","
       << "\"errors\": " << fims::FIMSLog::fims_log->get_errors() << "}}";
#ifdef TMB_MODEL
    model->do_reporting = true;
#endif
    return fims::JsonParser::PrettyFormatJSON(ss.str());
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
    double min = v[0];
    for (size_t i = 1; i < v.size(); i++) {
      if (v[i] < min) {
        min = v[i];
      }
    }
    return min;
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

      // replace elements in the variable map
      info->variable_map[population->numbers_at_age.id_m] =
          &(derived_quantities["numbers_at_age"]);

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
      // if (fleet_interface->n_lengths.get() > 0)
      // {
      //   info->variable_map[fleet_interface->age_to_length_conversion.id_m] =
      //       &(derived_quantities["age_to_length_conversion"]);
      // }
      // info->variable_map[fleet_interface->lengthcomp_expected.id_m] =
      //     &(derived_quantities["length_comp_expected"]);
      info->variable_map[fleet_interface->lengthcomp_proportion.id_m] =
          &(derived_quantities["lengthcomp_proportion"]);
    }

    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
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
