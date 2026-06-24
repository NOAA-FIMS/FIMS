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

#include "rcpp_interface_base.hpp"
#include "../../../population_dynamics/population/population.hpp"

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
  VariableVector log_M;
  /**
   * @brief The population spawning biomass ratio for each year.
   */
  VariableVector spawning_biomass_ratio;
  /**
   * @brief Log of the population annual fishing mortality multiplier.
   */
  VariableVector log_f_multiplier;
  /**
   * @brief The natural log of the initial numbers at age.
   */
  VariableVector log_init_naa;
  /**
   * @brief Ages that are modeled in the population, the length of this vector
   * should equal \"n_ages\".
   */
  RealVector ages;
  /**
   * @brief The name for the population.
   */
  SharedString name = fims::to_string("NA");

  // Derived quantity VariableVectors
  /** @brief Total landings weight summed across fleets (n_years). */
  VariableVector total_landings_weight;
  /** @brief Total landings numbers summed across fleets (n_years). */
  VariableVector total_landings_numbers;
  /** @brief Fishing mortality by year and age (n_years x n_ages). */
  VariableVector mortality_F;
  /** @brief Natural mortality by year and age (n_years x n_ages). */
  VariableVector mortality_M;
  /** @brief Total mortality by year and age (n_years x n_ages). */
  VariableVector mortality_Z;
  /** @brief Numbers at age ((n_years+1) x n_ages). */
  VariableVector numbers_at_age;
  /** @brief Unfished numbers at age ((n_years+1) x n_ages). */
  VariableVector unfished_numbers_at_age;
  /** @brief Total biomass by year (n_years+1). */
  VariableVector biomass;
  /** @brief Spawning biomass by year (n_years+1). */
  VariableVector spawning_biomass;
  /** @brief Unfished biomass by year (n_years+1). */
  VariableVector unfished_biomass;
  /** @brief Unfished spawning biomass by year (n_years+1). */
  VariableVector unfished_spawning_biomass;
  /** @brief Proportion mature at age ((n_years+1) x n_ages). */
  VariableVector proportion_mature_at_age;
  /** @brief Expected recruitment by year (n_years+1). */
  VariableVector expected_recruitment;
  /** @brief Sum of selectivity across fleets (n_years x n_ages). */
  VariableVector sum_selectivity;

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
        n_ages(other.n_ages),
        n_fleets(other.n_fleets),
        fleet_ids(other.fleet_ids),
        n_years(other.n_years),
        n_lengths(other.n_lengths),
        maturity_id(other.maturity_id),
        growth_id(other.growth_id),
        recruitment_id(other.recruitment_id),
        recruitment_err_id(other.recruitment_id),
        log_M(other.log_M),
        spawning_biomass_ratio(other.spawning_biomass_ratio),
        log_f_multiplier(other.log_f_multiplier),
        log_init_naa(other.log_init_naa),
        ages(other.ages),
        name(other.name),
        total_landings_weight(other.total_landings_weight),
        total_landings_numbers(other.total_landings_numbers),
        mortality_F(other.mortality_F),
        mortality_M(other.mortality_M),
        mortality_Z(other.mortality_Z),
        numbers_at_age(other.numbers_at_age),
        unfished_numbers_at_age(other.unfished_numbers_at_age),
        biomass(other.biomass),
        spawning_biomass(other.spawning_biomass),
        unfished_biomass(other.unfished_biomass),
        unfished_spawning_biomass(other.unfished_spawning_biomass),
        proportion_mature_at_age(other.proportion_mature_at_age),
        expected_recruitment(other.expected_recruitment),
        sum_selectivity(other.sum_selectivity) {}

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

    if (it == info->populations.end()) {
      FIMS_WARNING_LOG("Population " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    }

    std::shared_ptr<fims_popdy::Population<double>> pop =
        info->populations[this->id];

    for (size_t i = 0; i < this->log_M.size(); i++) {
      if (this->log_M[i].estimation_type_m.get() == "constant") {
        this->log_M[i].final_value_m = this->log_M[i].initial_value_m;
      } else {
        this->log_M[i].final_value_m = pop->log_M[i];
      }
    }

    for (size_t i = 0; i < this->log_f_multiplier.size(); i++) {
      if (this->log_f_multiplier[i].estimation_type_m.get() == "constant") {
        this->log_f_multiplier[i].final_value_m =
            this->log_f_multiplier[i].initial_value_m;
      } else {
        this->log_f_multiplier[i].final_value_m = pop->log_f_multiplier[i];
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

    for (size_t i = 0; i < this->spawning_biomass_ratio.size(); i++) {
      this->spawning_biomass_ratio[i].final_value_m =
          pop->spawning_biomass_ratio[i];
    }

    // Finalize derived quantities
    this->total_landings_weight.resize(pop->total_landings_weight.size());
    for (size_t i = 0; i < pop->total_landings_weight.size(); i++) {
      this->total_landings_weight[i].final_value_m =
          pop->total_landings_weight[i];
    }

    this->total_landings_numbers.resize(pop->total_landings_numbers.size());
    for (size_t i = 0; i < pop->total_landings_numbers.size(); i++) {
      this->total_landings_numbers[i].final_value_m =
          pop->total_landings_numbers[i];
    }

    this->mortality_F.resize(pop->mortality_F.size());
    for (size_t i = 0; i < pop->mortality_F.size(); i++) {
      this->mortality_F[i].final_value_m = pop->mortality_F[i];
    }

    this->mortality_M.resize(pop->mortality_M.size());
    for (size_t i = 0; i < pop->mortality_M.size(); i++) {
      this->mortality_M[i].final_value_m = pop->mortality_M[i];
    }

    this->mortality_Z.resize(pop->mortality_Z.size());
    for (size_t i = 0; i < pop->mortality_Z.size(); i++) {
      this->mortality_Z[i].final_value_m = pop->mortality_Z[i];
    }

    this->numbers_at_age.resize(pop->numbers_at_age.size());
    for (size_t i = 0; i < pop->numbers_at_age.size(); i++) {
      this->numbers_at_age[i].final_value_m = pop->numbers_at_age[i];
    }

    this->unfished_numbers_at_age.resize(pop->unfished_numbers_at_age.size());
    for (size_t i = 0; i < pop->unfished_numbers_at_age.size(); i++) {
      this->unfished_numbers_at_age[i].final_value_m =
          pop->unfished_numbers_at_age[i];
    }

    this->biomass.resize(pop->biomass.size());
    for (size_t i = 0; i < pop->biomass.size(); i++) {
      this->biomass[i].final_value_m = pop->biomass[i];
    }

    this->spawning_biomass.resize(pop->spawning_biomass.size());
    for (size_t i = 0; i < pop->spawning_biomass.size(); i++) {
      this->spawning_biomass[i].final_value_m = pop->spawning_biomass[i];
    }

    this->unfished_biomass.resize(pop->unfished_biomass.size());
    for (size_t i = 0; i < pop->unfished_biomass.size(); i++) {
      this->unfished_biomass[i].final_value_m = pop->unfished_biomass[i];
    }

    this->unfished_spawning_biomass.resize(
        pop->unfished_spawning_biomass.size());
    for (size_t i = 0; i < pop->unfished_spawning_biomass.size(); i++) {
      this->unfished_spawning_biomass[i].final_value_m =
          pop->unfished_spawning_biomass[i];
    }

    this->proportion_mature_at_age.resize(pop->proportion_mature_at_age.size());
    for (size_t i = 0; i < pop->proportion_mature_at_age.size(); i++) {
      this->proportion_mature_at_age[i].final_value_m =
          pop->proportion_mature_at_age[i];
    }

    this->expected_recruitment.resize(pop->expected_recruitment.size());
    for (size_t i = 0; i < pop->expected_recruitment.size(); i++) {
      this->expected_recruitment[i].final_value_m =
          pop->expected_recruitment[i];
    }

    this->sum_selectivity.resize(pop->sum_selectivity.size());
    for (size_t i = 0; i < pop->sum_selectivity.size(); i++) {
      this->sum_selectivity[i].final_value_m = pop->sum_selectivity[i];
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
      if (static_cast<size_t>(this->n_ages.get()) == this->ages.size()) {
        population->ages.resize(this->n_ages.get());
      } else {
        throw std::invalid_argument(
            "The size of the ages vector for population " +
            fims::to_string(this->id) + " is not equal to n_ages.");
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

    if (this->log_f_multiplier.size() ==
        static_cast<size_t>(this->n_years.get())) {
      population->log_f_multiplier.resize(this->log_f_multiplier.size());
    } else {
      FIMS_WARNING_LOG(
          "The log_f_multiplier vector is not of size n_years. Filling with "
          "zeros.");
      this->log_f_multiplier.resize((this->n_years.get()));
      for (size_t i = 0; i < log_f_multiplier.size(); i++) {
        this->log_f_multiplier[i].initial_value_m = static_cast<double>(0.0);
        this->log_f_multiplier[i].estimation_type_m.set("constant");
      }
      population->log_f_multiplier.resize(this->log_f_multiplier.size());
    }

    if (this->spawning_biomass_ratio.size() ==
        static_cast<size_t>(this->n_years.get() + 1)) {
      population->spawning_biomass_ratio.resize(
          this->spawning_biomass_ratio.size());
    } else {
      FIMS_WARNING_LOG(
          "Setting spawning_biomass_ratio vector to size n_years + 1.");
      this->spawning_biomass_ratio.resize((this->n_years.get() + 1));
      population->spawning_biomass_ratio.resize(
          this->spawning_biomass_ratio.size());
    }
    info->variable_map[this->spawning_biomass_ratio.id_m] =
        &(population)->spawning_biomass_ratio;

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

    for (size_t i = 0; i < log_f_multiplier.size(); i++) {
      population->log_f_multiplier[i] =
          this->log_f_multiplier[i].initial_value_m;
      if (this->log_f_multiplier[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "Population." << this->id << ".log_f_multiplier."
           << this->log_f_multiplier[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(population->log_f_multiplier[i]);
      }
      if (this->log_f_multiplier[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "Population." << this->id << ".log_f_multiplier."
           << this->log_f_multiplier[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(population->log_f_multiplier[i]);
      }
    }
    info->variable_map[this->log_f_multiplier.id_m] =
        &(population)->log_f_multiplier;

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

    for (size_t i = 0; i < ages.size(); i++) {
      population->ages[i] = this->ages[i];
    }

    // Link derived quantity vectors to variable_map
    // (these are resized in CatchAtAge::Initialize(); linking the pointer here
    // is safe because resize does not move the fims::Vector object itself)
    info->variable_map[this->total_landings_weight.id_m] =
        &(population)->total_landings_weight;
    info->variable_map[this->total_landings_numbers.id_m] =
        &(population)->total_landings_numbers;
    info->variable_map[this->mortality_F.id_m] = &(population)->mortality_F;
    info->variable_map[this->mortality_M.id_m] = &(population)->mortality_M;
    info->variable_map[this->mortality_Z.id_m] = &(population)->mortality_Z;
    info->variable_map[this->numbers_at_age.id_m] =
        &(population)->numbers_at_age;
    info->variable_map[this->unfished_numbers_at_age.id_m] =
        &(population)->unfished_numbers_at_age;
    info->variable_map[this->biomass.id_m] = &(population)->biomass;
    info->variable_map[this->spawning_biomass.id_m] =
        &(population)->spawning_biomass;
    info->variable_map[this->unfished_biomass.id_m] =
        &(population)->unfished_biomass;
    info->variable_map[this->unfished_spawning_biomass.id_m] =
        &(population)->unfished_spawning_biomass;
    info->variable_map[this->proportion_mature_at_age.id_m] =
        &(population)->proportion_mature_at_age;
    info->variable_map[this->expected_recruitment.id_m] =
        &(population)->expected_recruitment;
    info->variable_map[this->sum_selectivity.id_m] =
        &(population)->sum_selectivity;

    // add to Information
    info->populations[population->id] = population;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

#endif
