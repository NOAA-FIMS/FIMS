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
#include "rcpp_fleet.hpp"
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
   * @brief Initialize the catch at age model.
   *
   */
  bool initialize_catch_at_age = false;
  /**
   * @brief Initialize the surplus production model.
   *
   */
  bool initialize_surplus_production = false;
  /**
   * @brief The constructor.
   */
  PopulationInterfaceBase() { this->id = PopulationInterfaceBase::id_g++; }

  /**
   * @brief Interface objects are not copyable.
   */
  PopulationInterfaceBase(const PopulationInterfaceBase &) = delete;
  PopulationInterfaceBase &operator=(const PopulationInterfaceBase &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~PopulationInterfaceBase() {}

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
  int n_ages = 0;
  /**
   * @brief The number of fleets.
   */
  int n_fleets = 0;
  /**
   * list of fleets that operate on this population.
   */
  std::shared_ptr<std::set<uint32_t>> fleet_ids;
  /**
   * @brief The fleet modules that operate on this population.
   *
   * @details Population holds direct shared_ptrs to the fleets. 
   * CatchAtAgeInterface walks these to reach every fleet in the
   * model, both to size its derived-quantity maps and to serialize it.
   *
   * fleet_ids and fleets_m are always written together by SetFleets(), so the
   * association and the reachability link cannot drift apart. A fleet that
   * operates on several populations appears in each population's list and is
   * de-duplicated by ID in CatchAtAgeInterface::GetFleets().
   */
  std::vector<std::shared_ptr<FleetInterface>> fleets_m;
  /**
   * Iterator for fleet ids.
   */
  typedef typename std::set<uint32_t>::iterator fleet_ids_iterator;
  /**
   * @brief The number of years.
   */
  int n_years = 0;
    /**
   * @brief The ID of the maturity module.
   */
  int maturity_id = -999;
  /**
   * @brief The ID of the growth module.
   */
  int growth_id = -999;
  /**
   * @brief The ID of the recruitment module.
   */
  int recruitment_id = -999;
  /**
   * @brief The ID of the recruitment process module.
   */
  int recruitment_err_id = -999;
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
   * @brief Proportion of females in the population.
   *
   * Used to calculate spawning biomass and other sex-specific derived
   * quantities. Supplied from R as a single constant (length 1) and applied
   * to all ages during model evaluation. Values should be in [0, 1].
   * Out-of-range inputs are logged as warnings.
   */
  VariableVector proportion_female;
  /**
   * @brief Ages that are modeled in the population, the length of this vector
   * should equal \"n_ages\".
   */
  fims::Vector<double> ages;
  /**
   * @brief The name for the population.
   */
  std::string name = "NA";

  // Population based derived quantities
  /**
   * @brief Total annual catch removed from a population by all fleets in
   * weight.
   */
  VariableVector total_catch_weight;

  /**
   * @brief Total annual catch removed from a population by all fleets in
   * numbers.
   */
  VariableVector total_catch_numbers;

  /**
   * @brief Total annual fishing mortality a population is subject to.
   */
  VariableVector mortality_F;

  /**
   * @brief Total annual natural mortality a population is subject to.
   */
  VariableVector mortality_M;

  /**
   * @brief Total annual mortality a population is subject to.
   */
  VariableVector mortality_Z;

  /**
   * @brief Current population composition in numbers at age.
   */
  VariableVector numbers_at_age;

  /**
   * @brief Theoretical population composition in numbers at age if no fishing
   * had occurred.
   */
  VariableVector unfished_numbers_at_age;

  /**
   * @brief Total weight of all fish in the population.
   */
  VariableVector biomass;
  /**
   * @brief Total weight of mature fish in the population.
   */
  VariableVector spawning_biomass;
  /**
   * @brief Total theoretical weight of all fish in the population if no fishing
   * had occurred.
   */
  VariableVector unfished_biomass;
  /**
   * @brief Total theoretical weight of mature fish in the population if no
   * fishing had occurred.
   */
  VariableVector unfished_spawning_biomass;
  /**
   * @brief Fraction of all fish at a given age that are sexually mature at
   * each age.
   */
  VariableVector proportion_mature_at_age;
  /**
   * @brief Model-expected recruitment each year based on the stock--recruit
   * relationship.
   */
  VariableVector expected_recruitment;

  /**
   * @brief Sum of selectivity at age across all fleets for a population.
   */
  VariableVector sum_selectivity;

  /**
   * @brief The constructor.
   */
  PopulationInterface() : PopulationInterfaceBase() {
    this->proportion_female[0].initial_value_m = static_cast<double>(0.5);
    this->proportion_female[0].estimation_status_m = fims_enum::EstimationStatus::kAssumedKnown;
    this->fleet_ids = std::make_shared<std::set<uint32_t>>();
  }

  /**
   * @brief Interface objects are not copyable.
   */
  PopulationInterface(const PopulationInterface &) = delete;
  PopulationInterface &operator=(const PopulationInterface &) = delete;

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
   * @copydoc FIMSRcppInterfaceBase::get_numeric_vector
   */
  virtual fims::Vector<double> *get_numeric_vector(const std::string &name) {
    if (name == "ages") return &this->ages;
    return nullptr;
  }

  /**
   * @copydoc FIMSRcppInterfaceBase::get_variable_vector
   */
  virtual VariableVector *get_variable_vector(const std::string &name) {
    if (name == "log_M") return &this->log_M;
    if (name == "log_init_naa") return &this->log_init_naa;
    if (name == "log_f_multiplier") return &this->log_f_multiplier;
    if (name == "proportion_female") return &this->proportion_female;
    if (name == "spawning_biomass_ratio") return &this->spawning_biomass_ratio;

    // Quantities the model computes. Reachable by name for the same reason
    // the inputs are: each is a VariableVector with an id in
    // Information::variable_map, which is how a distribution says which
    // quantity it applies to.
    if (name == "numbers_at_age") return &this->numbers_at_age;
    if (name == "unfished_numbers_at_age")
      return &this->unfished_numbers_at_age;
    if (name == "biomass") return &this->biomass;
    if (name == "spawning_biomass") return &this->spawning_biomass;
    if (name == "unfished_biomass") return &this->unfished_biomass;
    if (name == "unfished_spawning_biomass")
      return &this->unfished_spawning_biomass;
    if (name == "proportion_mature_at_age")
      return &this->proportion_mature_at_age;
    if (name == "expected_recruitment") return &this->expected_recruitment;
    if (name == "sum_selectivity") return &this->sum_selectivity;
    if (name == "mortality_F") return &this->mortality_F;
    if (name == "mortality_M") return &this->mortality_M;
    if (name == "mortality_Z") return &this->mortality_Z;
    if (name == "total_catch_numbers") return &this->total_catch_numbers;
    if (name == "total_catch_weight") return &this->total_catch_weight;
    return nullptr;
  }

  /**
   * @brief Sets the name of the population.
   * @param name The name to set.
   */
  void SetName(const std::string &name) { this->name = name; }

  /**
   * @brief Gets the name of the population.
   * @return The name.
   */
  std::string GetName() const { return this->name; }

  /**
   * @brief Sets the unique ID for the Maturity object.
   * @param maturity_id Unique ID for the Maturity object.
   */
  void SetMaturityID(uint32_t maturity_id) {
    this->maturity_id = maturity_id;
  }

  /**
   * @brief Set the unique ID for the growth object.
   * @param growth_id Unique ID for the growth object.
   */
  void SetGrowthID(uint32_t growth_id) { this->growth_id = growth_id; }

  /**
   * @brief Set the unique ID for the recruitment object.
   * @param recruitment_id Unique ID for the recruitment object.
   */
  void SetRecruitmentID(uint32_t recruitment_id) {
    this->recruitment_id = recruitment_id;
  }

  /**
   * @brief Set the fleets that operate on this population, replacing whatever
   * was there before.
   *
   * @details Records each fleet's unique ID and a direct link to the interface
   * object, rebuilding both together so they cannot disagree. Replacing rather
   * than appending is what lets a population copied from another one be given
   * a different set of fleets.
   *
   * A fleet may operate on more than one population, so the same fleet
   * appearing in two populations' lists is expected. The same fleet appearing
   * twice in one list is not, and the repeat is ignored.
   *
   * @param fleets The fleet interface objects, in the order they should be
   * recorded.
   */
  void SetFleets(const std::vector<std::shared_ptr<FleetInterface>> &fleets) {
    this->fleet_ids->clear();
    this->fleets_m.clear();
    for (size_t i = 0; i < fleets.size(); i++) {
      if (!fleets[i]) {
        FIMS_ERROR_LOG("Cannot add a null fleet to Population " +
                       fims::to_string(this->id) + ".");
        continue;
      }
      if (!this->fleet_ids->insert(fleets[i]->get_id()).second) {
        FIMS_WARNING_LOG("Fleet " + fims::to_string(fleets[i]->get_id()) +
                         " appears more than once in the fleets given to "
                         "Population " + fims::to_string(this->id) +
                         "; ignoring the repeat.");
        continue;
      }
      this->fleets_m.push_back(fleets[i]);
    }
    this->n_fleets = static_cast<int>(this->fleets_m.size());
  }

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
        set_final_value_by_estimation_status(this->log_M[i], pop->log_M[i]);
      }

      for (size_t i = 0; i < this->log_f_multiplier.size(); i++) {
        set_final_value_by_estimation_status(this->log_f_multiplier[i],
                                             pop->log_f_multiplier[i]);
      }

      for (size_t i = 0; i < this->log_init_naa.size(); i++) {
        set_final_value_by_estimation_status(this->log_init_naa[i],
                                             pop->log_init_naa[i]);
      }

      for (size_t i = 0; i < this->proportion_female.size(); i++) {
        set_final_value_by_estimation_status(
            this->proportion_female[i], pop->proportion_female.get_force_scalar(i));
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
    population->n_years = this->n_years;
    population->n_fleets = this->n_fleets;
    // only define ages if n_ages greater than 0
    if (this->n_ages > 0) {
      population->n_ages = this->n_ages;
      if (static_cast<size_t>(this->n_ages) == this->ages.size()) {
        population->ages.resize(this->n_ages);
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

    population->growth_id = this->growth_id;
    population->recruitment_id = this->recruitment_id;
    population->maturity_id = this->maturity_id;
    population->log_M.resize(this->log_M.size());

    if (this->log_f_multiplier.size() ==
        static_cast<size_t>(this->n_years)) {
      population->log_f_multiplier.resize(this->log_f_multiplier.size());
    } else {
      FIMS_WARNING_LOG(
          "The log_f_multiplier vector is not of size n_years. Filling with "
          "zeros.");
      this->log_f_multiplier.resize(this->n_years);
      for (size_t i = 0; i < log_f_multiplier.size(); i++) {
        this->log_f_multiplier[i].initial_value_m = static_cast<double>(0.0);
        this->log_f_multiplier[i].estimation_status_m = fims_enum::EstimationStatus::kAssumedKnown;
      }
      population->log_f_multiplier.resize(this->log_f_multiplier.size());
    }

    if (this->spawning_biomass_ratio.size() ==
        static_cast<size_t>(this->n_years + 1)) {
      population->spawning_biomass_ratio.resize(
          this->spawning_biomass_ratio.size());
    } else {
      FIMS_WARNING_LOG(
          "Setting spawning_biomass_ratio vector to size n_years + 1.");
      this->spawning_biomass_ratio.resize(this->n_years + 1);
      population->spawning_biomass_ratio.resize(
          this->spawning_biomass_ratio.size());
    }
    info->variable_map[this->spawning_biomass_ratio.id_m] =
        &(population)->spawning_biomass_ratio;

    population->log_init_naa.resize(this->log_init_naa.size());
    for (size_t i = 0; i < log_M.size(); i++) {
      population->log_M[i] = this->log_M[i].initial_value_m;
      ss.str("");
      ss << "Population." << this->id << ".log_M." << this->log_M[i].id_m;
      register_parameter_if_estimable(
          population->log_M[i], this->log_M[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->log_M.id_m] = &(population)->log_M;

    for (size_t i = 0; i < log_f_multiplier.size(); i++) {
      population->log_f_multiplier[i] =
          this->log_f_multiplier[i].initial_value_m;
      ss.str("");
      ss << "Population." << this->id << ".log_f_multiplier."
         << this->log_f_multiplier[i].id_m;
      register_parameter_if_estimable(
          population->log_f_multiplier[i],
          this->log_f_multiplier[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->log_f_multiplier.id_m] =
        &(population)->log_f_multiplier;

    for (size_t i = 0; i < log_init_naa.size(); i++) {
      population->log_init_naa[i] = this->log_init_naa[i].initial_value_m;
      ss.str("");
      ss << "Population." << this->id << ".log_init_naa."
         << this->log_init_naa[i].id_m;
      register_parameter_if_estimable(
          population->log_init_naa[i], this->log_init_naa[i].estimation_status_m,
          ss.str());
    }
    info->variable_map[this->log_init_naa.id_m] = &(population)->log_init_naa;

    if (this->proportion_female.size() == 1 ||
        this->proportion_female.size() ==
            static_cast<size_t>(this->n_ages)) {
      population->proportion_female.resize(this->proportion_female.size());
    } else {
      FIMS_WARNING_LOG(
          "The proportion_female vector is not of size 1 or n_ages. Filling "
          "with 0.5.");
      this->proportion_female.resize(1);
      this->proportion_female[0].initial_value_m = static_cast<double>(0.5);
      this->proportion_female[0].estimation_status_m = fims_enum::EstimationStatus::kAssumedKnown;
      population->proportion_female.resize(this->proportion_female.size());
    }
    info->variable_map[this->proportion_female.id_m] =
        &(population)->proportion_female;

    for (size_t i = 0; i < this->proportion_female.size(); i++) {
      if (this->proportion_female[i].initial_value_m < 0.0 ||
          this->proportion_female[i].initial_value_m > 1.0) {
        FIMS_WARNING_LOG(
            "proportion_female should be in [0, 1]; got " +
            fims::to_string(this->proportion_female[i].initial_value_m) +
            " at index " + fims::to_string(i) + ".");
      }
      population->proportion_female[i] =
          this->proportion_female[i].initial_value_m;
      ss.str("");
      ss << "Population." << this->id << ".proportion_female."
         << this->proportion_female[i].id_m;
      register_parameter_if_estimable(
          population->proportion_female[i],
          this->proportion_female[i].estimation_status_m, ss.str());
    }

    for (size_t i = 0; i < ages.size(); i++) {
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
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

#endif
