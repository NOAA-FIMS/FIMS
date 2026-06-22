/**
 * @file fishery_model_base.hpp
 * @brief Defines the base class for all fishery models within the FIMS
 * framework.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_MODELS_FISHERY_MODEL_BASE_HPP
#define FIMS_MODELS_FISHERY_MODEL_BASE_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../../common/model_object.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "../../population_dynamics/population/population.hpp"
/**
 * @brief The population dynamics of FIMS.
 *
 */
namespace fims_popdy {

/**
 * @brief Structure to hold dimension information for derived quantities.
 */
struct DimensionInfo {
  std::string name;                    /*!< name of the derived quantity */
  int ndims;                           /*!< number of dimensions */
  fims::Vector<int> dims;              /*!< vector of dimensions */
  fims::Vector<std::string> dim_names; /*!< vector of dimension names */
  fims::Vector<double> se_values_m;    /*!< final values of the report vector */

  /**
   * @brief Default constructor for dimension information.
   */
  DimensionInfo() : ndims(0) {}

  /**
   * @brief Constructor with parameters.
   * @param name The name of the derived quantity.
   * @param dims A vector of integers representing the dimensions.
   * @param dim_names A vector of strings representing the names of the
   * dimensions.
   */
  DimensionInfo(const std::string &name, const fims::Vector<int> &dims,
                const fims::Vector<std::string> &dim_names)
      : name(name), ndims(dims.size()), dims(dims), dim_names(dim_names) {}

  /**
   * Copy constructor
   */
  DimensionInfo(const DimensionInfo &other)
      : name(other.name),
        ndims(other.dims.size()),
        dims(other.dims),
        dim_names(other.dim_names) {}

  /**
   * @brief Assignment operator for DimensionInfo.
   */
  DimensionInfo &operator=(const DimensionInfo &other) {
    if (this != &other) {
      name = other.name;
      ndims = other.ndims;
      dims = other.dims;
      dim_names = other.dim_names;
      se_values_m = other.se_values_m;
    }
    return *this;
  }
};

/**
 * @brief Container for model derived quantities and their dimension metadata.
 *
 * @details Derived quantities are stored by component ID, e.g., population ID
 * or fleet ID, then by derived quantity name. The class keeps the values and
 * dimension metadata together while preserving map references used throughout
 * the model calculations.
 */
template <typename Type>
class DerivedQuantities {
 public:
  /**
   * @brief Type definitions for derived quantities and dimension information
   * maps.
   */
  typedef typename std::map<uint32_t, std::map<std::string, fims::Vector<Type>>>
      DerivedQuantitiesMap;
  typedef typename std::map<uint32_t, std::map<std::string, DimensionInfo>>
      DimensionInfoMap;

 private:
  DerivedQuantitiesMap values;
  DimensionInfoMap dimension_info;

 public:
  /**
   * @brief Get all derived quantity values.
   */
  DerivedQuantitiesMap &Get() { return values; }

  /**
   * @brief Get all dimension information.
   */
  DimensionInfoMap &GetDimensionInfo() { return dimension_info; }

  /**
   * @brief Initialize storage for a component ID.
   *
   * @param component_id The population or fleet ID to initialize.
   */
  void Initialize(uint32_t component_id) {
    if (values.find(component_id) == values.end()) {
      values.emplace(component_id, std::map<std::string, fims::Vector<Type>>{});
    }
  }

  /**
   * @brief Get derived quantities for a specified component.
   *
   * @param component_id The population or fleet ID.
   * @param context A label used in exception messages.
   * @return std::map<std::string, fims::Vector<Type>>&
   */
  std::map<std::string, fims::Vector<Type>> &Get(
      uint32_t component_id, const std::string &context) {
    auto it = values.find(component_id);
    if (it == values.end()) {
      std::ostringstream ss;
      ss << context << ": component_id " << component_id
         << " not found in derived quantities";
      throw std::out_of_range(ss.str());
    }
    return it->second;
  }

  /**
   * @brief Get dimension information for a specified component.
   *
   * @param component_id The population or fleet ID.
   * @return std::map<std::string, DimensionInfo>&
   */
  std::map<std::string, DimensionInfo> &GetDimensionInfo(
      uint32_t component_id) {
    return dimension_info[component_id];
  }

  /**
   * @brief Add a derived quantity and dimension information.
   *
   * @param component_id The population or fleet ID.
   * @param name The derived quantity name.
   * @param value The derived quantity vector.
   * @param dims Dimension extents.
   * @param dim_names Dimension names.
   */
  void Add(uint32_t component_id, const std::string &name,
           const fims::Vector<Type> &value, const fims::Vector<int> &dims,
           const fims::Vector<std::string> &dim_names) {
    Initialize(component_id);
    values[component_id][name] = value;
    dimension_info[component_id][name] =
        DimensionInfo(name, dims, dim_names);
  }

