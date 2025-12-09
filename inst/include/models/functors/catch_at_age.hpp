/**
 * @file catch_at_age.hpp
 * @brief Code to specify the catch-at-age model.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_MODELS_CATCH_AT_AGE_HPP
#define FIMS_MODELS_CATCH_AT_AGE_HPP

#include <regex>
#include <set>

#include "fishery_model_base.hpp"

namespace fims_popdy {

template <typename Type>
/**
 * @brief CatchAtAge is a class containing a catch-at-age model, which is
 * just one of many potential fishery models that can be used in FIMS. The
 * CatchAtAge class inherits from the FisheryModelBase class and can be used
 * to fit both age and length data even though it is called CatchAtAge.
 *
 */
class CatchAtAge : public FisheryModelBase<Type> {
public:
  /**
   * @brief The name of the model.
   *
   */
  std::string name_m;

  /**
   * @brief Iterate the derived quantities.
   *
   */
  typedef typename std::map<std::string, fims::Vector<Type>>::iterator
      derived_quantities_iterator;

  /**
   * @brief Used to iterate through fleet-based derived quantities.
   *
   */
  typedef typename std::map<uint32_t,
                            std::map<std::string, fims::Vector<Type>>>::iterator
      fleet_derived_quantities_iterator;

  /**
   * @brief Used to iterate through fleet-based derived quantities dimensions.
   */
  typedef
      typename std::map<uint32_t,
                        std::map<std::string, fims::Vector<size_t>>>::iterator
          fleet_derived_quantities_dims_iterator;
  /**
   * @brief Used to iterate through population-based derived quantities.
   *
   */
  typedef typename std::map<uint32_t,
                            std::map<std::string, fims::Vector<Type>>>::iterator
      population_derived_quantities_iterator;

  /**
   * @brief Used to iterate through population-based derived quantities
   * dimensions.
   */
  typedef
      typename std::map<uint32_t,
                        std::map<std::string, fims::Vector<size_t>>>::iterator
          population_derived_quantities_dims_iterator;

  /**
   * @brief Iterate through fleets.
   *
   */
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
      fleet_iterator;
  /**
   * @brief Iterate through derived quantities.
   *
   */
  typedef
      typename std::map<std::string, fims::Vector<Type>>::iterator dq_iterator;
  /**
   * @brief A map of report vectors for the object.
   * used to populate the report_vectors map in for submodule
   * parameters.
   */
  std::map<std::string, fims::Vector<fims::Vector<Type>>> report_vectors;

public:
  std::vector<Type> ages; /*!< vector of the ages for referencing*/
  /**
   * Constructor for the CatchAtAge class. This constructor initializes the
   * name of the model and sets the id of the model.
   */
  CatchAtAge() : FisheryModelBase<Type>() {
    std::stringstream ss;
    ss << "caa_" << this->GetId() << "_";
    this->name_m = ss.str();
    this->model_type_m = "caa";
  }

  /**
   * @brief Copy constructor for the CatchAtAge class.
   *
   * @param other The other CatchAtAge object to copy from.
   */
  CatchAtAge(const CatchAtAge &other)
      : FisheryModelBase<Type>(other), name_m(other.name_m), ages(other.ages) {
    this->model_type_m = "caa";
  }

  /**
   * @brief Destroy the Catch At Age object.
   *
   */
  virtual ~CatchAtAge() {}

