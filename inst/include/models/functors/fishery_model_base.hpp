#ifndef FIMS_MODELS_FISHERY_MODEL_BASE_HPP
#define FIMS_MODELS_FISHERY_MODEL_BASE_HPP

#include "../../common/model_object.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "../../population_dynamics/population/population.hpp"

/**
 * @brief The population dynamics of FIMS.
 *
 */
namespace fims_popdy {

template <typename Type>
/**
 * @brief FisheryModelBase is a base class for fishery models in FIMS.
 *
 */
class FisheryModelBase : public fims_model_object::FIMSObject<Type> {
  public: 
  static uint32_t id_g; /**< The ID of the instance of the FisheryModelBase class */
  uint32_t id; /*!< unique identifier assigned for fishery model object */

 public:
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
  typedef typename std::map<uint32_t, std::map<std::string, fims::Vector<Type>>>
      DerivedQuantitiesMap;
  /**
   * @brief Iterator for the derived quantities map.
   */
  typedef typename DerivedQuantitiesMap::iterator DerivedQuantitiesMapIterator;

  /**
   * @brief Shared pointer for the fleet derived quantities map.
   */
  std::shared_ptr<DerivedQuantitiesMap> fleet_derived_quantities;

  /**
   * @brief Shared pointer for the population derived quantities map.
   */
  std::shared_ptr<DerivedQuantitiesMap> population_derived_quantities;

#ifdef TMB_MODEL
  ::objective_function<Type> *of;
#endif
  /**
   * @brief Construct a new Fishery Model Base object.
   *
   */
  FisheryModelBase() : id(FisheryModelBase::id_g++) {
    fleet_derived_quantities = std::make_shared<DerivedQuantitiesMap>();
    population_derived_quantities = std::make_shared<DerivedQuantitiesMap>();
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
        fleet_derived_quantities(other.fleet_derived_quantities),
        population_derived_quantities(other.population_derived_quantities) {}

  /**
   * @brief Destroy the Fishery Model Base object.
   *
   */
  virtual ~FisheryModelBase() {}

  /**
   * @brief Show the Catch At Age model derived quantities.
   *
   * @param p
   */
  void ShowPopulation(std::shared_ptr<fims_popdy::Population<double>> &p) {
    std::cout << "\n\nCAA Model Derived Quantities:\n";
    typename fims_popdy::Population<double>::derived_quantities_iterator it;
    for (it = p->derived_quantities.begin(); it != p->derived_quantities.end();
         it++) {
      fims::Vector<double> &dq = (*it).second;
      std::cout << (*it).first << ":" << std::endl;
      for (int i = 0; i < dq.size(); i++) {
        std::cout << dq[i] << " ";
      }
      std::cout << std::endl << std::endl;
    }
  }

  /**
   * @brief Show the populations in the fishery model.
   *
   */
  void Show() {
    for (size_t p = 0; p < this->populations.size(); p++) {
      this->ShowPopulation(this->populations[p]);
    }
  }

   /**
   * @brief Get the fleet derived quantities.
   *
   * @return DerivedQuantitiesMap
   */
  DerivedQuantitiesMap &GetFleetDerivedQuantities() {
    return *fleet_derived_quantities;
  }

  /**
   * @brief Get the population derived quantities.
   *
   * @return DerivedQuantitiesMap
   */
  DerivedQuantitiesMap &GetPopulationDerivedQuantities() {
    return *population_derived_quantities;
  }
  /**
   * @brief Get the fleet derived quantities for a specified fleet.
   *
   * @param fleet_id The ID of the fleet.
   * @return std::map<std::string, fims::Vector<Type>>&
   */
  std::map<std::string, fims::Vector<Type>> &GetFleetDerivedQuantities(
      uint32_t fleet_id) {
    if (!fleet_derived_quantities) {
      throw std::runtime_error(
          "GetFleetDerivedQuantities: fleet_derived_quantities is null");
    }
    auto &outer = *fleet_derived_quantities;
    auto it = outer.find(fleet_id);
    if (it == outer.end()) {
      std::stringstream ss;

      ss << "GetFleetDerivedQuantities: fleet_id " << fleet_id
         << " not found in fleet_derived_quantities";
      throw std::out_of_range(ss.str());
    }
    return it->second;
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
    if (!fleet_derived_quantities) {
      fleet_derived_quantities = std::make_shared<
          std::map<uint32_t, std::map<std::string, fims::Vector<Type>>>>();
    }

    auto &outer = *fleet_derived_quantities;

    // Insert only if not already present
    if (outer.find(fleet_id) == outer.end()) {
      outer.emplace(fleet_id, std::map<std::string, fims::Vector<Type>>{});
    }
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
    if (!population_derived_quantities) {
      population_derived_quantities = std::make_shared<
          std::map<uint32_t, std::map<std::string, fims::Vector<Type>>>>();
    }

    auto &outer = *population_derived_quantities;

    // Insert only if not already present
    if (outer.find(population_id) == outer.end()) {
      outer.emplace(population_id, std::map<std::string, fims::Vector<Type>>{});
    }
  }

  /**
   * @brief Get the population derived quantities for a specified population.
   *
   * @param population_id The ID of the population.
   * @return std::map<std::string, fims::Vector<Type>>&
   */
  std::map<std::string, fims::Vector<Type>> &GetPopulationDerivedQuantities(
      uint32_t population_id) {
    if (!population_derived_quantities) {
      throw std::runtime_error(
          "GetPopulationDerivedQuantities: population_derived_quantities is "
          "null");
    }
    auto &outer = *population_derived_quantities;
    auto it = outer.find(population_id);
    if (it == outer.end()) {
      std::ostringstream ss;
      ss << "GetPopulationDerivedQuantities: population_id " << population_id
         << " not found in population_derived_quantities";
      throw std::out_of_range(ss.str());
    }
    return it->second;
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
