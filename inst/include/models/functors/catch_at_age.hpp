/**
 * @file catch_at_age.hpp
 * @brief Code to specify the catch-at-age model.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_MODELS_CATCH_AT_AGE_HPP
#define FIMS_MODELS_CATCH_AT_AGE_HPP

#include <algorithm>
#include <cmath>
#include <set>
#include <regex>
#include <stdexcept>

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
  /**
   * @brief Controls whether reporting materializes the full derived
   * age-to-length tensor for fleets using the growth-derived ALK path.
   *
   * Default is false to avoid large report-side allocations.
   */
  bool report_growth_derived_alk_tensor = false;

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
      : FisheryModelBase<Type>(other),
        name_m(other.name_m),
        ages(other.ages),
        report_growth_derived_alk_tensor(
            other.report_growth_derived_alk_tensor) {
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

      this->populations[p]->f_multiplier.resize(this->populations[p]->n_years);

      this->populations[p]->spawning_biomass_ratio.resize(
          (this->populations[p]->n_years + 1));
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

      for (size_t year = 0; year < population->n_years; year++) {
        population->f_multiplier[year] =
            fims_math::exp(population->log_f_multiplier[year]);
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["numbers_at_age"][i_age_year] =
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      // evaluate is a member function of the selectivity class
      Type s = population->fleets[fleet_]->selectivity->evaluate(
          population->ages[age]);

      dq_["mortality_F"][i_age_year] +=
          population->fleets[fleet_]->Fmort[year] *
          population->f_multiplier[year] * s;

      dq_["sum_selectivity"][i_age_year] += s;
    }
    dq_["mortality_M"][i_age_year] = population->M[i_age_year];

    dq_["mortality_Z"][i_age_year] =
        population->M[i_age_year] + dq_["mortality_F"][i_age_year];
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["biomass"][year] +=
        dq_["numbers_at_age"][i_age_year] *
        population->growth->evaluate(population->ages[age]);
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["unfished_biomass"][year] +=
        dq_["unfished_numbers_at_age"][i_age_year] *
        population->growth->evaluate(population->ages[age]);
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["spawning_biomass"][year] +=
        population->proportion_female[age] * dq_["numbers_at_age"][i_age_year] *
        dq_["proportion_mature_at_age"][i_age_year] *
        population->growth->evaluate(population->ages[age]);
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["unfished_spawning_biomass"][year] +=
        population->proportion_female[age] *
        dq_["unfished_numbers_at_age"][i_age_year] *
        dq_["proportion_mature_at_age"][i_age_year] *
        population->growth->evaluate(population->ages[age]);
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());
    population->spawning_biomass_ratio[year] =
        dq_["spawning_biomass"][year] / dq_["unfished_spawning_biomass"][0];
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    Type phi0 = CalculateSBPR0(population);

    if (i_dev == population->n_years) {
      dq_["numbers_at_age"][i_age_year] =
          population->recruitment->evaluate_mean(
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
    std::map<std::string, fims::Vector<Type>> &dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    dq_["proportion_mature_at_age"][i_age_year] =
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
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::map<std::string, fims::Vector<Type>> &fdq_ =
          this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
      size_t i_age_year = year * population->n_ages + age;

      pdq_["total_landings_weight"][year] +=
          fdq_["landings_weight_at_age"][i_age_year];

      fdq_["landings_weight"][year] +=
          fdq_["landings_weight_at_age"][i_age_year];

      pdq_["total_landings_numbers"][year] +=
          fdq_["landings_numbers_at_age"][i_age_year];

      fdq_["landings_numbers"][year] +=
          fdq_["landings_numbers_at_age"][i_age_year];
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
      std::map<std::string, fims::Vector<Type>> &fdq_ =
          this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

      fdq_["landings_weight_at_age"][i_age_year] =
          fdq_["landings_numbers_at_age"][i_age_year] *
          population->growth->evaluate(population->ages[age]);
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
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::map<std::string, fims::Vector<Type>> &fdq_ =
          this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

      // Baranov Catch Equation
      fdq_["landings_numbers_at_age"][i_age_year] +=
          (population->fleets[fleet_]->Fmort[year] *
           population->f_multiplier[year] *
           population->fleets[fleet_]->selectivity->evaluate(
               population->ages[age])) /
          pdq_["mortality_Z"][i_age_year] * pdq_["numbers_at_age"][i_age_year] *
          (1 - fims_math::exp(-(pdq_["mortality_Z"][i_age_year])));
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
      std::map<std::string, fims::Vector<Type>> &fdq_ =
          this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

      fdq_["index_weight"][year] += fdq_["index_weight_at_age"][i_age_year];

      fdq_["index_numbers"][year] += fdq_["index_numbers_at_age"][i_age_year];
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

  // --- Growth-derived ALK helpers for temp VonB path ---
  //
  // Previous FIMS behavior used a precomputed fleet age_to_length_conversion
  // matrix for length compositions and generic growth->evaluate(age) for
  // weight-at-age. The helpers below were added so that, when VonB is active
  // and supported, catch_at_age can instead:
  // 1. build a dynamic ALK from fitted growth products and fleet length bins,
  // 2. compute mean WAA from that same ALK row and the fleet's bin centers,
  // 3. fall back to the historical path when the supported VonB path is
  //    unavailable.

  /**
   * @brief Returns the midpoint between two adjacent fleet length bins.
   *
   * @param fleet Shared pointer to the fleet object.
   * @param left Index of the left length bin center.
   * @param right Index of the right length bin center.
   * @return Midpoint between the two bin centers.
   */
  inline Type LengthBinMidpoint(const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
                                size_t left,
                                size_t right) const {
    return (fleet->lengths[left] + fleet->lengths[right]) *
           static_cast<Type>(0.5);
  }

#ifdef TMB_MODEL
  /**
   * @brief Evaluates the normal cumulative distribution function.
   *
   * Uses TMB's atomic normal CDF implementation in model code and a standard
   * erf-based fallback outside the TMB path.
   *
   * @param x Value at which to evaluate the CDF.
   * @param mean Mean of the normal distribution.
   * @param sd Standard deviation of the normal distribution.
   * @return Lower-tail normal CDF evaluated at x.
   */
  inline Type NormalCdf(const Type& x, const Type& mean, const Type& sd) const {
    CppAD::vector<Type> tx(1);
    tx[0] = (x - mean) / sd;
    return atomic::pnorm1(tx)[0];
  }
#else
  /**
   * @brief Evaluates the normal cumulative distribution function.
   *
   * @param x Value at which to evaluate the CDF.
   * @param mean Mean of the normal distribution.
   * @param sd Standard deviation of the normal distribution.
   * @return Lower-tail normal CDF evaluated at x.
   */
  inline Type NormalCdf(const Type& x, const Type& mean, const Type& sd) const {
    const double z = static_cast<double>(x - mean) /
                     (static_cast<double>(sd) * std::sqrt(2.0));
    return static_cast<Type>(0.5 * (1.0 + std::erf(z)));
  }
#endif

  /**
   * @brief Builds an age-to-length probability for one length bin from growth
   * products and fleet length bins.
   *
   * The probability is computed from the normal CDF implied by mean and
   * standard deviation of length-at-age for the given year and age.
   *
   * The first and last bins use one-sided tails; interior bins use the
   * difference between upper and lower CDF bounds.
   * 
   * @param fleet Shared pointer to the fleet object.
   * @param growth_products Growth products for the linked population.
   * @param year Year index.
   * @param age Age index.
   * @param length_bin Length-bin index.
   * @return Probability of observing the age in the requested length bin.
   */
  inline Type GrowthDerivedALKProb(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      const fims_popdy::GrowthProducts<Type>& growth_products,
      size_t year,
      size_t age,
      size_t length_bin) const {
    if (fleet->n_lengths == 0) {
      return static_cast<Type>(0.0);
    }
    if (fleet->n_lengths == 1) {
      return static_cast<Type>(1.0);
    }

    const std::size_t y =
        (growth_products.n_years == 0)
            ? 0
            : std::min(year, growth_products.n_years - 1);
    const Type mean_laa = growth_products.MeanLAA(y, age, 0);
    const Type sd_laa = fims_math::ad_max(
        growth_products.SdLAA(y, age, 0), static_cast<Type>(1e-8));

    if (length_bin == 0) {
      const Type upper = LengthBinMidpoint(fleet, 0, 1);
      return NormalCdf(upper, mean_laa, sd_laa);
    }

    if (length_bin + 1 == fleet->n_lengths) {
      const Type lower =
          LengthBinMidpoint(fleet, fleet->n_lengths - 2, fleet->n_lengths - 1);
      return static_cast<Type>(1.0) - NormalCdf(lower, mean_laa, sd_laa);
    }

    const Type lower = LengthBinMidpoint(fleet, length_bin - 1, length_bin);
    const Type upper = LengthBinMidpoint(fleet, length_bin, length_bin + 1);
    return NormalCdf(upper, mean_laa, sd_laa) -
           NormalCdf(lower, mean_laa, sd_laa);
  }

  /**
   * @brief Builds a normalized growth-derived ALK row for one year and age.
   *
   * This helper turns the per-bin probabilities above into a full ALK row and
   * normalizes it so the row sums to one before it is used in likelihood or
   * derived quantities. Normalization was added to guard against small numeric
   * drift from the CDF calculations.
   *
   * @param fleet Shared pointer to the fleet object.
   * @param growth_products Growth products for the linked population.
   * @param year Year index.
   * @param age Age index.
   * @return Normalized age-to-length probabilities for the requested year and
   * age.
   */
  inline  fims::Vector<Type>  BuildNormalizedGrowthDerivedALKRow(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      const fims_popdy::GrowthProducts<Type>& growth_products,
      size_t year,
      size_t age) const {
     fims::Vector<Type>  alk_row(fleet->n_lengths);
    Type row_sum = static_cast<Type>(0.0);
    for (size_t l = 0; l < fleet->n_lengths; ++l) {
      alk_row[l] = GrowthDerivedALKProb(fleet, growth_products, year, age, l);
      row_sum += alk_row[l];
    }
    const Type safe_row_sum =
        fims_math::ad_max(row_sum, static_cast<Type>(1e-12));
    for (size_t l = 0; l < fleet->n_lengths; ++l) {
      alk_row[l] /= safe_row_sum;
    }
    return alk_row;
  }

  /**
   * @brief Computes expected weight-at-age from a normalized ALK row and fleet
   * length-bin centers.
   * 
   * This is the bin-based WAA step built to accomdate growth dervied ALK.
   * Instead of using only growth->evaluate(age), it computes expected weight as
   * the weighted average of weight-at-length over the same fleet bins used in
   * the dynamic ALK row.
   *
   * @param growth_observation Shared pointer to the linked growth-derived
   * observation capability.
   * @param fleet Shared pointer to the fleet object.
   * @param alk_row Normalized age-to-length probabilities for one year and age.
   * @return Expected weight-at-age on the natural scale.
   */
  Type MeanWeightFromALKRow(
      const std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>&
          growth_observation,
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      const  fims::Vector<Type> & alk_row) {
    Type mean_weight = static_cast<Type>(0.0);
    for (size_t l = 0; l < fleet->n_lengths; ++l) {
      mean_weight +=
          alk_row[l] * growth_observation->EvaluateWeightAtLength(fleet->lengths[l]);
    }
    return mean_weight;
  }

  /**
   * @brief Builds a normalized dynamic ALK row and returns the matching
   * midpoint-based expected weight-at-age.
   * 
   * This bundles the ALK and WAA calculations together so both quantities are
   * derived from the same row and cannot drift apart across callers.
   *
   * @param growth_observation Shared pointer to the linked growth-derived
   * observation capability.
   * @param fleet Shared pointer to the fleet object.
   * @param growth_products Growth products for the linked population.
   * @param year Year index.
   * @param age Age index.
   * @param alk_row Output vector storing the normalized age-to-length
   * probabilities for the requested year and age.
   * @return Expected weight-at-age from the same normalized ALK row.
   */
  Type BuildGrowthDerivedALKRowAndMeanWeight(
      const std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>&
          growth_observation,
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      const fims_popdy::GrowthProducts<Type>& growth_products,
      size_t year,
      size_t age,
       fims::Vector<Type> & alk_row) {
    alk_row = BuildNormalizedGrowthDerivedALKRow(fleet, growth_products, year,
                                                 age);
    return MeanWeightFromALKRow(growth_observation, fleet, alk_row);
  }

  /**
   * @brief Attempts to build a normalized dynamic ALK row for one fleet, year,
   * and age.
   *
   * This wrapper was added so callers can ask for the supported growth-derived
   * observation path
   * without duplicating eligibility checks everywhere. A return value of false
   * means "fall back to historical behavior."
   * 
   * @param fleet Shared pointer to the fleet object.
   * @param year Year index.
   * @param age Age index.
   * @param alk_row Output vector storing the normalized age-to-length
   * probabilities when the supported path is available.
   * @return True when the supported growth-derived ALK path is available.
   */
  bool TryBuildGrowthDerivedALKRow(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      size_t year,
      size_t age,
       fims::Vector<Type> & alk_row) {
    const fims_popdy::GrowthProducts<Type>* growth_products =
        GetGrowthProductsForFleet(fleet);
    if (growth_products == nullptr || fleet->lengths.size() != fleet->n_lengths) {
      return false;
    }
    alk_row =
        BuildNormalizedGrowthDerivedALKRow(fleet, *growth_products, year, age);
    return true;
  }

  /**
   * @brief Attempts to build a normalized dynamic ALK row and matching
   * midpoint-based expected weight-at-age for one fleet, year, and age.
   *
   * This is the same pattern as above, but for callers that need both
   * the ALK row and the corresponding bin-based mean WAA from the same row.
   * 
   * @param fleet Shared pointer to the fleet object.
   * @param year Year index.
   * @param age Age index.
   * @param alk_row Output vector storing the normalized age-to-length
   * probabilities when the supported path is available.
   * @param mean_weight Output expected weight-at-age from the same ALK row.
   * @return True when the supported growth-derived ALK path is available.
   */
  bool TryBuildGrowthDerivedALKRowAndMeanWeight(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      size_t year,
      size_t age,
       fims::Vector<Type> & alk_row,
      Type& mean_weight) {
    std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
        growth_observation = GetGrowthObservationForFleet(fleet);
    const fims_popdy::GrowthProducts<Type>* growth_products =
        GetGrowthProductsForFleet(fleet);
    if (growth_observation == nullptr || growth_products == nullptr ||
        fleet->lengths.size() != fleet->n_lengths) {
      return false;
    }
    mean_weight = BuildGrowthDerivedALKRowAndMeanWeight(
        growth_observation, fleet, *growth_products, year, age, alk_row);
    return true;
  }

  /**
   * @brief Computes fleet-specific expected weight-at-age from the same
   * normalized ALK row used in the dynamic length-composition path.
   * 
   * This was added so fleet-level quantities like landings and index weights
   * can use the same fleet-bin-aware WAA as the active growth-derived length
   * pathway,
   * rather than always the historical generic growth evaluation.
   *
   * @param population Shared pointer to the population object.
   * @param fleet Shared pointer to the fleet object.
   * @param year Year index.
   * @param age Age index.
   * @return Expected fleet-specific weight-at-age for the supported dynamic
   * ALK path, or the default growth evaluation if the dynamic path is
   * unavailable.
   */
  Type GrowthDerivedFleetMeanWeightAA(
      const std::shared_ptr<fims_popdy::Population<Type>>& population,
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      size_t year,
      size_t age) {
     fims::Vector<Type>  alk_row;
    Type mean_weight = static_cast<Type>(0.0);
    if (TryBuildGrowthDerivedALKRowAndMeanWeight(fleet, year, age, alk_row,
                                                 mean_weight)) {
      return mean_weight;
    }
    return population->growth->evaluate(population->ages[age]);
  }

  /**
   * @brief Computes population-level mean weight-at-age using the same
   * explicit-bin expectation as the supported dynamic ALK path when possible.
   *
   * This replaces direct use of growth->evaluate(age) for biomass and reference
   * point quantities when all linked fleets share one reusable explicit-bin
   * definition. If that shared definition does not exist, the historical
   * population growth evaluation is still used.
   *
   * @param population Shared pointer to the population object.
   * @param year Year index.
   * @param age Age index.
   * @return Mean weight-at-age on the natural scale.
   */
  Type PopulationMeanWeightAA(
      const std::shared_ptr<fims_popdy::Population<Type>>& population,
      size_t year,
      size_t age) {
    std::shared_ptr<fims_popdy::Fleet<Type>> canonical_fleet =
        GetCanonicalGrowthDerivedWeightFleet(population);
    if (canonical_fleet != nullptr) {
      return GrowthDerivedFleetMeanWeightAA(
          population, canonical_fleet, year, age);
    }

    // Historical fallback when no single shared fleet-bin definition can be
    // reused safely at the population level.
    return population->growth->evaluate(population->ages[age]);
  }

  /**
   * @brief Returns the canonical fleet whose explicit bins can be reused for
   * population-level growth-derived weight calculations.
   *
   * This helper was added because population-level biomass quantities do not
   * naturally belong to one fleet, but the bin-based WAA path needs one shared
   * bin definition. The canonical fleet is only accepted when every linked
   * fleet supports the narrow VonB path and all fleets share identical bins.
   *
   * @param population Shared pointer to the population object.
   * @return Shared pointer to the canonical fleet, or nullptr when the
   * population cannot use a single shared fleet-bin definition.
   */
  std::shared_ptr<fims_popdy::Fleet<Type>> GetCanonicalGrowthDerivedWeightFleet(
      const std::shared_ptr<fims_popdy::Population<Type>>& population) {
    if (population->fleets.size() == 0) {
      return nullptr;
    }
    const std::shared_ptr<fims_popdy::Fleet<Type>>& canonical_fleet =
        population->fleets[0];
    if (GetGrowthProductsForFleet(canonical_fleet) == nullptr ||
        canonical_fleet->lengths.size() != canonical_fleet->n_lengths) {
      return nullptr;
    }
    for (size_t fleet_ = 1; fleet_ < population->fleets.size(); ++fleet_) {
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet =
          population->fleets[fleet_];
      if (GetGrowthProductsForFleet(fleet) == nullptr ||
          fleet->n_lengths != canonical_fleet->n_lengths ||
          fleet->lengths.size() != canonical_fleet->lengths.size()) {
        return nullptr;
      }
      for (size_t l = 0; l < canonical_fleet->lengths.size(); ++l) {
        if (fleet->lengths[l] != canonical_fleet->lengths[l]) {
          return nullptr;
        }
      }
    }
    return canonical_fleet;
  }

  /**
   * @brief Returns whether a population can use a single canonical explicit
   * length-bin definition for the current growth-derived weight path.
   *
   * Small convenience wrapper used to keep reporting and higher-level logic
   * readable when checking whether the population can reuse one shared fleet
   * bin definition.
   * 
   * This is true when every linked fleet supports the narrow dynamic ALK path
   * and all linked fleets share the same length-bin centers.
   *
   * @param population Shared pointer to the population object.
   * @return True if a canonical fleet-bin definition can be used.
   */
  bool PopulationUsesCanonicalGrowthDerivedWeightBins(
      const std::shared_ptr<fims_popdy::Population<Type>>& population) {
    return GetCanonicalGrowthDerivedWeightFleet(population) != nullptr;
  }

  /**
   * @brief Returns the population linked to a fleet.
   * 
   * This lookup helper was added because fleets do not own growth objects
   * directly. The dynamic VonB path has to recover the linked population first
   * before it can access growth state for that fleet.
   *
   * @param fleet Shared pointer to the fleet object.
   * @return Linked population pointer, or nullptr if no population matches the
   * fleet.
   */
  std::shared_ptr<fims_popdy::Population<Type>> GetPopulationForFleet(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet) {
    for (size_t p = 0; p < this->populations.size(); ++p) {
      std::shared_ptr<fims_popdy::Population<Type>>& population =
          this->populations[p];
      for (size_t fleet_ = 0; fleet_ < population->fleets.size(); ++fleet_) {
        if (population->fleets[fleet_]->GetId() == fleet->GetId()) {
          return population;
        }
      }
    }
    return nullptr;
  }

  /**
   * @brief Returns the growth-derived observation capability for the
   * population linked to a fleet.
   *
   * This keeps the new path intentionally narrow: only fleets linked to a
   * growth model exposing the growth-derived observation contract can use the
   * dynamic ALK and bin-based WAA logic.
   * 
   * @param fleet Shared pointer to the fleet object.
   * @return Shared pointer to the linked growth-derived observation
   * capability, or nullptr.
   */
  std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
  GetGrowthObservationForFleet(const std::shared_ptr<fims_popdy::Fleet<Type>>&
                                   fleet) {
    std::shared_ptr<fims_popdy::Population<Type>> population =
        GetPopulationForFleet(fleet);
    if (population == nullptr) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<
        fims_popdy::GrowthDerivedObservationBase<Type>>(population->growth);
  }

  /**
   * @brief Returns growth products for the population linked to a fleet.
   *
   * This currently requires the linked growth object to expose the
   * growth-derived observation capability and returns `nullptr` when the fleet
   * is not linked to a supported growth model.
   *
   * @param fleet Shared pointer to the fleet object.
   * @return Pointer to the linked population growth products, or nullptr.
   */
  const fims_popdy::GrowthProducts<Type>* GetGrowthProductsForFleet(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet) {
    std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
        growth_observation = GetGrowthObservationForFleet(fleet);
    if (growth_observation == nullptr ||
        !growth_observation->SupportsGrowthDerivedALK()) {
      return nullptr;
    }
    const fims_popdy::GrowthProducts<Type>& gp =
        growth_observation->GetProductsForReporting();
    if (gp.n_sexes == 1) {
      return &gp;
    }
    return nullptr;
  }

  /**
   * @brief Returns whether a fleet can use the narrow growth-derived ALK path.
   *
   * Central eligibility gate for the growth-derived path. A fleet is eligible only
   * when supported growth products are available and the runtime fleet has a
   * consistent explicit length-bin definition.
   * 
   * @param fleet Shared pointer to the fleet object.
   * @return True when the fleet is linked to a supported single-sex
   * growth-derived observation path and has explicit length bins.
   */
  bool FleetUsesGrowthDerivedALK(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet) {
    return GetGrowthProductsForFleet(fleet) != nullptr &&
           fleet->lengths.size() == fleet->n_lengths;
  }

/**
 * @brief Builds fleet-level realized dynamic ALK and optional mean
 * weight-at-age vectors for the supported growth-derived path.
 *
 * Central eligibility gate for the growth-derived path. A fleet is eligible only
 * when supported growth products are available and the runtime fleet has a
 * consistent explicit length-bin definition.
 * 
 * @param fleet Shared pointer to the fleet object.
 * @param growth_derived_age_to_length_conversion Output vector storing the
 * realized year-age-length probabilities.
 * @param growth_derived_mean_WAA Optional output vector storing fleet-
 * specific expected weight-at-age by year and age.
 * @return True when the fleet supports the narrow growth-derived ALK path.
 */
bool TryBuildGrowthDerivedFleetALKAndMeanWeight(
    const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
     fims::Vector<Type> & growth_derived_age_to_length_conversion,
     fims::Vector<Type> * growth_derived_mean_WAA = nullptr) {
  // Stop immediately unless this fleet is linked to the supported
  // single-sex growth-derived observation path and has a consistent runtime
  // bin definition.
  if (!FleetUsesGrowthDerivedALK(fleet)) {
    return false;
  }

  // Allocate flattened [year, age, length] storage for the realized ALK.
  growth_derived_age_to_length_conversion.resize(
      fleet->n_years * fleet->n_ages * fleet->n_lengths);

  // If fleet-specific mean WAA output was requested, allocate flattened
  // [year, age] storage for those values.
  if (growth_derived_mean_WAA != nullptr) {
    growth_derived_mean_WAA->resize(fleet->n_years * fleet->n_ages);
  }

  // Build one normalized ALK row, and its matching mean WAA, for each
  // year-age combination in this fleet.
  for (size_t y = 0; y < fleet->n_years; ++y) {
    for (size_t a = 0; a < fleet->n_ages; ++a) {
      // Temporary ALK row over length bins for this (year, age).
       fims::Vector<Type>  alk_row;

      // Flattened [year, age] index for optional mean WAA storage.
      const size_t i_age_year = y * fleet->n_ages + a;

      // Expected mean WAA associated with this ALK row.
      Type mean_weight = static_cast<Type>(0.0);

      // Build the normalized ALK row and the matching mean WAA for this
      // year-age combination.
      if (!TryBuildGrowthDerivedALKRowAndMeanWeight(fleet, y, a, alk_row,
                                                    mean_weight)) {
        // This should be unreachable after the earlier fleet-level
        // eligibility check. If it happens, emit an error message and abort.
        std::stringstream ss;
        ss << "Growth-derived ALK eligibility changed while building fleet "
           << "growth report outputs for fleet id " << fleet->GetId()
           << ".";
        FIMS_ERROR_LOG(ss.str());
        return false;
      }

      // Store fleet-specific mean WAA for this (year, age) when requested.
      if (growth_derived_mean_WAA != nullptr) {
        (*growth_derived_mean_WAA)[i_age_year] = mean_weight;
      }

      // Copy this temporary ALK row into flattened [year, age, length]
      // storage using the report output order.
      for (size_t l = 0; l < fleet->n_lengths; ++l) {
        const size_t i_length_age_year =
            y * (fleet->n_ages * fleet->n_lengths) +
            a * fleet->n_lengths + l;
        growth_derived_age_to_length_conversion[i_length_age_year] =
            alk_row[l];
      }
    }
  }

  // Report that the supported growth-derived path was used successfully.
  return true;
}

  /**
   * @brief Resolves and validates which fleet ALK path is usable for the
   * current fleet context.
   *
   * Growth-derived ALK is preferred when the linked growth module supports the
   * derived path for this fleet. Otherwise, the historical fixed
   * age-to-length conversion is used when available. If neither path is valid
   * for a fleet with length bins, this throws a runtime error.
   *
   * @param fleet Shared pointer to the fleet object.
   * @param use_growth_derived_alk Output flag indicating selected ALK path.
   * @param has_fixed_age_to_length_matrix Output flag indicating whether a
   * valid fixed ALK matrix is available.
   * @param growth_derived_age_to_length_conversion Output realized dynamic ALK
   * values when the growth-derived path is selected.
   * @param growth_derived_mean_WAA Optional output realized mean WAA values
   * when the growth-derived path is selected.
   * @param context_label Short label used in logs/errors to identify caller
   * context.
   * @param materialize_growth_derived_alk Whether to materialize the full
   * [year, age, length] derived ALK tensor when selecting the derived path.
   * Use false in objective-side evaluation to avoid large AD allocations.
   * @return True when the growth-derived ALK path was selected.
   */
  bool ResolveFleetALKPath(
      const std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
      bool& use_growth_derived_alk,
      bool& has_fixed_age_to_length_matrix,
       fims::Vector<Type> & growth_derived_age_to_length_conversion,
       fims::Vector<Type> * growth_derived_mean_WAA = nullptr,
      const std::string& context_label = "model evaluation",
      bool materialize_growth_derived_alk = true) {
    has_fixed_age_to_length_matrix =
        fleet->age_to_length_conversion.size() ==
        (fleet->n_ages * fleet->n_lengths);

    use_growth_derived_alk = false;
    growth_derived_age_to_length_conversion.resize(0);
    if (growth_derived_mean_WAA != nullptr) {
      growth_derived_mean_WAA->resize(0);
    }

    // Preferred path: if this fleet supports growth-derived ALK, use it.
    if (FleetUsesGrowthDerivedALK(fleet)) {
      if (materialize_growth_derived_alk) {
        if (TryBuildGrowthDerivedFleetALKAndMeanWeight(
                fleet, growth_derived_age_to_length_conversion,
                growth_derived_mean_WAA)) {
          use_growth_derived_alk = true;
        } else {
          std::stringstream ss;
          ss << "Growth-derived ALK eligibility changed while resolving "
             << context_label << " path for fleet id " << fleet->GetId() << ".";
          FIMS_ERROR_LOG(ss.str());
          throw std::runtime_error(ss.str());
        }
      } else {
        // Objective-side check: verify derived ALK availability using one
        // representative (year, age) row without allocating the full tensor.
         fims::Vector<Type>  probe_row;
        if (fleet->n_years == 0 || fleet->n_ages == 0 ||
            TryBuildGrowthDerivedALKRow(fleet, 0, 0, probe_row)) {
          use_growth_derived_alk = true;
        } else {
          std::stringstream ss;
          ss << "Growth-derived ALK eligibility changed while resolving "
             << context_label << " path for fleet id " << fleet->GetId() << ".";
          FIMS_ERROR_LOG(ss.str());
          throw std::runtime_error(ss.str());
        }

        // Report-side lightweight path: compute only fleet mean WAA on-demand
        // with row-wise ALK construction, without materializing the full
        // [year, age, length] tensor.
        if (use_growth_derived_alk && growth_derived_mean_WAA != nullptr) {
          growth_derived_mean_WAA->resize(fleet->n_years * fleet->n_ages);
          for (size_t y = 0; y < fleet->n_years; ++y) {
            for (size_t a = 0; a < fleet->n_ages; ++a) {
              const size_t i_age_year = y * fleet->n_ages + a;
               fims::Vector<Type>  alk_row;
              Type mean_weight = static_cast<Type>(0.0);
              if (!TryBuildGrowthDerivedALKRowAndMeanWeight(
                      fleet, y, a, alk_row, mean_weight)) {
                std::stringstream mean_waa_ss;
                mean_waa_ss
                    << "Growth-derived ALK eligibility changed while resolving "
                    << context_label << " mean WAA for fleet id "
                    << fleet->GetId() << ".";
                FIMS_ERROR_LOG(mean_waa_ss.str());
                throw std::runtime_error(mean_waa_ss.str());
              }
              (*growth_derived_mean_WAA)[i_age_year] = mean_weight;
            }
          }
        }
      }
    }

    // Fallback path: use historical fixed ALK when available.
    if (!use_growth_derived_alk && has_fixed_age_to_length_matrix) {
      return false;
    }

    if (fleet->n_lengths > 0 && !use_growth_derived_alk) {
      std::stringstream ss;
      ss << "Fleet id " << fleet->GetId()
         << " has length composition bins but no usable age-to-length "
         << "conversion path for " << context_label << ". Provide fixed "
         << "age-to-length conversion of size "
         << (fleet->n_ages * fleet->n_lengths)
         << " or use a supported growth-derived ALK path.";
      FIMS_ERROR_LOG(ss.str());
      throw std::runtime_error(ss.str());
    }

    return use_growth_derived_alk;
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
        bool use_growth_derived_alk = false;
        bool has_fixed_age_to_length_matrix = false;
         fims::Vector<Type>  growth_derived_age_to_length_conversion;
        ResolveFleetALKPath(fleet, use_growth_derived_alk,
                            has_fixed_age_to_length_matrix,
                            growth_derived_age_to_length_conversion, nullptr,
                            "length composition", false);
        (void)has_fixed_age_to_length_matrix;

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
          for (size_t a = 0; a < fleet->n_ages; a++) {
            size_t i_age_year = y * fleet->n_ages + a;
            size_t i_length_age_base = a * fleet->n_lengths;
             fims::Vector<Type>  alk_row;
            if (use_growth_derived_alk &&
                !TryBuildGrowthDerivedALKRow(fleet, y, a, alk_row)) {
              std::stringstream ss;
              ss << "Growth-derived ALK eligibility changed while resolving "
                 << "length composition row for fleet id " << fleet->GetId()
                 << ".";
              FIMS_ERROR_LOG(ss.str());
              throw std::runtime_error(ss.str());
            }
            for (size_t l = 0; l < fleet->n_lengths; l++) {
              size_t i_length_year = y * fleet->n_lengths + l;
              const Type age_to_length_prob =
                  use_growth_derived_alk ? alk_row[l]
                                         : fleet->age_to_length_conversion[i_length_age_base + l];
              fdq_["lengthcomp_expected"][i_length_year] +=
                  fdq_["agecomp_expected"][i_age_year] * age_to_length_prob;

              fdq_["landings_numbers_at_length"][i_length_year] +=
                  fdq_["landings_numbers_at_age"][i_age_year] *
                  age_to_length_prob;

              fdq_["index_numbers_at_length"][i_length_year] +=
                  fdq_["index_numbers_at_age"][i_age_year] *
                  age_to_length_prob;
            }
          }

          for (size_t l = 0; l < fleet->n_lengths; l++) {
            size_t i_length_year = y * fleet->n_lengths + l;
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
              /*
             Expected recruitment in year 0 is numbers at age 0 in year 0.
             */
              pdq_["expected_recruitment"][y] =
                  pdq_["numbers_at_age"][i_age_year];
              pdq_["unfished_numbers_at_age"][i_age_year] =
                  fims_math::exp(population->recruitment->log_rzero[0]);
            } else {
              CalculateUnfishedNumbersAA(population, i_age_year, a - 1, a);
            }

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
      vector<vector<Type>> log_M_p(n_pops);
      vector<vector<Type>> log_init_naa_p(n_pops);
      vector<vector<Type>> spawning_biomass_ratio_p(n_pops);
      vector<vector<Type>> log_f_multiplier_p(n_pops);
      vector<vector<Type>> growth_mean_LAA_p(n_pops);
      vector<vector<Type>> growth_sd_LAA_p(n_pops);
      vector<vector<Type>> growth_mean_WAA_p(n_pops);

      // initialize fleet vectors
      vector<vector<Type>> agecomp_expected_f(n_fleets);
      vector<vector<Type>> agecomp_proportion_f(n_fleets);
      vector<vector<Type>> age_to_length_conversion_f(n_fleets);
      vector<vector<Type>> catch_index_f(n_fleets);
      vector<vector<Type>> growth_derived_age_to_length_conversion_f(n_fleets);
      vector<vector<Type>> growth_derived_alk_used_f(n_fleets);
      vector<vector<Type>> growth_derived_mean_WAA_f(n_fleets);
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
        this->populations[p]->recruitment->create_report_vectors(
            report_vectors);
        biomass_p(pop_idx) = derived_quantities["biomass"].to_tmb();
        expected_recruitment_p(pop_idx) =
            derived_quantities["expected_recruitment"].to_tmb();
        mortality_F_p(pop_idx) = derived_quantities["mortality_F"].to_tmb();
        mortality_M_p(pop_idx) = derived_quantities["mortality_M"].to_tmb();
        mortality_Z_p(pop_idx) = derived_quantities["mortality_Z"].to_tmb();
        numbers_at_age_p(pop_idx) =
            derived_quantities["numbers_at_age"].to_tmb();
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
        spawning_biomass_ratio_p(pop_idx) =
            this->populations[pop_idx]->spawning_biomass_ratio.to_tmb();
        log_f_multiplier_p(pop_idx) =
            this->populations[pop_idx]->log_f_multiplier.to_tmb();

        if (std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<Type>>
                growth_observation = std::dynamic_pointer_cast<
                    fims_popdy::GrowthDerivedObservationBase<Type>>(
                    this->populations[p]->growth)) {
          const auto& gp = growth_observation->GetProductsForReporting();
          const std::size_t n = gp.Size();
          vector<Type> mean_laa(n);
          vector<Type> sd_laa(n);
          vector<Type> mean_waa(n);
          const bool use_growth_derived_mean_waa =
              isDouble<Type>::value &&
              PopulationUsesCanonicalGrowthDerivedWeightBins(
                  this->populations[p]);
          for (std::size_t i = 0; i < n; ++i) {
            mean_laa(i) = gp.mean_LAA[i];
            sd_laa(i) = gp.sd_LAA[i];
            if (use_growth_derived_mean_waa) {
              const std::size_t year = i / this->populations[p]->n_ages;
              const std::size_t age = i % this->populations[p]->n_ages;
              mean_waa(i) =
                  PopulationMeanWeightAA(this->populations[p], year, age);
            } else {
              mean_waa(i) = gp.mean_WAA[i];
            }
          }
          growth_mean_LAA_p(pop_idx) = mean_laa;
          growth_sd_LAA_p(pop_idx) = sd_laa;
          growth_mean_WAA_p(pop_idx) = mean_waa;
        }

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
        age_to_length_conversion_f(fleet_idx) =
            fleet->age_to_length_conversion.to_tmb();
        vector<Type> growth_derived_age_to_length_conversion;
        vector<Type> growth_derived_alk_used;
        vector<Type> growth_derived_mean_WAA;
        if (isDouble<Type>::value) {
          growth_derived_alk_used.resize(1);
          bool use_growth_derived_alk = false;
          bool has_fixed_age_to_length_matrix = false;
          const bool materialize_growth_derived_alk_report =
              this->report_growth_derived_alk_tensor;
          ResolveFleetALKPath(
              fleet, use_growth_derived_alk, has_fixed_age_to_length_matrix,
              growth_derived_age_to_length_conversion,
              &growth_derived_mean_WAA, "reporting",
              materialize_growth_derived_alk_report);
          (void)has_fixed_age_to_length_matrix;
          growth_derived_alk_used(0) =
              use_growth_derived_alk ? static_cast<Type>(1.0)
                                     : static_cast<Type>(0.0);
        }
        growth_derived_age_to_length_conversion_f(fleet_idx) =
            growth_derived_age_to_length_conversion;
        growth_derived_alk_used_f(fleet_idx) = growth_derived_alk_used;
        growth_derived_mean_WAA_f(fleet_idx) = growth_derived_mean_WAA;
        catch_index_f(fleet_idx) = derived_quantities["catch_index"].to_tmb();
        index_expected_f(fleet_idx) =
            derived_quantities["index_expected"].to_tmb();
        index_numbers_f(fleet_idx) =
            derived_quantities["index_numbers"].to_tmb();
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
      vector<Type> log_f_multiplier = ADREPORTvector(log_f_multiplier_p);

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
      FIMS_REPORT_F_("log_M", log_M_p, this->of);
      FIMS_REPORT_F_("log_init_naa", log_init_naa_p, this->of);
      FIMS_REPORT_F_("spawning_biomass_ratio", spawning_biomass_ratio_p,
                     this->of);
      FIMS_REPORT_F_("log_f_multiplier", log_f_multiplier_p, this->of);
      FIMS_REPORT_F_("growth_mean_LAA", growth_mean_LAA_p, this->of);
      FIMS_REPORT_F_("growth_sd_LAA", growth_sd_LAA_p, this->of);
      FIMS_REPORT_F_("growth_mean_WAA", growth_mean_WAA_p, this->of);

      // adreport
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
      ADREPORT_F(log_f_multiplier, this->of);

      // fleets
      // report
      FIMS_REPORT_F_("agecomp_expected", agecomp_expected_f, this->of);
      FIMS_REPORT_F_("agecomp_proportion", agecomp_proportion_f, this->of);
      FIMS_REPORT_F_("age_to_length_conversion", age_to_length_conversion_f,
                     this->of);
      FIMS_REPORT_F_("growth_derived_age_to_length_conversion",
                     growth_derived_age_to_length_conversion_f, this->of);
      FIMS_REPORT_F_("growth_derived_alk_used", growth_derived_alk_used_f,
                     this->of);
      FIMS_REPORT_F_("growth_derived_mean_WAA", growth_derived_mean_WAA_f,
                     this->of);
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