  /**
   * This function is called once at the beginning of the model run. It
   * initializes the derived quantities for the populations and fleets.
   */
  virtual void Initialize() {
    for (size_t p = 0; p < this->populations.size(); p++) {
      this->populations[p]->proportion_female.resize(
          this->populations[p]->n_ages);

      this->populations[p]->M.resize(this->populations[p]->n_years *
                                     this->populations[p]->n_ages);
    }

    for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

      if (fleet->log_q.size() == 0) {
        fleet->log_q.resize(1);
        fleet->log_q[0] = static_cast<Type>(0.0);
      }
      fleet->q.resize(fleet->log_q.size());
      fleet->Fmort.resize(fleet->n_years);
      fleet->selectivity_at_age.resize(fleet->n_ages);
      fleet->selectivity_at_length.resize(fleet->n_lengths);
    }
  }

  /**
   * This function is used to reset the derived quantities of a population or
   * fleet to a given value.
   */
  virtual void Prepare() {
    for (size_t p = 0; p < this->populations.size(); p++) {
      std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];

      auto &derived_quantities =
          this->GetPopulationDerivedQuantities(population->GetId());

      // Reset the derived quantities for the population
      for (auto &kv : derived_quantities) {
        this->ResetVector(kv.second);
      }

      // Prepare proportion_female
      for (size_t age = 0; age < population->n_ages; age++) {
        population->proportion_female[age] = 0.5;
      }

      // Transformation Section
      for (size_t age = 0; age < population->n_ages; age++) {
        for (size_t year = 0; year < population->n_years; year++) {
          size_t i_age_year = age * population->n_years + year;
          population->M[i_age_year] =
              fims_math::exp(population->log_M[i_age_year]);
        }
      }
    }

    for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      auto &derived_quantities =
          this->GetFleetDerivedQuantities(fleet->GetId());

      for (auto &kv : derived_quantities) {
        this->ResetVector(kv.second);
      }

      // Transformation Section
      for (size_t i = 0; i < fleet->log_q.size(); i++) {
        fleet->q[i] = fims_math::exp(fleet->log_q[i]);
      }

      for (size_t year = 0; year < fleet->n_years; year++) {
        fleet->Fmort[year] = fims_math::exp(fleet->log_Fmort[year]);
      }
      // // TODO: does this age_length_to_conversion need to be a dq and
      // parameter
      // // of fleet?
      // for (size_t i_length_age = 0;
      //      i_length_age < fleet->age_to_length_conversion.size();
      //      i_length_age++)
      // {
      //   derived_quantities["age_to_length_conversion"][i_length_age] =
      //       fleet->age_to_length_conversion[i_length_age];
      // }
    }
  }
  /**
   * This function is used to add a population id to the set of population ids.
   */
  void AddPopulation(uint32_t id) { this->population_ids.insert(id); }

  /**
   * @brief Get the population ids of the model.
   */
  std::set<uint32_t> &GetPopulationIds() { return this->population_ids; }

  /**
   * This function is used to get the populations of the model. It returns a
   * vector of shared pointers to the populations.
   * @return std::vector<std::shared_ptr<fims_popdy::Population<Type>>>&
   */
  std::vector<std::shared_ptr<fims_popdy::Population<Type>>> &GetPopulations() {
    return this->populations;
  }

  /**
   * This method is used to calculate the initial numbers at age for a
   * population. It takes a population object and an age as input and
   * calculates the initial numbers at age for that population.
   * @param population
   * @param i_age_year
   * @param a
   */
  void CalculateInitialNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t a) {
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["numbers_at_age"][i_age_year] =
        fims_math::exp(population->log_init_naa[a]);
  }

  /**
   * * This method is used to calculate the numbers at age for a
   * population. It takes a population object, the index of the age
   * in the current year, the index of the age in the previous year,
   * and the age as input and calculates the numbers at age for that
   * population.
   * @param population
   * @param i_age_year
   * @param i_agem1_yearm1
   * @param age
   */
  void
  CalculateNumbersAA(std::shared_ptr<fims_popdy::Population<Type>> &population,
                     size_t i_age_year, size_t i_agem1_yearm1, size_t age) {
    // using Z from previous age/year

    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["numbers_at_age"][i_age_year] =
        dq_["numbers_at_age"][i_agem1_yearm1] *
        (fims_math::exp(-dq_["mortality_Z"][i_agem1_yearm1]));

    // Plus group calculation
    if (age == (population->n_ages - 1)) {
      dq_["numbers_at_age"][i_age_year] =
          dq_["numbers_at_age"][i_age_year] +
          dq_["numbers_at_age"][i_agem1_yearm1 + 1] *
              (fims_math::exp(-dq_["mortality_Z"][i_agem1_yearm1 + 1]));
    }
  }

  /**
   * This method is used to calculate the unfished numbers at age for a
   * population. It takes a population object, the index of the age
   * in the current year, the index of the age in the previous year,
   * and the age as input and calculates the unfished numbers at age
   * for that population.
   * @param population
   * @param i_age_year
   * @param i_agem1_yearm1
   * @param age
   */
  void CalculateUnfishedNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t i_agem1_yearm1, size_t age) {
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    // using M from previous age/year
    dq_["unfished_numbers_at_age"][i_age_year] =
        dq_["unfished_numbers_at_age"][i_agem1_yearm1] *
        (fims_math::exp(-population->M[i_agem1_yearm1]));

    // Plus group calculation
    if (age == (population->n_ages - 1)) {
      dq_["unfished_numbers_at_age"][i_age_year] =
          dq_["unfished_numbers_at_age"][i_age_year] +
          dq_["unfished_numbers_at_age"][i_agem1_yearm1 + 1] *
              (fims_math::exp(-population->M[i_agem1_yearm1 + 1]));
    }
  }

  /**
   * * This method is used to calculate the mortality for a population. It takes
   * a population object, the index of the age in the current year, the year,
   * and the age as input and calculates the mortality for that population.
   * @param population
   * @param i_age_year
   * @param year
   * @param age
   */
  void
  CalculateMortality(std::shared_ptr<fims_popdy::Population<Type>> &population,
                     size_t i_age_year, size_t year, size_t age) {
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      // evaluate is a member function of the selectivity class
      Type s;
      if (population->fleets[fleet_]->selectivity_units == "age")
        s = population->fleets[fleet_]->selectivity->evaluate(
            population->ages[age]);
      else if (population->fleets[fleet_]->selectivity_units == "length")
        s = population->fleets[fleet_]->selectivity->evaluate(
            population->growth->evaluate(population->ages[age]));
    }

    dq_["mortality_F"][i_age_year] +=
        population->fleets[fleet_]->Fmort[year] * s;

    dq_["sum_selectivity"][i_age_year] += s;
  }
  dq_["mortality_Z"][i_age_year] =
      population->M[i_age_year] + dq_["mortality_F"][i_age_year];
}

  /**
   * * This method is used to calculate the biomass for a population. It takes a
   * population object, the index of the age in the current year, the year,
   * and the age as input and calculates the biomass for that population.
   * @param population
   * @param i_age_year
   * @param year
   * @param age
   */
  void CalculateBiomass(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  dq_["biomass"][year] += dq_["numbers_at_age"][i_age_year] *
                          population->growth->evaluate(population->ages[age]);
}

/**
 * * This method is used to calculate the unfished biomass for a population.
 * It takes a population object, the index of the age in the current year, the
 * year, and the age as input and calculates the unfished biomass for that
 * population.
 * @param population
 * @param i_age_year
 * @param year
 * @param age
 */
void CalculateUnfishedBiomass(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  dq_["unfished_biomass"][year] +=
      dq_["unfished_numbers_at_age"][i_age_year] *
      population->growth->evaluate(population->ages[age]);
}

/**
 * * This method is used to calculate the spawning biomass for a population.
 * It takes a population object, the index of the age in the current year, the
 * year, and the age as input and calculates the spawning biomass for that
 * population.
 * @param population
 * @param i_age_year
 * @param year
 * @param age
 */
void CalculateSpawningBiomass(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  dq_["spawning_biomass"][year] +=
      population->proportion_female[age] * dq_["numbers_at_age"][i_age_year] *
      dq_["proportion_mature_at_age"][i_age_year] *
      population->growth->evaluate(population->ages[age]);
}

/**
 * This method is used to calculate the unfished spawning biomass for a
 * population. It takes a population object, the index of the age in the
 * current year, the year, and the age as input and calculates the unfished
 * spawning biomass for that population.
 * @param population
 * @param i_age_year
 * @param year
 * @param age
 */
