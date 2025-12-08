/**
 * @file rcpp_population.hpp
 * @brief The Rcpp interface to declare different types of populations. Allows
 * for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP

#include "../../../population_dynamics/population/population.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp population
 * interfaces. This type should be inherited and not called from R directly.
 */
class PopulationInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the PopulationInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the PopulationInterfaceBase object.
   */
  uint32_t id;
  /**
   * @brief The map associating the IDs of PopulationInterfaceBase to the
   * objects. This is a live object, which is an object that has been created
   * and lives in memory.
   */
  static std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>
      live_objects;

  /**
   * @brief Initialize the catch at age model.
   *
   */
  SharedBoolean initialize_catch_at_age;
  /**
   * @brief Initialize the surplus production model.
   *
   */
  SharedBoolean initialize_surplus_production;
  /**
   * @brief The constructor.
   */
  PopulationInterfaceBase() {
    this->id = PopulationInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    PopulationInterfaceBase */
    // PopulationInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Population Interface Base object
   *
   * @param other
   */
  PopulationInterfaceBase(const PopulationInterfaceBase &other)
      : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~PopulationInterfaceBase() {}

  /**
   * @brief Get the ID for the child population interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;
};
// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;
// local id of the PopulationInterfaceBase object map relating the ID of the
// PopulationInterfaceBase to the PopulationInterfaceBase objects
std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>
    PopulationInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for a new Population to instantiate from R:
 * population <- methods::new(population)
 */
class PopulationInterface : public PopulationInterfaceBase {
 public:
  /**
   * @brief The number of age bins.
   */
  SharedInt n_ages = 0;
  /**
   * @brief The number of fleets.
   */
  SharedInt n_fleets;
  /**
   * list of fleets that operate on this population.
   */
  std::shared_ptr<std::set<uint32_t>> fleet_ids;
  /**
   * Iterator for fleet ids.
   */
  typedef typename std::set<uint32_t>::iterator fleet_ids_iterator;
  /**
   * @brief The number of years.
   */
  SharedInt n_years;
  /**
   * @brief The number of length bins.
   */
  SharedInt n_lengths;
  /**
   * @brief The ID of the maturity module.
   */
  SharedInt maturity_id;
  /**
   * @brief The ID of the growth module.
   */
  SharedInt growth_id;
  /**
   * @brief The ID of the recruitment module.
   */
  SharedInt recruitment_id;
  /**
   * @brief The ID of the recruitment process module.
   */
  SharedInt recruitment_err_id;
  /**
   * @brief The natural log of the natural mortality for each year.
   */
  ParameterVector log_M;
  /**
   * @brief The natural log of the initial numbers at age.
   */
  ParameterVector log_init_naa;
  /**
   * @brief Numbers at age.
   */
  ParameterVector numbers_at_age;
  /**
   * @brief random effect for recruitment.
   */
  ParameterVector log_r;
  /**
   * @brief Ages that are modeled in the population, the length of this vector
   * should equal \"n_ages\".
   */
  RealVector ages;
  /**
   * @brief Derived spawning biomass.
   * TODO: This should be sb not ssb if left as an acronym.
   */
  Rcpp::NumericVector derived_ssb;
  /**
   * @brief Derived numbers at age.
   */
  Rcpp::NumericVector derived_naa;
  /**
   * @brief Derived biomass (mt).
   */
  Rcpp::NumericVector derived_biomass;
  /**
   * @brief Derived recruitment in numbers.
   */
  Rcpp::NumericVector derived_recruitment;
  /**
   * @brief The name for the population.
   */
  SharedString name = fims::to_string("NA");

  /**
   * @brief The constructor.
   */
  PopulationInterface() : PopulationInterfaceBase() {
    this->fleet_ids = std::make_shared<std::set<uint32_t>>();
    std::shared_ptr<PopulationInterface> population =
        std::make_shared<PopulationInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(population);
    PopulationInterfaceBase::live_objects[this->id] = population;
  }

  /**
   * @brief Construct a new Population Interface object
   *
   * @param other
   */
  PopulationInterface(const PopulationInterface &other)
      : PopulationInterfaceBase(other),
        fleet_ids(other.fleet_ids),
        n_ages(other.n_ages),
        n_fleets(other.n_fleets),
        n_years(other.n_years),
        n_lengths(other.n_lengths),
        maturity_id(other.maturity_id),
        growth_id(other.growth_id),
        recruitment_id(other.recruitment_id),
        log_M(other.log_M),
        log_init_naa(other.log_init_naa),
        numbers_at_age(other.numbers_at_age),
        ages(other.ages),
        derived_ssb(other.derived_ssb),
        derived_naa(other.derived_naa),
        derived_biomass(other.derived_biomass),
        derived_recruitment(other.derived_recruitment),
        name(other.name) {}

  /**
   * @brief The destructor.
   */
  virtual ~PopulationInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Sets the name of the population.
   * @param name The name to set.
   */
  void SetName(const std::string &name) { this->name.set(name); }

  /**
   * @brief Gets the name of the population.
   * @return The name.
   */
  std::string GetName() const { return this->name.get(); }

  /**
   * @brief Sets the unique ID for the Maturity object.
   * @param maturity_id Unique ID for the Maturity object.
   */
  void SetMaturityID(uint32_t maturity_id) {
    this->maturity_id.set(maturity_id);
  }

  /**
   * @brief Set the unique ID for the growth object.
   * @param growth_id Unique ID for the growth object.
   */
  void SetGrowthID(uint32_t growth_id) { this->growth_id.set(growth_id); }

  /**
   * @brief Set the unique ID for the recruitment object.
   * @param recruitment_id Unique ID for the recruitment object.
   */
  void SetRecruitmentID(uint32_t recruitment_id) {
    this->recruitment_id.set(recruitment_id);
  }

  /**
   * @brief Add a fleet id to the list of fleets
   * operating on this population.
   */
  void AddFleet(uint32_t fleet_id) { this->fleet_ids->insert(fleet_id); }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Population " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::population_iterator it;

    it = info->populations.find(this->id);

    std::shared_ptr<fims_popdy::Population<double>> pop =
        info->populations[this->id];
    it = info->populations.find(this->id);
    if (it == info->populations.end()) {
      FIMS_WARNING_LOG("Population " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      for (size_t i = 0; i < this->log_M.size(); i++) {
        if (this->log_M[i].estimation_type_m.get() == "constant") {
          this->log_M[i].final_value_m = this->log_M[i].initial_value_m;
        } else {
          this->log_M[i].final_value_m = pop->log_M[i];
        }
      }

      for (size_t i = 0; i < this->log_init_naa.size(); i++) {
        if (this->log_init_naa[i].estimation_type_m.get() == "constant") {
          this->log_init_naa[i].final_value_m =
              this->log_init_naa[i].initial_value_m;
        } else {
          this->log_init_naa[i].final_value_m = pop->log_init_naa[i];
        }
      }
    }
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::Population<Type>> population =
        std::make_shared<fims_popdy::Population<Type>>();

    std::stringstream ss;

    // set relative info
    population->id = this->id;
    population->n_years = this->n_years.get();
    population->n_fleets = this->n_fleets.get();
    // only define ages if n_ages greater than 0
    if (this->n_ages.get() > 0) {
      population->n_ages = this->n_ages.get();
      if (this->n_ages.get() == this->ages.size()) {
        population->ages.resize(this->n_ages.get());
      } else {
        warning("The ages vector is not of size n_ages.");
      }
    }

    fleet_ids_iterator it;
    for (it = this->fleet_ids->begin(); it != this->fleet_ids->end(); it++) {
      population->fleet_ids.insert(*it);
    }

    population->growth_id = this->growth_id.get();
    population->recruitment_id = this->recruitment_id.get();
    population->maturity_id = this->maturity_id.get();
    population->log_M.resize(this->log_M.size());
    population->log_init_naa.resize(this->log_init_naa.size());
    for (size_t i = 0; i < log_M.size(); i++) {
      population->log_M[i] = this->log_M[i].initial_value_m;
      if (this->log_M[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Population." << this->id << ".log_M." << this->log_M[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(population->log_M[i]);
      }
      if (this->log_M[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Population." << this->id << ".log_M." << this->log_M[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(population->log_M[i]);
      }
    }
    info->variable_map[this->log_M.id_m] = &(population)->log_M;

    for (size_t i = 0; i < log_init_naa.size(); i++) {
      population->log_init_naa[i] = this->log_init_naa[i].initial_value_m;
      if (this->log_init_naa[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Population." << this->id << ".log_init_naa."
           << this->log_init_naa[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(population->log_init_naa[i]);
      }
      if (this->log_init_naa[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Population." << this->id << ".log_init_naa."
           << this->log_init_naa[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(population->log_init_naa[i]);
      }
    }
    info->variable_map[this->log_init_naa.id_m] = &(population)->log_init_naa;

    for (int i = 0; i < ages.size(); i++) {
      population->ages[i] = this->ages[i];
    }

    // add to Information
    info->populations[population->id] = population;

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
