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
  static uint32_t id_g;
  uint32_t id;

 public:
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

#ifdef TMB_MODEL
  ::objective_function<Type> *of;
#endif
  /**
   * @brief Construct a new Fishery Model Base object.
   *
   */
  FisheryModelBase() : id(FisheryModelBase::id_g++) {}

  /**
   * @brief Construct a new Fishery Model Base object.
   *
   * @param other
   */
  FisheryModelBase(const FisheryModelBase &other) : id(other.id) {
    this->population_ids = other.population_ids;
    this->populations = other.populations;
  }

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