void CalculateUnfishedSpawningBiomass(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  dq_["unfished_spawning_biomass"][year] +=
      population->proportion_female[age] *
      dq_["unfished_numbers_at_age"][i_age_year] *
      dq_["proportion_mature_at_age"][i_age_year] *
      population->growth->evaluate(population->ages[age]);
}

/**
 * This method is used to calculate the spawning biomass per recruit for a
 * population. It takes a population object.
 */
Type CalculateSBPR0(std::shared_ptr<fims_popdy::Population<Type>> &population) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  std::vector<Type> numbers_spr(population->n_ages, 1.0);
  Type phi_0 = 0.0;
  phi_0 += numbers_spr[0] * population->proportion_female[0] *
           dq_["proportion_mature_at_age"][0] *
           population->growth->evaluate(population->ages[0]);
  for (size_t a = 1; a < (population->n_ages - 1); a++) {
    numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-population->M[a]);
    phi_0 += numbers_spr[a] * population->proportion_female[a] *
             dq_["proportion_mature_at_age"][a] *
             population->growth->evaluate(population->ages[a]);
  }

  numbers_spr[population->n_ages - 1] =
      (numbers_spr[population->n_ages - 2] *
       fims_math::exp(-population->M[population->n_ages - 2])) /
      (1 - fims_math::exp(-population->M[population->n_ages - 1]));
  phi_0 +=
      numbers_spr[population->n_ages - 1] *
      population->proportion_female[population->n_ages - 1] *
      dq_["proportion_mature_at_age"][population->n_ages - 1] *
      population->growth->evaluate(population->ages[population->n_ages - 1]);

  return phi_0;
}

/**
 * This method is used to calculate the recruitment for a population.
 *
 */
void CalculateRecruitment(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t i_dev) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  Type phi0 = CalculateSBPR0(population);

  if (i_dev == population->n_years) {
    dq_["numbers_at_age"][i_age_year] = population->recruitment->evaluate_mean(
        dq_["spawning_biomass"][year - 1], phi0);
    /*the final year of the time series has no data to inform recruitment
    devs, so this value is set to the mean recruitment.*/
  } else {
    // Why are we using evaluate_mean, how come a virtual function was
    // changed? AMH: there are now two virtual functions: evaluate_mean and
    // evaluate_process (see below)
    population->recruitment->log_expected_recruitment[year - 1] =
        fims_math::log(population->recruitment->evaluate_mean(
            dq_["spawning_biomass"][year - 1], phi0));

    dq_["numbers_at_age"][i_age_year] = fims_math::exp(
        population->recruitment->process->evaluate_process(year - 1));
  }

  dq_["expected_recruitment"][year] = dq_["numbers_at_age"][i_age_year];
}

/**
 * This method is used to calculate the maturity at age for a population. It
 * takes a population object, the index of the age in the current year, the
 * age as input and calculates the maturity at age for that population.
 * @param population
 * @param i_age_year
 * @param age
 */
void CalculateMaturityAA(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &dq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  dq_["proportion_mature_at_age"][i_age_year] =
      population->maturity->evaluate(population->ages[age]);
}

/**
 * This method is used to calculate the landings for a population and adds to
 * existing expected total landings by fleet. It takes a population object,
 * the year, and the age as input and calculates the landings for that
 * population.
 * @param population
 * @param year
 * @param age
 */
void CalculateLandings(
    std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
    size_t age) {
  std::map<std::string, fims::Vector<Type>> &pdq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
    size_t i_age_year = year * population->n_ages + age;

    pdq_["total_landings_weight"][year] +=
        fdq_["landings_weight_at_age"][i_age_year];

    fdq_["landings_weight"][year] += fdq_["landings_weight_at_age"][i_age_year];

    pdq_["total_landings_numbers"][year] +=
        fdq_["landings_numbers_at_age"][i_age_year];

    fdq_["landings_numbers"][year] +=
        fdq_["landings_numbers_at_age"][i_age_year];
  }
}

/**
 * This method is used to calculate the catch weight at age for a population.
 * It takes a population object, the index of the age in the current year, the
 * year, and the age as input and calculates the weight at age for that
 * population.
 * @param population
 * @param year
 * @param age
 */
void CalculateLandingsWeightAA(
    std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
    size_t age) {
  int i_age_year = year * population->n_ages + age;
  for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

    fdq_["landings_weight_at_age"][i_age_year] =
        fdq_["landings_numbers_at_age"][i_age_year] *
        population->growth->evaluate(population->ages[age]);
  }
}

/**
 * @brief Calculate the numbers at age for landings in a population.
 *
 * @param population The population.
 * @param i_age_year The index of the age and year.
 * @param year The year.
 * @param age The age.
 */
void CalculateLandingsNumbersAA(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &pdq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

    // Baranov Catch Equation
    fdq_["landings_numbers_at_age"][i_age_year] +=
        (population->fleets[fleet_]->Fmort[year] *
         population->fleets[fleet_]->selectivity->evaluate(
             population->ages[age])) /
        pdq_["mortality_Z"][i_age_year] * pdq_["numbers_at_age"][i_age_year] *
        (1 - fims_math::exp(-(pdq_["mortality_Z"][i_age_year])));
  }
}

/**
 * @brief Calculates landings in numbers at length for each fleet for a given
 * year and length, then adds the value to the expected landings in numbers at
 * length for each fleet
 *
 * @param i_age_year dimension folded index for age and year
 * @param year the year of expected landings composition is being calculated for
 * @param age the age composition is being calculated for
 */
void CalculateLandingsNumbersAL(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
 for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
 }
}

/**
 * @brief Calculate the index for a population.
 *
 * @param population The population.
 * @param i_age_year The index of the year and age.
 * @param year The year.
 * @param age The age.
 */
void CalculateIndex(std::shared_ptr<fims_popdy::Population<Type>> &population,
                    size_t i_age_year, size_t year, size_t age) {
  for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

    fdq_["index_weight"][year] += fdq_["index_weight_at_age"][i_age_year];

    fdq_["index_numbers"][year] += fdq_["index_numbers_at_age"][i_age_year];
  }
}