  /**
   * @brief Reset derived quantity vectors for a specified component.
   *
   * @param component_id The population or fleet ID.
   * @param value The value used for all elements.
   */
  void Reset(uint32_t component_id, Type value = 0.0) {
    std::map<std::string, fims::Vector<Type>> &component_values =
        Get(component_id, "DerivedQuantities::Reset");
    for (auto &kv : component_values) {
      std::fill(kv.second.begin(), kv.second.end(), value);
    }
  }
};

/**
 * @brief FisheryModelBase is a base class for fishery models in FIMS.
 *
 */
template <typename Type>
class FisheryModelBase : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /*!< global id where unique id is drawn from for fishery
                           model object*/
  uint32_t id; /*!< unique identifier assigned for fishery model object */

 public:
#ifdef TMB_MODEL
  bool do_reporting =
      true; /*!< flag to control reporting of derived quantities */
#endif
  /**
   * @brief A string specifying the model type.
   *
   */
  std::string model_type_m;
  /**
   * @brief Unique identifier for the fishery model.
   *
   */
  std::set<uint32_t> population_ids;
  /**
   * @brief A vector of populations in the fishery model.
   *
   */
  std::vector<std::shared_ptr<fims_popdy::Population<Type>>> populations;
  /**
   * @brief A map of fleets in the fishery model, indexed by fleet id.
   * Unique instances to eliminate duplicate initialization.
   *
   */
  std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type>>> fleets;
  /**
   * @brief Fleet-based iterator.
   *
   */
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
      fleet_iterator;

  /**
   * @brief Type definitions for derived quantities and dimension information
   * maps.
   */
  typedef typename DerivedQuantities<Type>::DerivedQuantitiesMap
      DerivedQuantitiesMap;

  /**
   * @brief Iterator for the derived quantities map.
   */
  typedef typename DerivedQuantitiesMap::iterator DerivedQuantitiesMapIterator;

  /**
   * @brief Shared pointer for the fleet derived quantities map.
   */
  std::shared_ptr<DerivedQuantities<Type>> fleet_derived_quantities_ptr;

  /**
   * @brief Shared pointer for the population derived quantities map.
   */
  std::shared_ptr<DerivedQuantities<Type>> population_derived_quantities_ptr;

  /**
   * @brief Type definitions for dimension information maps.
   */
  typedef typename DerivedQuantities<Type>::DimensionInfoMap DimensionInfoMap;

  /**
   * @brief Shared pointer for the fleet dimension information map.
   */
  std::shared_ptr<DerivedQuantities<Type>> fleet_dimension_info;

  /**
   * @brief Shared pointer for the population dimension information map.
   */
  std::shared_ptr<DerivedQuantities<Type>> population_dimension_info;

#ifdef TMB_MODEL
  ::objective_function<Type> *of;
#endif
  /**
   * @brief Construct a new Fishery Model Base object.
   *
   */
  FisheryModelBase() : id(FisheryModelBase::id_g++) {
    fleet_derived_quantities_ptr = std::make_shared<DerivedQuantities<Type>>();
    population_derived_quantities_ptr =
        std::make_shared<DerivedQuantities<Type>>();
    fleet_dimension_info = fleet_derived_quantities_ptr;
    population_dimension_info = population_derived_quantities_ptr;
  }

  /**
   * @brief Construct a new Fishery Model Base object.
   *
   * @param other
   */
  FisheryModelBase(const FisheryModelBase &other)
      : id(other.id),
        population_ids(other.population_ids),
        populations(other.populations),
        fleet_derived_quantities_ptr(other.fleet_derived_quantities_ptr),
        population_derived_quantities_ptr(
            other.population_derived_quantities_ptr),
        fleet_dimension_info(other.fleet_dimension_info),
        population_dimension_info(other.population_dimension_info) {}

  /**
   * @brief Destroy the Fishery Model Base object.
   *
   */
  virtual ~FisheryModelBase() {}

  /**
   * @brief Get the fleet dimension information.
   *
   * @return std::map<uint32_t, std::map<std::string, DimensionInfo>>
   */
  std::map<uint32_t, std::map<std::string, DimensionInfo>> &
  GetFleetDimensionInfo() {
    return fleet_dimension_info->GetDimensionInfo();
  }

  /**
   * @brief Get the population dimension information.
   *
   * @return std::map<uint32_t, std::map<std::string, DimensionInfo>>
   */
  std::map<uint32_t, std::map<std::string, DimensionInfo>> &
  GetPopulationDimensionInfo() {
    return population_dimension_info->GetDimensionInfo();
  }

