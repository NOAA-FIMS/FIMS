/**
 * @file catch_at_age.hpp
 * @brief Code to specify the catch-at-age model.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_MODELS_CATCH_AT_AGE_HPP
#define FIMS_MODELS_CATCH_AT_AGE_HPP

#include <set>
#include <regex>

#include "fishery_model_base.hpp"

/* Dictionary block for shared parameter snippet documentations.
 * Referenced in function docs via @snippet{doc} this snippet_id.
  [param_population]
  @param population Shared pointer to the population object.
  [param_population]
  [param_i_age_year]
  @param i_age_year Dimension folded index for age and year.
  [param_i_age_year]
  [param_year]
  @param year Year index.
  [param_year]
  [param_age]
  @param age Age index.
  [param_age]
  [param_i_agem1_yearm1]
  @param i_agem1_yearm1 Dimension folded index for age-1 and year-1.
  [param_i_agem1_yearm1]
  [param_i_dev]
  @param i_dev Index to log_recruit_dev of vector length n_years-1.
  [param_i_dev]
  [param_other]
  @param other The other CatchAtAge object to copy from.
  [param_other]
 */

namespace fims_popdy {

template <typename Type>
/**
 * @brief CatchAtAge is a class containing a catch-at-age model, which is
 * just one of many potential fishery models that can be used in FIMS. The
 * CatchAtAge class inherits from the FisheryModelBase class and can be used
 * to fit both age and length data even though it is called CatchAtAge.
 *
 * See the @ref glossary for definitions of mathematical symbols used below.
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
   * @brief Iterate through fleets.
   *
   */
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
      fleet_iterator;

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
   * @snippet{doc} this param_other
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
      std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];

      population->proportion_female.resize(population->n_ages);

      population->M.resize(population->n_years * population->n_ages);

      population->f_multiplier.resize(population->n_years);

      population->spawning_biomass_ratio.resize(population->n_years + 1);

      // Resize population derived quantities
      population->total_landings_weight.resize(population->n_years);
      population->total_landings_numbers.resize(population->n_years);
      population->mortality_F.resize(population->n_years * population->n_ages);
      population->mortality_M.resize(population->n_years * population->n_ages);
      population->mortality_Z.resize(population->n_years * population->n_ages);
      population->numbers_at_age.resize((population->n_years + 1) *
                                        population->n_ages);
      population->unfished_numbers_at_age.resize((population->n_years + 1) *
                                                 population->n_ages);
      population->biomass.resize(population->n_years + 1);
      population->spawning_biomass.resize(population->n_years + 1);
      population->unfished_biomass.resize(population->n_years + 1);
      population->unfished_spawning_biomass.resize(population->n_years + 1);
      population->proportion_mature_at_age.resize((population->n_years + 1) *
                                                  population->n_ages);
      population->expected_recruitment.resize(population->n_years + 1);
      population->sum_selectivity.resize(population->n_years *
                                         population->n_ages);
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

      // Resize fleet derived quantities
      fleet->landings_numbers_at_age.resize(fleet->n_years * fleet->n_ages);
      fleet->landings_weight_at_age.resize(fleet->n_years * fleet->n_ages);
      fleet->landings_numbers_at_length.resize(fleet->n_years *
                                               fleet->n_lengths);
      fleet->landings_weight.resize(fleet->n_years);
      fleet->landings_numbers.resize(fleet->n_years);
      fleet->landings_expected.resize(fleet->n_years);
      fleet->log_landings_expected.resize(fleet->n_years);
      fleet->agecomp_proportion.resize(fleet->n_years * fleet->n_ages);
      fleet->lengthcomp_proportion.resize(fleet->n_years * fleet->n_lengths);
      fleet->agecomp_expected.resize(fleet->n_years * fleet->n_ages);
      fleet->lengthcomp_expected.resize(fleet->n_years * fleet->n_lengths);
      fleet->index_numbers_at_age.resize(fleet->n_years * fleet->n_ages);
      fleet->index_weight_at_age.resize(fleet->n_years * fleet->n_ages);
      fleet->index_numbers_at_length.resize(fleet->n_years * fleet->n_lengths);
      fleet->index_weight.resize(fleet->n_years);
      fleet->index_numbers.resize(fleet->n_years);
      fleet->index_expected.resize(fleet->n_years);
      fleet->log_index_expected.resize(fleet->n_years);
      fleet->catch_index.resize(fleet->n_years);
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

      // Reset population derived quantities
      this->ResetVector(population->total_landings_weight);
      this->ResetVector(population->total_landings_numbers);
      this->ResetVector(population->mortality_F);
      this->ResetVector(population->mortality_M);
      this->ResetVector(population->mortality_Z);
      this->ResetVector(population->numbers_at_age);
      this->ResetVector(population->unfished_numbers_at_age);
      this->ResetVector(population->biomass);
      this->ResetVector(population->spawning_biomass);
      this->ResetVector(population->unfished_biomass);
      this->ResetVector(population->unfished_spawning_biomass);
      this->ResetVector(population->proportion_mature_at_age);
      this->ResetVector(population->expected_recruitment);
      this->ResetVector(population->sum_selectivity);

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

      for (size_t year = 0; year < population->n_years; year++) {
        population->f_multiplier[year] =
            fims_math::exp(population->log_f_multiplier[year]);
      }
    }

    for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

      // Reset fleet derived quantities
      this->ResetVector(fleet->landings_numbers_at_age);
      this->ResetVector(fleet->landings_weight_at_age);
      this->ResetVector(fleet->landings_numbers_at_length);
      this->ResetVector(fleet->landings_weight);
      this->ResetVector(fleet->landings_numbers);
      this->ResetVector(fleet->landings_expected);
      this->ResetVector(fleet->log_landings_expected);
      this->ResetVector(fleet->agecomp_proportion);
      this->ResetVector(fleet->lengthcomp_proportion);
      this->ResetVector(fleet->agecomp_expected);
      this->ResetVector(fleet->lengthcomp_expected);
      this->ResetVector(fleet->index_numbers_at_age);
      this->ResetVector(fleet->index_weight_at_age);
      this->ResetVector(fleet->index_numbers_at_length);
      this->ResetVector(fleet->index_weight);
      this->ResetVector(fleet->index_numbers);
      this->ResetVector(fleet->index_expected);
      this->ResetVector(fleet->log_index_expected);
      this->ResetVector(fleet->catch_index);

      // Transformation Section
      for (size_t i = 0; i < fleet->log_q.size(); i++) {
        fleet->q[i] = fims_math::exp(fleet->log_q[i]);
      }

      for (size_t year = 0; year < fleet->n_years; year++) {
        fleet->Fmort[year] = fims_math::exp(fleet->log_Fmort[year]);
      }
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
   * @brief Calculates initial numbers at age for index and age.
   *
   * The formula used is:
   * \f[
   * N_{a,0} = \exp(\log N_{a,0})
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_age
   */
  void CalculateInitialNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t age) {
    population->numbers_at_age[i_age_year] =
        fims_math::exp(population->log_init_naa[age]);
  }

  /**
   * @brief Calculates numbers at age for a population.
   *
   * This function calculates numbers at age by applying total mortality
   * \f$Z\f$ to individuals from the previous time step. It also handles
   * the accumulation of a plus group.
   *
   * Standard update:
   * \f[
   * N_{a,y} = N_{a-1,y-1} \exp(-Z_{a-1,y-1})
   * \f]
   *
   * Plus group update (if \f$a = A\f$):
   * \f[
   * N_{A,y} = N_{A-1,y-1} \exp(-Z_{A-1,y-1}) + N_{A,y-1} \exp(-Z_{A,y-1})
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_i_agem1_yearm1
   * @snippet{doc} this param_age
   */
  void CalculateNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t i_agem1_yearm1, size_t age) {
    population->numbers_at_age[i_age_year] =
        population->numbers_at_age[i_agem1_yearm1] *
        (fims_math::exp(-population->mortality_Z[i_agem1_yearm1]));

    // Plus group calculation
    if (age == (population->n_ages - 1)) {
      population->numbers_at_age[i_age_year] =
          population->numbers_at_age[i_age_year] +
          population->numbers_at_age[i_agem1_yearm1 + 1] *
              (fims_math::exp(-population->mortality_Z[i_agem1_yearm1 + 1]));
    }
  }

  /**
   * @brief Calculates unfished numbers at age at year and age specific indices.
   *
   * This function computes unfished numbers at age by applying survival
   * through time using only natural mortality, without any fishing pressure.
   * It also accounts for accumulation of the plus group.
   *
   * Standard update:
   * \f[
   * N^U_{a,y} = N^U_{a-1,y-1} \exp(-M_{a-1,y-1})
   * \f]
   *
   * Plus group update (if \f$a = A\f$):
   * \f[
   * N^U_{A,y} = N^U_{A-1,y-1} \exp(-M_{A-1,y-1}) + N^U_{A,y-1} \exp(-M_{A,y-1})
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_i_agem1_yearm1
   * @snippet{doc} this param_age
   */
  void CalculateUnfishedNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t i_agem1_yearm1, size_t age) {
    population->unfished_numbers_at_age[i_age_year] =
        population->unfished_numbers_at_age[i_agem1_yearm1] *
        (fims_math::exp(-population->M[i_agem1_yearm1]));

    // Plus group calculation
    if (age == (population->n_ages - 1)) {
      population->unfished_numbers_at_age[i_age_year] =
          population->unfished_numbers_at_age[i_age_year] +
          population->unfished_numbers_at_age[i_agem1_yearm1 + 1] *
              (fims_math::exp(-population->M[i_agem1_yearm1 + 1]));
    }
  }

  /**
   * @brief Calculates total mortality for a population.
   *
   * This function calculates total mortality \f$Z\f$ for a specific age and
   * year, combining natural mortality \f$M\f$ and fishing mortality \f$F\f$
   * from all fleets.
   *
   * The fishing mortality \f$F_{f,a,y}\f$ for each fleet \f$f\f$ is computed
   * using age-specific selectivity \f$S_f(a)\f$, fleet-specific annual
   * fishing mortality \f$F_{f,y}\f$, and year-specific F multiplier
   * \f$f_y\f$:
   * \f[
   * F_{f,a,y} = F_{f,y} \times f_y \times S_f(a)
   * \f]
   *
   * Total fishing mortality at age \f$a\f$ and year \f$y\f$ is the sum over
   * fleets:
   * \f[
   * F_{a,y} = \sum_{f=1}^{N_{fleets}} F_{f,a,y}
   * \f]
   *
   * Total mortality \f$Z_{a,y}\f$ is the sum of natural and fishing mortality:
   * \f[
   * Z_{a,y} = M_{a,y} + F_{a,y}
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateMortality(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      Type s = population->fleets[fleet_]->selectivity->evaluate(
          population->ages[age], year);

      population->mortality_F[i_age_year] +=
          population->fleets[fleet_]->Fmort[year] *
          population->f_multiplier[year] * s;

      population->sum_selectivity[i_age_year] += s;
    }
    population->mortality_M[i_age_year] = population->M[i_age_year];

    population->mortality_Z[i_age_year] =
        population->M[i_age_year] + population->mortality_F[i_age_year];
  }

  /**
   * @brief Calculates biomass for a population.
   *
   * Adds the biomass at age to the total biomass for a given year \f$y\f$ by
   * multiplying numbers at age \f$a\f$ by weight at age \f$w_a\f$:
   * \f[
   * B_y \mathrel{+}= N_{a,y} \times w_a
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateBiomass(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    population->biomass[year] +=
        population->numbers_at_age[i_age_year] *
        population->growth->evaluate(year, population->ages[age]);
  }

  /**
   * @brief Calculates the unfished biomass for a population.
   *
   * Updates unfished biomass \f$B^U_y\f$ by adding the biomass of age \f$a\f$
   * in year \f$y\f$:
   * \f[
   * B^U_y \mathrel{+}= N^U_{a,y} \times w_a
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateUnfishedBiomass(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    population->unfished_biomass[year] +=
        population->unfished_numbers_at_age[i_age_year] *
        population->growth->evaluate(year, population->ages[age]);
  }

  /**
   * @brief Calculates spawning biomass for a population.
   *
   * This function computes yearly \f$y\f$ spawning biomass \f$SB_y\f$ by
   * summing the contributions from each age \f$a\f$, accounting for proportion
   * female, proportion mature, and weight at age \f$w_a\f$:
   * \f[
   * SB_y \mathrel{+}= N_{a,y} \times w_a \times p_{female,a} \times
   * p_{mature,a}
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateSpawningBiomass(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    population->spawning_biomass[year] +=
        population->proportion_female[age] *
        population->numbers_at_age[i_age_year] *
        population->proportion_mature_at_age[i_age_year] *
        population->growth->evaluate(year, population->ages[age]);
  }

  /**
   * @brief Calculated unfished spawning biomass for a population
   *
   * Updates unfished spawning biomass \f$SB^U_y\f$ by adding the biomass of age
   * \f$a\f$ in year \f$y\f$:
   * \f[
   * SB^U_y \mathrel{+}= N^U_{a,y} \times w_a \times p_{female,a} \times
   * p_{mature,a}
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateUnfishedSpawningBiomass(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    population->unfished_spawning_biomass[year] +=
        population->proportion_female[age] *
        population->unfished_numbers_at_age[i_age_year] *
        population->proportion_mature_at_age[i_age_year] *
        population->growth->evaluate(year, population->ages[age]);
  }

  /**
   * @brief Calculate the spawning biomass ratio for a population and year.
   *
   * This method computes the ratio of spawning biomass in a given year to the
   * spawning biomass at year zero (typically unfished), for the specified
   * population:
   * \f[
   * \text{ratio}_y = \frac{SB_y}{SB^U_0}
   * \f]
   *
   * The result is stored in the population's spawning_biomass_ratio vector.
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_year
   */
  void CalculateSpawningBiomassRatio(
      std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year) {
    population->spawning_biomass_ratio[year] =
        population->spawning_biomass[year] /
        population->unfished_spawning_biomass[0];
  }

  /**
   * @brief Calculates equilibrium spawning biomass per recruit.
   *
   * This function calculates the spawning biomass per recruit \f$\phi_0\f$ at
   * equilibrium, assuming an unfished stock. The biomass is calculated as the
   * sum of the biomass contributions from each age \f$a\f$:
   * \f[
   * \phi_0 = \sum_{a=0}^{A} N_a \times p_{female,a} \times p_{mature,a} \times
   * w_a
   * \f]
   *
   * The numbers at age \f$N_a\f$ are calculated recursively with natural
   * mortality: \f[ N_a = N_{a-1} \times \exp(-M_a) \quad \text{for } a = 1,
   * \ldots, A-1 \f]
   *
   * Plus group update:
   * \f[
   * N_A = \frac{N_{A-1} \times \exp(-M_{A-1})}{1 - \exp(-M_A)}
   * \f]
   *
   * @snippet{doc} this param_population
   * @return Type
   */
  Type CalculateSBPR0(
      std::shared_ptr<fims_popdy::Population<Type>> &population) {
    std::vector<Type> numbers_spr(population->n_ages, 1.0);
    Type phi_0 = 0.0;
    phi_0 += numbers_spr[0] * population->proportion_female[0] *
             population->proportion_mature_at_age[0] *
             population->growth->evaluate(0, population->ages[0]);
    for (size_t a = 1; a < (population->n_ages - 1); a++) {
      numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-population->M[a]);
      phi_0 += numbers_spr[a] * population->proportion_female[a] *
               population->proportion_mature_at_age[a] *
               population->growth->evaluate(0, population->ages[a]);
    }

    numbers_spr[population->n_ages - 1] =
        (numbers_spr[population->n_ages - 2] *
         fims_math::exp(-population->M[population->n_ages - 2])) /
        (1 - fims_math::exp(-population->M[population->n_ages - 1]));
    phi_0 += numbers_spr[population->n_ages - 1] *
             population->proportion_female[population->n_ages - 1] *
             population->proportion_mature_at_age[population->n_ages - 1] *
             population->growth->evaluate(
                 0, population->ages[population->n_ages - 1]);

    return phi_0;
  }

  /**
   * @brief Calculates expected recruitment for a population.
   *
   * Calculates expected recruitment as a function of spawning biomass and
   * equilibrium spawning biomass per recruit \f$\phi_0\f$.
   *
   * The expected recruitment \f$R_y\f$ in year \f$y\f$ is given by:
   * \f[
   * R_y =
   * \begin{cases}
   * f(SB_{y-1}, \phi_0), & \text{if } i_{dev} = n_{years} \\
   * \exp(g(y-1)), & \text{otherwise}
   * \end{cases}
   * \f]
   *
   * Where \f$f()\f$ evaluates mean recruitment based on spawning biomass and
   * \f$\phi_0\f$, and \f$g(y-1)\f$ evaluates recruitment deviations.
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_i_dev
   */
  void CalculateRecruitment(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t i_dev) {
    Type phi_0 = CalculateSBPR0(population);

    if (i_dev == population->n_years) {
      population->numbers_at_age[i_age_year] =
          population->recruitment->evaluate_mean(
              population->spawning_biomass[year - 1], phi_0);
    } else {
      population->recruitment->log_expected_recruitment[year - 1] =
          fims_math::log(population->recruitment->evaluate_mean(
              population->spawning_biomass[year - 1], phi_0));

      population->numbers_at_age[i_age_year] = fims_math::exp(
          population->recruitment->process->evaluate_process(year - 1));
    }

    population->expected_recruitment[year] =
        population->numbers_at_age[i_age_year];
  }

  /**
   * @brief Calculates maturity at age, in proportion, for a population.
   *
   * This function evaluates the maturity ogive at the specified age to estimate
   * the proportion of individuals that are mature:
   * \f[
   * p_{mature,a} = \text{maturity}(a)
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_age
   */
  void CalculateMaturityAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t age) {
    population->proportion_mature_at_age[i_age_year] =
        population->maturity->evaluate(population->ages[age]);
  }

  /**
   * @brief Calculates total catch (landings) by fleet and population for a
   * given year by aggregating age-specific catch over ages.
   *
   * This function updates fleet-specific and total expected landings for a
   * given year and age by accumulating age-specific catch from each fleet:
   * \f[
   * CW_{f,y} \mathrel{+}= CW_{f,a,y}, \quad
   * C_{f,y} \mathrel{+}= C_{f,a,y}
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateLandings(
      std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
      size_t age) {
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet =
          population->fleets[fleet_];
      size_t i_age_year = year * population->n_ages + age;

      population->total_landings_weight[year] +=
          fleet->landings_weight_at_age[i_age_year];

      fleet->landings_weight[year] += fleet->landings_weight_at_age[i_age_year];

      population->total_landings_numbers[year] +=
          fleet->landings_numbers_at_age[i_age_year];

      fleet->landings_numbers[year] +=
          fleet->landings_numbers_at_age[i_age_year];
    }
  }

  /**
   * @brief Calculates weight at age of the landings for a given fleet from a
   * population.
   *
   * This function computes the expected landings at age in weight by
   * multiplying the expected landings numbers at age by the corresponding
   * weight at age:
   * \f[
   * CW_{f,a,y} = C_{f,a,y} \times w_a
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateLandingsWeightAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
      size_t age) {
    int i_age_year = year * population->n_ages + age;
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet =
          population->fleets[fleet_];
      fleet->landings_weight_at_age[i_age_year] =
          fleet->landings_numbers_at_age[i_age_year] *
          population->growth->evaluate(year, population->ages[age]);
    }
  }

  /**
   * @brief Calculates numbers of fish for the landings for a given fleet from a
   * population, year and age.
   *
   * This function uses the Baranov Catch Equation to calculate expected
   * landings in numbers at age for each fleet. With F multiplier \f$f_y\f$:
   * \f[
   * C_{f,a,y} = \frac{F_{f,y} \times f_y \times S_f(a)}{Z_{a,y}} \times N_{a,y}
   * \times
   * \left( 1 - \exp(-Z_{a,y}) \right)
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateLandingsNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet =
          population->fleets[fleet_];

      // Baranov Catch Equation
      fleet->landings_numbers_at_age[i_age_year] +=
          (fleet->Fmort[year] * population->f_multiplier[year] *
           fleet->selectivity->evaluate(population->ages[age], year)) /
          population->mortality_Z[i_age_year] *
          population->numbers_at_age[i_age_year] *
          (1 - fims_math::exp(-(population->mortality_Z[i_age_year])));
    }
  }

  /**
   * @brief Calculates the index for a fleet from a population.
   *
   * This function updates the population indices for each fleet by adding the
   * age- and year-specific index weights and numbers to the corresponding
   * annual totals. The updated index weight and index numbers for a given
   * fleet and year are calculated as:
   * \f[
   * IW_{f,y} \mathrel{+}= IWAA_{a,y}, \quad
   * IN_{f,y} \mathrel{+}= INAA_{a,y}
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateIndex(std::shared_ptr<fims_popdy::Population<Type>> &population,
                      size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet =
          population->fleets[fleet_];
      fleet->index_weight[year] += fleet->index_weight_at_age[i_age_year];
      fleet->index_numbers[year] += fleet->index_numbers_at_age[i_age_year];
    }
  }

  /**
   * @brief Calculates the numbers for the index for a fleet from a population.
   *
   * This function calculates the expected index in numbers at age for each
   * fleet, using catchability, selectivity, and population numbers at age:
   * \f[
   * IN_{f,a,y} \mathrel{+}= q_{f,y} \times S_f(a) \times N_{a,y}
   * \f]
   *
   * When timing is accounted for within FIMS the equation will include the
   * fraction of the year when the survey was conducted \f$t_y\f$:
   * \f[ IN_{f,a,y} \mathrel{+}= S_{f,y}(a) \times N_{a,y} \times
   * e^{(-t_{y}Z_{a,y})}\f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_i_age_year
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateIndexNumbersAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet =
          population->fleets[fleet_];
      fleet->index_numbers_at_age[i_age_year] +=
          (fleet->q.get_force_scalar(year) *
           fleet->selectivity->evaluate(population->ages[age], year)) *
          population->numbers_at_age[i_age_year];
    }
  }

  /**
   * @brief Calculates biomass of fish for the index for a given fleet from a
   * population.
   *
   * This function computes the expected index weight at age by multiplying the
   * expected index numbers at age by the corresponding weight at age:
   * \f[
   * IWAA_{f,a,y} = IN_{f,a,y} \times w_a
   * \f]
   *
   * @snippet{doc} this param_population
   * @snippet{doc} this param_year
   * @snippet{doc} this param_age
   */
  void CalculateIndexWeightAA(
      std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
      size_t age) {
    int i_age_year = year * population->n_ages + age;
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet =
          population->fleets[fleet_];
      fleet->index_weight_at_age[i_age_year] =
          fleet->index_numbers_at_age[i_age_year] *
          population->growth->evaluate(year, population->ages[age]);
    }
  }

  /**
   * Evaluate the proportion of landings numbers at age.
   */
  void evaluate_age_comp() {
    fleet_iterator fit;
    for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

      for (size_t y = 0; y < fleet->n_years; y++) {
        Type sum = static_cast<Type>(0.0);
        Type sum_obs = static_cast<Type>(0.0);

        for (size_t a = 0; a < fleet->n_ages; a++) {
          size_t i_age_year = y * fleet->n_ages + a;
          if (fleet->fleet_observed_landings_data_id_m == -999) {
            fleet->agecomp_expected[i_age_year] =
                fleet->index_numbers_at_age[i_age_year];
          } else {
            fleet->agecomp_expected[i_age_year] =
                fleet->landings_numbers_at_age[i_age_year];
          }
          sum += fleet->agecomp_expected[i_age_year];

          if (fleet->fleet_observed_agecomp_data_id_m != -999) {
            if (fleet->observed_agecomp_data->at(i_age_year) !=
                fleet->observed_agecomp_data->na_value) {
              sum_obs += fleet->observed_agecomp_data->at(i_age_year);
            }
          }
        }
        for (size_t a = 0; a < fleet->n_ages; a++) {
          size_t i_age_year = y * fleet->n_ages + a;
          fleet->agecomp_proportion[i_age_year] =
              fleet->agecomp_expected[i_age_year] / sum;

          if (fleet->fleet_observed_agecomp_data_id_m != -999) {
            fleet->agecomp_expected[i_age_year] =
                fleet->agecomp_proportion[i_age_year] * sum_obs;
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
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

      if (fleet->n_lengths > 0) {
        for (size_t y = 0; y < fleet->n_years; y++) {
          Type sum = static_cast<Type>(0.0);
          Type sum_obs = static_cast<Type>(0.0);

          for (size_t l = 0; l < fleet->n_lengths; l++) {
            size_t i_length_year = y * fleet->n_lengths + l;
            for (size_t a = 0; a < fleet->n_ages; a++) {
              size_t i_age_year = y * fleet->n_ages + a;
              size_t i_length_age = a * fleet->n_lengths + l;
              fleet->lengthcomp_expected[i_length_year] +=
                  fleet->agecomp_expected[i_age_year] *
                  fleet->age_to_length_conversion[i_length_age];

              fleet->landings_numbers_at_length[i_length_year] +=
                  fleet->landings_numbers_at_age[i_age_year] *
                  fleet->age_to_length_conversion[i_length_age];

              fleet->index_numbers_at_length[i_length_year] +=
                  fleet->index_numbers_at_age[i_age_year] *
                  fleet->age_to_length_conversion[i_length_age];
            }

            sum += fleet->lengthcomp_expected[i_length_year];

            if (fleet->fleet_observed_lengthcomp_data_id_m != -999) {
              if (fleet->observed_lengthcomp_data->at(i_length_year) !=
                  fleet->observed_lengthcomp_data->na_value) {
                sum_obs += fleet->observed_lengthcomp_data->at(i_length_year);
              }
            }
          }
          for (size_t l = 0; l < fleet->n_lengths; l++) {
            size_t i_length_year = y * fleet->n_lengths + l;
            fleet->lengthcomp_proportion[i_length_year] =
                fleet->lengthcomp_expected[i_length_year] / sum;
            if (fleet->fleet_observed_lengthcomp_data_id_m != -999) {
              fleet->lengthcomp_expected[i_length_year] =
                  fleet->lengthcomp_proportion[i_length_year] * sum_obs;
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
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

      for (size_t i = 0; i < fleet->index_numbers.size(); i++) {
        if (fleet->observed_index_units == "number") {
          fleet->index_expected[i] = fleet->index_numbers[i];
        } else {
          fleet->index_expected[i] = fleet->index_weight[i];
        }
        fleet->log_index_expected[i] = log(fleet->index_expected[i]);
      }
    }
  }

  /**
   * Evaluate the natural log of the expected landings.
   */
  void evaluate_landings() {
    fleet_iterator fit;
    for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

      for (size_t i = 0; i < fleet->landings_weight.size(); i++) {
        if (fleet->observed_landings_units == "number") {
          fleet->landings_expected[i] = fleet->landings_numbers[i];
        } else {
          fleet->landings_expected[i] = fleet->landings_weight[i];
        }
        fleet->log_landings_expected[i] = log(fleet->landings_expected[i]);
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
            CalculateMortality(population, i_age_year, y, a);
          }
          CalculateMaturityAA(population, i_age_year, a);
          if (y == 0) {
            CalculateInitialNumbersAA(population, i_age_year, a);

            if (a == 0) {
              population->expected_recruitment[y] =
                  population->numbers_at_age[i_age_year];
              population->unfished_numbers_at_age[i_age_year] =
                  fims_math::exp(population->recruitment->log_rzero[0]);
            } else {
              CalculateUnfishedNumbersAA(population, i_age_year, a - 1, a);
            }

          } else {
            if (a == 0) {
              CalculateRecruitment(population, i_age_year, y, y);
              population->unfished_numbers_at_age[i_age_year] =
                  fims_math::exp(population->recruitment->log_rzero[0]);
            } else {
              size_t i_agem1_yearm1 = (y - 1) * population->n_ages + (a - 1);
              CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, a);
              CalculateUnfishedNumbersAA(population, i_age_year, i_agem1_yearm1,
                                         a);
            }
          }

          CalculateBiomass(population, i_age_year, y, a);
          CalculateUnfishedBiomass(population, i_age_year, y, a);
          CalculateSpawningBiomass(population, i_age_year, y, a);
          CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);

          if (y < population->n_years) {
            CalculateLandingsNumbersAA(population, i_age_year, y, a);
            CalculateLandingsWeightAA(population, y, a);
            CalculateLandings(population, y, a);

            CalculateIndexNumbersAA(population, i_age_year, y, a);
            CalculateIndexWeightAA(population, y, a);
            CalculateIndex(population, i_age_year, y, a);
          }
        }
        /* Calculate spawning biomass depletion ratio */
        CalculateSpawningBiomassRatio(population, y);
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
      // initialize population vectors
      vector<vector<Type>> biomass_p(n_pops);
      vector<vector<Type>> expected_recruitment_p(n_pops);
      vector<vector<Type>> mortality_F_p(n_pops);
      vector<vector<Type>> mortality_M_p(n_pops);
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
      vector<vector<Type>> spawning_biomass_ratio_p(n_pops);

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

      int pop_idx = 0;
      for (size_t p = 0; p < this->populations.size(); p++) {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
            this->populations[p];
        biomass_p(pop_idx) = population->biomass.to_tmb();
        expected_recruitment_p(pop_idx) =
            population->expected_recruitment.to_tmb();
        mortality_F_p(pop_idx) = population->mortality_F.to_tmb();
        mortality_M_p(pop_idx) = population->mortality_M.to_tmb();
        mortality_Z_p(pop_idx) = population->mortality_Z.to_tmb();
        numbers_at_age_p(pop_idx) = population->numbers_at_age.to_tmb();
        proportion_mature_at_age_p(pop_idx) =
            population->proportion_mature_at_age.to_tmb();
        spawning_biomass_p(pop_idx) = population->spawning_biomass.to_tmb();
        sum_selectivity_p(pop_idx) = population->sum_selectivity.to_tmb();
        total_landings_numbers_p(pop_idx) =
            population->total_landings_numbers.to_tmb();
        total_landings_weight_p(pop_idx) =
            population->total_landings_weight.to_tmb();
        unfished_biomass_p(pop_idx) = population->unfished_biomass.to_tmb();
        unfished_numbers_at_age_p(pop_idx) =
            population->unfished_numbers_at_age.to_tmb();
        unfished_spawning_biomass_p(pop_idx) =
            population->unfished_spawning_biomass.to_tmb();
        spawning_biomass_ratio_p(pop_idx) =
            population->spawning_biomass_ratio.to_tmb();
        pop_idx += 1;
      }

      int fleet_idx = 0;
      fleet_iterator fit;
      for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

        agecomp_expected_f(fleet_idx) = fleet->agecomp_expected.to_tmb();
        agecomp_proportion_f(fleet_idx) = fleet->agecomp_proportion.to_tmb();
        catch_index_f(fleet_idx) = fleet->catch_index.to_tmb();
        index_expected_f(fleet_idx) = fleet->index_expected.to_tmb();
        index_numbers_f(fleet_idx) = fleet->index_numbers.to_tmb();
        index_numbers_at_age_f(fleet_idx) =
            fleet->index_numbers_at_age.to_tmb();
        index_numbers_at_length_f(fleet_idx) =
            fleet->index_numbers_at_length.to_tmb();
        index_weight_f(fleet_idx) = fleet->index_weight.to_tmb();
        index_weight_at_age_f(fleet_idx) = fleet->index_weight_at_age.to_tmb();
        landings_expected_f(fleet_idx) = fleet->landings_expected.to_tmb();
        landings_numbers_f(fleet_idx) = fleet->landings_numbers.to_tmb();
        landings_numbers_at_age_f(fleet_idx) =
            fleet->landings_numbers_at_age.to_tmb();
        landings_numbers_at_length_f(fleet_idx) =
            fleet->landings_numbers_at_length.to_tmb();
        landings_weight_f(fleet_idx) = fleet->landings_weight.to_tmb();
        landings_weight_at_age_f(fleet_idx) =
            fleet->landings_weight_at_age.to_tmb();
        lengthcomp_expected_f(fleet_idx) = fleet->lengthcomp_expected.to_tmb();
        lengthcomp_proportion_f(fleet_idx) =
            fleet->lengthcomp_proportion.to_tmb();
        log_index_expected_f(fleet_idx) = fleet->log_index_expected.to_tmb();
        log_landings_expected_f(fleet_idx) =
            fleet->log_landings_expected.to_tmb();
        fleet_idx += 1;
      }

      vector<Type> biomass = ADREPORTvector(biomass_p);
      vector<Type> expected_recruitment =
          ADREPORTvector(expected_recruitment_p);
      vector<Type> mortality_F = ADREPORTvector(mortality_F_p);
      vector<Type> mortality_M = ADREPORTvector(mortality_M_p);
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
      vector<Type> spawning_biomass_ratio =
          ADREPORTvector(spawning_biomass_ratio_p);

      vector<Type> agecomp_expected = ADREPORTvector(agecomp_expected_f);
      vector<Type> agecomp_proportion = ADREPORTvector(agecomp_proportion_f);
      vector<Type> catch_index = ADREPORTvector(catch_index_f);
      vector<Type> index_expected = ADREPORTvector(index_expected_f);
      vector<Type> index_numbers = ADREPORTvector(index_numbers_f);
      vector<Type> index_numbers_at_age =
          ADREPORTvector(index_numbers_at_age_f);
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
      vector<Type> lengthcomp_expected = ADREPORTvector(lengthcomp_expected_f);
      vector<Type> lengthcomp_proportion =
          ADREPORTvector(lengthcomp_proportion_f);
      vector<Type> log_index_expected = ADREPORTvector(log_index_expected_f);
      vector<Type> log_landings_expected =
          ADREPORTvector(log_landings_expected_f);

      // populations — report
      FIMS_REPORT_F_("biomass", biomass_p, this->of);
      FIMS_REPORT_F_("expected_recruitment", expected_recruitment_p, this->of);
      FIMS_REPORT_F_("mortality_F", mortality_F_p, this->of);
      FIMS_REPORT_F_("mortality_M", mortality_M_p, this->of);
      FIMS_REPORT_F_("mortality_Z", mortality_Z_p, this->of);
      FIMS_REPORT_F_("numbers_at_age", numbers_at_age_p, this->of);
      FIMS_REPORT_F_("proportion_mature_at_age", proportion_mature_at_age_p,
                     this->of);
      FIMS_REPORT_F_("spawning_biomass", spawning_biomass_p, this->of);
      FIMS_REPORT_F_("sum_selectivity", sum_selectivity_p, this->of);
      FIMS_REPORT_F_("total_landings_numbers", total_landings_numbers_p,
                     this->of);
      FIMS_REPORT_F_("total_landings_weight", total_landings_weight_p,
                     this->of);
      FIMS_REPORT_F_("unfished_biomass", unfished_biomass_p, this->of);
      FIMS_REPORT_F_("unfished_numbers_at_age", unfished_numbers_at_age_p,
                     this->of);
      FIMS_REPORT_F_("unfished_spawning_biomass", unfished_spawning_biomass_p,
                     this->of);
      FIMS_REPORT_F_("spawning_biomass_ratio", spawning_biomass_ratio_p,
                     this->of);

      // populations — adreport
      ADREPORT_F(biomass, this->of);
      ADREPORT_F(expected_recruitment, this->of);
      ADREPORT_F(mortality_F, this->of);
      ADREPORT_F(mortality_M, this->of);
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
      ADREPORT_F(spawning_biomass_ratio, this->of);

      // fleets — report
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
      FIMS_REPORT_F_("lengthcomp_proportion", lengthcomp_proportion_f,
                     this->of);
      FIMS_REPORT_F_("log_index_expected", log_index_expected_f, this->of);
      FIMS_REPORT_F_("log_landings_expected", log_landings_expected_f,
                     this->of);

      // fleets — adreport
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
      for (rvit = report_vectors.begin(); rvit != report_vectors.end();
           ++rvit) {
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

}  // namespace fims_popdy

#endif