/**
 * @brief Calculate the numbers at age for an index in the population.
 *
 * @param population The population.
 * @param i_age_year The index of the year and age.
 * @param year The year.
 * @param age The age.
 */
void CalculateIndexNumbersAA(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
  std::map<std::string, fims::Vector<Type>> &pdq_ =
      this->GetPopulationDerivedQuantities(population->GetId());

  for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

    fdq_["index_numbers_at_age"][i_age_year] +=
        (population->fleets[fleet_]->q.get_force_scalar(year) *
         population->fleets[fleet_]->selectivity->evaluate(
             population->ages[age])) *
        pdq_["numbers_at_age"][i_age_year];
  }
}

/**
 * @brief Calculates index sample in numbers at length for each fleet for
 * a given year and length, then adds the value to the expected index in
 * numbers at length for each fleet
 *
 * @param i_age_year dimension folded index for age and year
 * @param year the year the expected index is being calculated for
 * @param age the age index is being calculated for
 */
void CalculateIndexNumbersAL(
    std::shared_ptr<fims_popdy::Population<Type>> &population,
    size_t i_age_year, size_t year, size_t age) {
      for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
  }
  // Implementation for length-based index numbers at age
  // This is a placeholder for the actual implementation
}

/**
 * @brief Calculate the weight at age for an index in a population.
 *
 * @param population The population.
 * @param year The year.
 * @param age The age.
 */
void CalculateIndexWeightAA(
    std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
    size_t age) {
  int i_age_year = year * population->n_ages + age;
  for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

    fdq_["index_weight_at_age"][i_age_year] =
        fdq_["index_numbers_at_age"][i_age_year] *
        population->growth->evaluate(population->ages[age]);
  }
}

/**
 * Evaluate the proportion of landings numbers at age.
 */
void evaluate_age_comp() {
  fleet_iterator fit;
  for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities((*fit).second->GetId());

    std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
    for (size_t y = 0; y < fleet->n_years; y++) {
      Type sum = static_cast<Type>(0.0);
      Type sum_obs = static_cast<Type>(0.0);
      // robust_add is a small value to add to expected composition
      // proportions at age to stabilize likelihood calculations
      // when the expected proportions are close to zero.
      // Type robust_add = static_cast<Type>(0.0); // zeroed out before
      // testing 0.0001; sum robust is used to calculate the total sum of
      // robust additions to ensure that proportions sum to 1. Type robust_sum
      // = static_cast<Type>(1.0);

      for (size_t a = 0; a < fleet->n_ages; a++) {
        size_t i_age_year = y * fleet->n_ages + a;
        // Here we have a check to determine if the age comp
        // should be calculated from the retained landings or
        // the total population. These values are slightly different.
        // In the future this will have more impact as we implement
        // timing rather than everything occurring at the start of
        // the year.
        if (fleet->fleet_observed_landings_data_id_m == -999) {
          fdq_["agecomp_expected"][i_age_year] =
              fdq_["index_numbers_at_age"][i_age_year];
        } else {
          fdq_["agecomp_expected"][i_age_year] =
              fdq_["landings_numbers_at_age"][i_age_year];
        }
        sum += fdq_["agecomp_expected"][i_age_year];
        // robust_sum -= robust_add;

        // This sums over the observed age composition data so that
        // the expected age composition can be rescaled to match the
        // total number observed. The check for na values should not
        // be needed as individual years should not have missing data.
        // This is need to be re-explored if/when we modify FIMS to
        // allow for composition bins that do not match the population
        // bins.
        if (fleet->fleet_observed_agecomp_data_id_m != -999) {
          if (fleet->observed_agecomp_data->at(i_age_year) !=
              fleet->observed_agecomp_data->na_value) {
            sum_obs += fleet->observed_agecomp_data->at(i_age_year);
          }
        }
      }
      for (size_t a = 0; a < fleet->n_ages; a++) {
        size_t i_age_year = y * fleet->n_ages + a;
        fdq_["agecomp_proportion"][i_age_year] =
            fdq_["agecomp_expected"][i_age_year] / sum;
        // robust_add + robust_sum * this->agecomp_expected[i_age_year] / sum;

        if (fleet->fleet_observed_agecomp_data_id_m != -999) {
          fdq_["agecomp_expected"][i_age_year] =
              fdq_["agecomp_proportion"][i_age_year] * sum_obs;
        }
      }
    }
  }
}

/**
 * Evaluate the proportion of landings numbers at length.
 */
void evaluate_length_comp() {
  fleet_iterator fit;
  for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities((*fit).second->GetId());

    std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

    if (fleet->n_lengths > 0) {
      for (size_t y = 0; y < fleet->n_years; y++) {
        Type sum = static_cast<Type>(0.0);
        Type sum_obs = static_cast<Type>(0.0);
        // robust_add is a small value to add to expected composition
        // proportions at age to stabilize likelihood calculations
        // when the expected proportions are close to zero.
        // Type robust_add = static_cast<Type>(0.0); // 0.0001; zeroed out
        // before testing sum robust is used to calculate the total sum of
        // robust additions to ensure that proportions sum to 1. Type
        // robust_sum = static_cast<Type>(1.0);
        for (size_t l = 0; l < fleet->n_lengths; l++) {
          size_t i_length_year = y * fleet->n_lengths + l;
          for (size_t a = 0; a < fleet->n_ages; a++) {
            size_t i_age_year = y * fleet->n_ages + a;
            size_t i_length_age = a * fleet->n_lengths + l;
            fdq_["lengthcomp_expected"][i_length_year] +=
                fdq_["agecomp_expected"][i_age_year] *
                fleet->age_to_length_conversion[i_length_age];

            fdq_["landings_numbers_at_length"][i_length_year] +=
                fdq_["landings_numbers_at_age"][i_age_year] *
                fleet->age_to_length_conversion[i_length_age];

            fdq_["index_numbers_at_length"][i_length_year] +=
                fdq_["index_numbers_at_age"][i_age_year] *
                fleet->age_to_length_conversion[i_length_age];
          }

          sum += fdq_["lengthcomp_expected"][i_length_year];
          // robust_sum -= robust_add;

          if (fleet->fleet_observed_lengthcomp_data_id_m != -999) {
            if (fleet->observed_lengthcomp_data->at(i_length_year) !=
                fleet->observed_lengthcomp_data->na_value) {
              sum_obs += fleet->observed_lengthcomp_data->at(i_length_year);
            }
          }
        }
        for (size_t l = 0; l < fleet->n_lengths; l++) {
          size_t i_length_year = y * fleet->n_lengths + l;
          fdq_["lengthcomp_proportion"][i_length_year] =
              fdq_["lengthcomp_expected"][i_length_year] / sum;
          // robust_add + robust_sum *
          // this->lengthcomp_expected[i_length_year] / sum;
          if (fleet->fleet_observed_lengthcomp_data_id_m != -999) {
            fdq_["lengthcomp_expected"][i_length_year] =
                fdq_["lengthcomp_proportion"][i_length_year] * sum_obs;
          }
        }
      }
    }
  }
}