  /**
   * @brief Get the fleet derived quantities.
   *
   * @return DerivedQuantitiesMap
   */
  DerivedQuantitiesMap &GetFleetDerivedQuantities() {
    return fleet_derived_quantities_ptr->Get();
  }

  /**
   * @brief Get the population derived quantities.
   *
   * @return DerivedQuantitiesMap
   */
  DerivedQuantitiesMap &GetPopulationDerivedQuantities() {
    return population_derived_quantities_ptr->Get();
  }

  /**
   * @brief Get the fleet derived quantities for a specified fleet.
   *
   * @param fleet_id The ID of the fleet.
   * @return std::map<std::string, fims::Vector<Type>>&
   */
  std::map<std::string, fims::Vector<Type>> &GetFleetDerivedQuantities(
      uint32_t fleet_id) {
    if (!fleet_derived_quantities_ptr) {
      throw std::runtime_error(
          "GetFleetDerivedQuantities: fleet_derived_quantities_ptr is null");
    }
    return fleet_derived_quantities_ptr->Get(fleet_id,
                                             "GetFleetDerivedQuantities");
  }

  /**
   * @brief Initialize the derived quantities map for a fleet.
   *
   * @details Ensures the derived quantities map for the specified fleet
   * exists. If not, creates an empty map for the fleet ID.
   *
   * @param fleet_id The ID of the fleet to initialize.
   */
  void InitializeFleetDerivedQuantities(uint32_t fleet_id) {
    // Ensure the shared_ptr exists
    if (!fleet_derived_quantities_ptr) {
      fleet_derived_quantities_ptr =
          std::make_shared<DerivedQuantities<Type>>();
      fleet_dimension_info = fleet_derived_quantities_ptr;
    }

    fleet_derived_quantities_ptr->Initialize(fleet_id);
  }

  /**
   * @brief Initialize the derived quantities map for a population.
   *
   * @details Ensures the derived quantities map for the specified
   * population exists. If not, creates an empty map for the population ID.
   *
   * @param population_id The ID of the population to initialize.
   */
  void InitializePopulationDerivedQuantities(uint32_t population_id) {
    // Ensure the shared_ptr exists
    if (!population_derived_quantities_ptr) {
      population_derived_quantities_ptr =
          std::make_shared<DerivedQuantities<Type>>();
      population_dimension_info = population_derived_quantities_ptr;
    }

    population_derived_quantities_ptr->Initialize(population_id);
  }

  /**
   * @brief Get the population derived quantities for a specified population.
   *
   * @param population_id The ID of the population.
   * @return std::map<std::string, fims::Vector<Type>>&
   */
  std::map<std::string, fims::Vector<Type>> &GetPopulationDerivedQuantities(
      uint32_t population_id) {
    if (!population_derived_quantities_ptr) {
      throw std::runtime_error(
          "GetPopulationDerivedQuantities: population_derived_quantities_ptr "
          "is null");
    }
    return population_derived_quantities_ptr->Get(
        population_id, "GetPopulationDerivedQuantities");
  }

  /**
   * @brief Get the fleet dimension information for a specified fleet.
   *
   * @param fleet_id The ID of the fleet.
   * @return std::map<std::string, DimensionInfo>
   */
  std::map<std::string, DimensionInfo> &GetFleetDimensionInfo(
      uint32_t fleet_id) {
    return fleet_dimension_info->GetDimensionInfo(fleet_id);
  }

  /**
   * @brief Get the population dimension information for a specified population.
   *
   * @param population_id The ID of the population.
   * @return std::map<std::string, DimensionInfo>
   */
  std::map<std::string, DimensionInfo> &GetPopulationDimensionInfo(
      uint32_t population_id) {
    return population_dimension_info->GetDimensionInfo(population_id);
  }

  /**
   * @brief Initialize a model.
   *
   */
  virtual void Initialize() {}

  /**
   * @brief Prepare the model.
   *
   */
  virtual void Prepare() {}

  /**
   * @brief Reset a vector from start to end with a value.
   *
   * @param v A vector to reset.
   * @param value The value you want to use for all elements in the
   * vector. The default is 0.0.
   */
  virtual void ResetVector(fims::Vector<Type> &v, Type value = 0.0) {
    std::fill(v.begin(), v.end(), value);
  }

  /**
   * @brief Evaluate the model.
   *
   */
  virtual void Evaluate() {}

  /**
   * @brief Report the model results via TMB.
   *
   */
  virtual void Report() {}

  /**
   * @brief Get the Id object.
   *
   * @return uint32_t
   */
  uint32_t GetId() { return this->id; }
};

template <typename Type>
uint32_t FisheryModelBase<Type>::id_g = 0;

}  // namespace fims_popdy
#endif