/**
 * Evaluate the natural log of the expected index.
 */
void evaluate_index() {
  fleet_iterator fit;
  for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities((*fit).second->GetId());
    std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

    for (size_t i = 0; i < fdq_["index_numbers"].size(); i++) {
      if (fleet->observed_index_units == "number") {
        fdq_["index_expected"][i] = fdq_["index_numbers"][i];
      } else {
        fdq_["index_expected"][i] = fdq_["index_weight"][i];
      }
      fdq_["log_index_expected"][i] = log(fdq_["index_expected"][i]);
    }
  }
}

/**
 * Evaluate the natural log of the expected landings.
 */
void evaluate_landings() {
  fleet_iterator fit;
  for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities((*fit).second->GetId());
    std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

    for (size_t i = 0; i < fdq_["landings_weight"].size(); i++) {
      if (fleet->observed_landings_units == "number") {
        fdq_["landings_expected"][i] = fdq_["landings_numbers"][i];
      } else {
        fdq_["landings_expected"][i] = fdq_["landings_weight"][i];
      }
      fdq_["log_landings_expected"][i] = log(fdq_["landings_expected"][i]);
    }
  }
}

virtual void Evaluate() {
  /*
             Sets derived vectors to zero
             Performs parameters transformations
             Sets recruitment deviations to mean 0.
   */
  Prepare();
  /*
   start at year=0, age=0;
   here year 0 is the estimated initial population structure and age 0 are
   recruits loops start at zero with if statements inside to specify unique
   code for initial structure and recruitment 0 loops. Could also have started
   loops at 1 with initial structure and recruitment setup outside the loops.

   year loop is extended to <= n_years because SSB is calculated as the start
   of the year value and by extending one extra year we get estimates of the
   population structure at the end of the final year. An alternative approach
   would be to keep initial numbers at age in it's own vector and each year to
   include the population structure at the end of the year. This is likely a
   null point given that we are planning to modify to an event/stanza based
   structure in later milestones which will eliminate this confusion by
   explicitly referencing the exact date (or period of averaging) at which any
   calculation or output is being made.
   */
  for (size_t p = 0; p < this->populations.size(); p++) {
    std::shared_ptr<fims_popdy::Population<Type>> &population =
        this->populations[p];
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());
    // CAAPopulationProxy<Type>& population = this->populations_proxies[p];

    for (size_t y = 0; y <= population->n_years; y++) {
      for (size_t a = 0; a < population->n_ages; a++) {
        /*
         index naming defines the dimensional folding structure
         i.e. i_age_year is referencing folding over years and ages.
         */
        size_t i_age_year = y * population->n_ages + a;
        /*
         Mortality rates are not estimated in the final year which is
         used to show expected population structure at the end of the model
         period. This is because biomass in year i represents biomass at the
         start of the year. Should we add complexity to track more values such
         as start, mid, and end biomass in all years where, start biomass=end
         biomass of the previous year? Referenced above, this is probably not
         worth exploring as later milestone changes will eliminate this
         confusion.
         */
        if (y < population->n_years) {
          /*
           First thing we need is total mortality aggregated across all fleets
           to inform the subsequent catch and change in numbers at age
           calculations. This is only calculated for years < n_years as these
           are the model estimated years with data. The year loop extends to
           y=n_years so that population numbers at age and SSB can be
           calculated at the end of the last year of the model
           */
          CalculateMortality(population, i_age_year, y, a);
        }
        CalculateMaturityAA(population, i_age_year, a);
        /* if statements needed because some quantities are only needed
        for the first year and/or age, so these steps are included here.
         */
        if (y == 0) {
          // Initial numbers at age is a user input or estimated parameter
          // vector.
          CalculateInitialNumbersAA(population, i_age_year, a);

          if (a == 0) {
            pdq_["unfished_numbers_at_age"][i_age_year] =
                fims_math::exp(population->recruitment->log_rzero[0]);
          } else {
            CalculateUnfishedNumbersAA(population, i_age_year, a - 1, a);
          }

          /*
           Fished and unfished biomass vectors are summing biomass at
           age across ages.
           */

          CalculateBiomass(population, i_age_year, y, a);

          CalculateUnfishedBiomass(population, i_age_year, y, a);

          /*
           Fished and unfished spawning biomass vectors are summing biomass at
           age across ages to allow calculation of recruitment in the next
           year.
           */

          CalculateSpawningBiomass(population, i_age_year, y, a);

          CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);

          /*
           Expected recruitment in year 0 is numbers at age 0 in year 0.
           */

          pdq_["expected_recruitment"][i_age_year] =
              pdq_["numbers_at_age"][i_age_year];
        } else {
          if (a == 0) {
            // Set the nrecruits for age a=0 year y (use pointers instead of
            // functional returns) assuming fecundity = 1 and 50:50 sex ratio
            CalculateRecruitment(population, i_age_year, y, y);
            pdq_["unfished_numbers_at_age"][i_age_year] =
                fims_math::exp(population->recruitment->log_rzero[0]);
          } else {
            size_t i_agem1_yearm1 = (y - 1) * population->n_ages + (a - 1);
            CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, a);
            CalculateUnfishedNumbersAA(population, i_age_year, i_agem1_yearm1,
                                       a);
          }
          CalculateBiomass(population, i_age_year, y, a);
          CalculateSpawningBiomass(population, i_age_year, y, a);

          CalculateUnfishedBiomass(population, i_age_year, y, a);
          CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);
        }

        /*
        Here composition, total catch, and index values are calculated for all
        years with reference data. They are not calculated for y=n_years as
        there is this is just to get final population structure at the end of
        the terminal year.
         */
        if (y < population->n_years) {
          CalculateLandingsNumbersAA(population, i_age_year, y, a);
          CalculateLandingsWeightAA(population, y, a);
          CalculateLandings(population, y, a);

          CalculateIndexNumbersAA(population, i_age_year, y, a);
          CalculateIndexWeightAA(population, y, a);
          CalculateIndex(population, i_age_year, y, a);
        }
      }
    }
  }
  evaluate_age_comp();
  evaluate_length_comp();
  evaluate_index();
  evaluate_landings();
}
/**
 * * This method is used to generate TMB reports from the population dynamics
 * model.
 */
virtual void Report() {
  int n_fleets = this->fleets.size();
  int n_pops = this->populations.size();
#ifdef TMB_MODEL
  if (this->do_reporting == true) {
    report_vectors.clear();
    // std::shared_ptr<UncertaintyReportInfoMap>
    // population_uncertainty_report_info_map =
    //     this->GetPopulationUncertaintyReportInfoMap();

    // std::shared_ptr<UncertaintyReportInfoMap>
    // fleet_uncertainty_report_info_map =
    //     this->GetFleetUncertaintyReportInfoMap();

    // initialize population vectors
    vector<vector<Type>> biomass_p(n_pops);
    vector<vector<Type>> expected_recruitment_p(n_pops);
    vector<vector<Type>> mortality_F_p(n_pops);
    vector<vector<Type>> mortality_Z_p(n_pops);
    vector<vector<Type>> numbers_at_age_p(n_pops);
    vector<vector<Type>> proportion_mature_at_age_p(n_pops);
    vector<vector<Type>> spawning_biomass_p(n_pops);
    vector<vector<Type>> sum_selectivity_p(n_pops);
    vector<vector<Type>> total_landings_numbers_p(n_pops);
    vector<vector<Type>> total_landings_weight_p(n_pops);
    vector<vector<Type>> unfished_biomass_p(n_pops);
    vector<vector<Type>> unfished_numbers_at_age_p(n_pops);
    vector<vector<Type>> unfished_spawning_biomass_p(n_pops);
    vector<vector<Type>> log_M_p(n_pops);
    vector<vector<Type>> log_init_naa_p(n_pops);

    // initialize fleet vectors
    vector<vector<Type>> agecomp_expected_f(n_fleets);
    vector<vector<Type>> agecomp_proportion_f(n_fleets);
    vector<vector<Type>> catch_index_f(n_fleets);
    vector<vector<Type>> index_expected_f(n_fleets);
    vector<vector<Type>> index_numbers_f(n_fleets);
    vector<vector<Type>> index_numbers_at_age_f(n_fleets);
    vector<vector<Type>> index_numbers_at_length_f(n_fleets);
    vector<vector<Type>> index_weight_f(n_fleets);
    vector<vector<Type>> index_weight_at_age_f(n_fleets);
    vector<vector<Type>> landings_expected_f(n_fleets);
    vector<vector<Type>> landings_numbers_f(n_fleets);
    vector<vector<Type>> landings_numbers_at_age_f(n_fleets);
    vector<vector<Type>> landings_numbers_at_length_f(n_fleets);
    vector<vector<Type>> landings_weight_f(n_fleets);
    vector<vector<Type>> landings_weight_at_age_f(n_fleets);
    vector<vector<Type>> lengthcomp_expected_f(n_fleets);
    vector<vector<Type>> lengthcomp_proportion_f(n_fleets);
    vector<vector<Type>> log_index_expected_f(n_fleets);
    vector<vector<Type>> log_landings_expected_f(n_fleets);

    // initiate population index for structuring report out objects
    int pop_idx = 0;
    for (size_t p = 0; p < this->populations.size(); p++) {
      this->populations[p]->create_report_vectors(report_vectors);
      // std::shared_ptr<fims_popdy::Population<Type>> &population =
      //     this->populations[p];
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          this->GetPopulationDerivedQuantities(this->populations[p]->GetId());
      this->populations[p]->maturity->create_report_vectors(report_vectors);
      this->populations[p]->growth->create_report_vectors(report_vectors);
      this->populations[p]->recruitment->create_report_vectors(report_vectors);
      biomass_p(pop_idx) = derived_quantities["biomass"].to_tmb();
      expected_recruitment_p(pop_idx) =
          derived_quantities["expected_recruitment"].to_tmb();
      mortality_F_p(pop_idx) = derived_quantities["mortality_F"].to_tmb();
      mortality_Z_p(pop_idx) = derived_quantities["mortality_Z"].to_tmb();
      numbers_at_age_p(pop_idx) = derived_quantities["numbers_at_age"].to_tmb();
      proportion_mature_at_age_p(pop_idx) =
          derived_quantities["proportion_mature_at_age"].to_tmb();
      spawning_biomass_p(pop_idx) =
          derived_quantities["spawning_biomass"].to_tmb();
      sum_selectivity_p(pop_idx) =
          derived_quantities["sum_selectivity"].to_tmb();
      total_landings_numbers_p(pop_idx) =
          derived_quantities["total_landings_numbers"].to_tmb();
      total_landings_weight_p(pop_idx) =
          derived_quantities["total_landings_weight"].to_tmb();
      unfished_biomass_p(pop_idx) =
          derived_quantities["unfished_biomass"].to_tmb();
      unfished_numbers_at_age_p(pop_idx) =
          derived_quantities["unfished_numbers_at_age"].to_tmb();
      unfished_spawning_biomass_p(pop_idx) =
          derived_quantities["unfished_spawning_biomass"].to_tmb();
      log_M_p(pop_idx) = this->populations[pop_idx]->log_M.to_tmb();
      log_init_naa_p(pop_idx) =
          this->populations[pop_idx]->log_init_naa.to_tmb();

      pop_idx += 1;
    }

    // initiate fleet index for structuring report out objects
    int fleet_idx = 0;
    fleet_iterator fit;
    for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      fleet->create_report_vectors(report_vectors);
      fleet->selectivity->create_report_vectors(report_vectors);
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          this->GetFleetDerivedQuantities(fleet->GetId());

      agecomp_expected_f(fleet_idx) =
          derived_quantities["agecomp_expected"].to_tmb();
      agecomp_proportion_f(fleet_idx) =
          derived_quantities["agecomp_proportion"].to_tmb();
      catch_index_f(fleet_idx) = derived_quantities["catch_index"].to_tmb();
      index_expected_f(fleet_idx) =
          derived_quantities["index_expected"].to_tmb();
      index_numbers_f(fleet_idx) = derived_quantities["index_numbers"].to_tmb();
      index_numbers_at_age_f(fleet_idx) =
          derived_quantities["index_numbers_at_age"].to_tmb();
      index_numbers_at_length_f(fleet_idx) =
          derived_quantities["index_numbers_at_length"].to_tmb();
      index_weight_f(fleet_idx) = derived_quantities["index_weight"].to_tmb();
      index_weight_at_age_f(fleet_idx) =
          derived_quantities["index_weight_at_age"].to_tmb();
      landings_expected_f(fleet_idx) =
          derived_quantities["landings_expected"].to_tmb();
      landings_numbers_f(fleet_idx) =
          derived_quantities["landings_numbers"].to_tmb();
      landings_numbers_at_age_f(fleet_idx) =
          derived_quantities["landings_numbers_at_age"].to_tmb();
      landings_numbers_at_length_f(fleet_idx) =
          derived_quantities["landings_numbers_at_length"].to_tmb();
      landings_weight_f(fleet_idx) =
          derived_quantities["landings_weight"].to_tmb();
      landings_weight_at_age_f(fleet_idx) =
          derived_quantities["landings_weight_at_age"].to_tmb();
      // length_comp_expected_f(fleet_idx) =
      // derived_quantities["length_comp_expected"];
      // length_comp_proportion_f(fleet_idx) =
      // derived_quantities["length_comp_proportion"];
      lengthcomp_expected_f(fleet_idx) =
          derived_quantities["lengthcomp_expected"].to_tmb();
      lengthcomp_proportion_f(fleet_idx) =
          derived_quantities["lengthcomp_proportion"].to_tmb();
      log_index_expected_f(fleet_idx) =
          derived_quantities["log_index_expected"].to_tmb();
      log_landings_expected_f(fleet_idx) =
          derived_quantities["log_landings_expected"].to_tmb();
      fleet_idx += 1;
    }

    vector<Type> biomass = ADREPORTvector(biomass_p);
    vector<Type> expected_recruitment = ADREPORTvector(expected_recruitment_p);
    vector<Type> mortality_F = ADREPORTvector(mortality_F_p);
    vector<Type> mortality_Z = ADREPORTvector(mortality_Z_p);
    vector<Type> numbers_at_age = ADREPORTvector(numbers_at_age_p);
    vector<Type> proportion_mature_at_age =
        ADREPORTvector(proportion_mature_at_age_p);
    vector<Type> spawning_biomass = ADREPORTvector(spawning_biomass_p);
    vector<Type> sum_selectivity = ADREPORTvector(sum_selectivity_p);
    vector<Type> total_landings_numbers =
        ADREPORTvector(total_landings_numbers_p);
    vector<Type> total_landings_weight =
        ADREPORTvector(total_landings_weight_p);
    vector<Type> unfished_biomass = ADREPORTvector(unfished_biomass_p);
    vector<Type> unfished_numbers_at_age =
        ADREPORTvector(unfished_numbers_at_age_p);
    vector<Type> unfished_spawning_biomass =
        ADREPORTvector(unfished_spawning_biomass_p);

    vector<Type> agecomp_expected = ADREPORTvector(agecomp_expected_f);
    vector<Type> agecomp_proportion = ADREPORTvector(agecomp_proportion_f);
    vector<Type> catch_index = ADREPORTvector(catch_index_f);
    vector<Type> index_expected = ADREPORTvector(index_expected_f);
    vector<Type> index_numbers = ADREPORTvector(index_numbers_f);
    vector<Type> index_numbers_at_age = ADREPORTvector(index_numbers_at_age_f);
    vector<Type> index_numbers_at_length =
        ADREPORTvector(index_numbers_at_length_f);
    vector<Type> index_weight = ADREPORTvector(index_weight_f);
    vector<Type> index_weight_at_age = ADREPORTvector(index_weight_at_age_f);
    vector<Type> landings_expected = ADREPORTvector(landings_expected_f);
    vector<Type> landings_numbers = ADREPORTvector(landings_numbers_f);
    vector<Type> landings_numbers_at_age =
        ADREPORTvector(landings_numbers_at_age_f);
    vector<Type> landings_numbers_at_length =
        ADREPORTvector(landings_numbers_at_length_f);
    vector<Type> landings_weight = ADREPORTvector(landings_weight_f);
    vector<Type> landings_weight_at_age =
        ADREPORTvector(landings_weight_at_age_f);
    // vector<Type> length_comp_expected =
    // ADREPORTvector(length_comp_expected_f); vector<Type>
    // length_comp_proportion = ADREPORTvector(length_comp_proportion_f);
    vector<Type> lengthcomp_expected = ADREPORTvector(lengthcomp_expected_f);
    vector<Type> lengthcomp_proportion =
        ADREPORTvector(lengthcomp_proportion_f);
    vector<Type> log_index_expected = ADREPORTvector(log_index_expected_f);
    vector<Type> log_landings_expected =
        ADREPORTvector(log_landings_expected_f);
    // populations
    // report
    FIMS_REPORT_F_("biomass", biomass_p, this->of);
    FIMS_REPORT_F_("expected_recruitment", expected_recruitment_p, this->of);
    FIMS_REPORT_F_("mortality_F", mortality_F_p, this->of);
    FIMS_REPORT_F_("mortality_Z", mortality_Z_p, this->of);
    FIMS_REPORT_F_("numbers_at_age", numbers_at_age_p, this->of);
    FIMS_REPORT_F_("proportion_mature_at_age", proportion_mature_at_age_p,
                   this->of);
    FIMS_REPORT_F_("spawning_biomass", spawning_biomass_p, this->of);
    FIMS_REPORT_F_("sum_selectivity", sum_selectivity_p, this->of);
    FIMS_REPORT_F_("total_landings_numbers", total_landings_numbers_p,
                   this->of);
    FIMS_REPORT_F_("total_landings_weight", total_landings_weight_p, this->of);
    FIMS_REPORT_F_("unfished_biomass", unfished_biomass_p, this->of);
    FIMS_REPORT_F_("unfished_numbers_at_age", unfished_numbers_at_age_p,
                   this->of);
    FIMS_REPORT_F_("unfished_spawning_biomass", unfished_spawning_biomass_p,
                   this->of);
    FIMS_REPORT_F_("log_M", log_M_p, this->of);
    FIMS_REPORT_F_("log_init_naa", log_init_naa_p, this->of);

    // adreport
    ADREPORT_F(biomass, this->of);
    ADREPORT_F(expected_recruitment, this->of);
    ADREPORT_F(mortality_F, this->of);
    ADREPORT_F(mortality_Z, this->of);
    ADREPORT_F(numbers_at_age, this->of);
    ADREPORT_F(proportion_mature_at_age, this->of);
    ADREPORT_F(spawning_biomass, this->of);
    ADREPORT_F(sum_selectivity, this->of);
    ADREPORT_F(total_landings_numbers, this->of);
    ADREPORT_F(total_landings_weight, this->of);
    ADREPORT_F(unfished_biomass, this->of);
    ADREPORT_F(unfished_numbers_at_age, this->of);
    ADREPORT_F(unfished_spawning_biomass, this->of);

    // fleets
    // report
    FIMS_REPORT_F_("agecomp_expected", agecomp_expected_f, this->of);
    FIMS_REPORT_F_("agecomp_proportion", agecomp_proportion_f, this->of);
    FIMS_REPORT_F_("catch_index", catch_index_f, this->of);
    FIMS_REPORT_F_("index_expected", index_expected_f, this->of);
    FIMS_REPORT_F_("index_numbers", index_numbers_f, this->of);
    FIMS_REPORT_F_("index_numbers_at_age", index_numbers_at_age_f, this->of);
    FIMS_REPORT_F_("index_numbers_at_length", index_numbers_at_length_f,
                   this->of);
    FIMS_REPORT_F_("index_weight", index_weight_f, this->of);
    FIMS_REPORT_F_("index_weight_at_age", index_weight_at_age_f, this->of);
    FIMS_REPORT_F_("landings_expected", landings_expected_f, this->of);
    FIMS_REPORT_F_("landings_numbers", landings_numbers_f, this->of);
    FIMS_REPORT_F_("landings_numbers_at_age", landings_numbers_at_age_f,
                   this->of);
    FIMS_REPORT_F_("landings_numbers_at_length", landings_numbers_at_length_f,
                   this->of);
    FIMS_REPORT_F_("landings_weight", landings_weight_f, this->of);
    FIMS_REPORT_F_("landings_weight_at_age", landings_weight_at_age_f,
                   this->of);
    FIMS_REPORT_F_("lengthcomp_expected", lengthcomp_expected_f, this->of);
    FIMS_REPORT_F_("lengthcomp_proportion", lengthcomp_proportion_f, this->of);
    FIMS_REPORT_F_("log_index_expected", log_index_expected_f, this->of);
    FIMS_REPORT_F_("log_landings_expected", log_landings_expected_f, this->of);
    // adreport
    ADREPORT_F(agecomp_expected, this->of);
    ADREPORT_F(agecomp_proportion, this->of);
    ADREPORT_F(catch_index, this->of);
    ADREPORT_F(index_expected, this->of);
    ADREPORT_F(index_numbers, this->of);
    ADREPORT_F(index_numbers_at_age, this->of);
    ADREPORT_F(index_numbers_at_length, this->of);
    ADREPORT_F(index_weight, this->of);
    ADREPORT_F(index_weight_at_age, this->of);
    ADREPORT_F(landings_expected, this->of);
    ADREPORT_F(landings_numbers, this->of);
    ADREPORT_F(landings_numbers_at_age, this->of);
    ADREPORT_F(landings_numbers_at_length, this->of);
    ADREPORT_F(landings_weight, this->of);
    ADREPORT_F(landings_weight_at_age, this->of);
    ADREPORT_F(lengthcomp_expected, this->of);
    ADREPORT_F(lengthcomp_proportion, this->of);
    ADREPORT_F(log_index_expected, this->of);
    ADREPORT_F(log_landings_expected, this->of);
    std::stringstream var_name;
    typename std::map<std::string, fims::Vector<fims::Vector<Type>>>::iterator
        rvit;
    for (rvit = report_vectors.begin(); rvit != report_vectors.end(); ++rvit) {
      auto &x = rvit->second;

      int outer_dim = x.size();
      int dim = 0;
      for (int i = 0; i < outer_dim; i++) {
        dim += x[i].size();
      }
      vector<Type> res(dim);
      int idx = 0;
      for (int i = 0; i < outer_dim; i++) {
        int inner_dim = x[i].size();
        for (int j = 0; j < inner_dim; j++) {
          res(idx) = x[i][j];
          idx += 1;
        }
      }
      this->of->reportvector.push(res, rvit->first.c_str());
    }
  }
#endif
}
};

} // namespace fims_popdy

#endif
