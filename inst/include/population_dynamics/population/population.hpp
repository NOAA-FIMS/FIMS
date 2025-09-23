/**
 * @file population.hpp
 * @brief Defines the Population class and its fields and methods.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_HPP

#include "../../common/model_object.hpp"
#include "../fleet/fleet.hpp"
#include "../growth/growth.hpp"
#include "../recruitment/recruitment.hpp"
#include "../../interface/interface.hpp"
#include "../maturity/maturity.hpp"

namespace fims_popdy {

/**
 * @brief Population class. Contains subpopulations
 * that are divided into generic partitions (e.g., sex, area).
 */
template <typename Type>
struct Population : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for population object*/
  size_t nyears;        /*!< total number of years in the fishery*/
  size_t nseasons;      /*!< total number of seasons in the fishery*/
  size_t nages;         /*!< total number of ages in the population*/
  size_t nfleets;       /*!< total number of fleets in the fishery*/

  // parameters are estimated; after initialize in create_model, push_back to
  // parameter list - in information.hpp (same for initial F in fleet)
  fims::Vector<Type>
      log_init_naa; /*!< estimated parameter: natural log of numbers at age*/
  fims::Vector<Type>
      log_M; /*!< estimated parameter: natural log of Natural Mortality*/
  fims::Vector<Type> proportion_female = fims::Vector<Type>(
      1, static_cast<Type>(0.5)); /*!< proportion female by age */

  // Transformed values
  fims::Vector<Type> M; /*!< transformed parameter: natural mortality*/

  fims::Vector<double> ages;      /*!< vector of the ages for referencing*/
  fims::Vector<double> years;     /*!< vector of years for referencing*/
  fims::Vector<Type> mortality_F; /*!< vector of fishing mortality summed across
                             fleet by year and age*/
  fims::Vector<Type>
      mortality_Z; /*!< vector of total mortality by year and age*/

  // derived quantities
  fims::Vector<Type>
      weight_at_age; /*!< Derived quantity: expected weight at age */
  // fecundity removed because we don't need it yet
  fims::Vector<Type> numbers_at_age; /*!< Derived quantity: population expected
                                numbers at age in each year*/
  fims::Vector<Type>
      unfished_numbers_at_age; /*!< Derived quantity: population expected
                              unfished numbers at age in each year*/
  fims::Vector<Type>
      biomass; /*!< Derived quantity: total population biomass in each year*/
  fims::Vector<Type> spawning_biomass; /*!< Derived quantity: Spawning_biomass*/
  fims::Vector<Type> unfished_biomass; /*!< Derived quantity
                                  biomass assuming unfished*/
  fims::Vector<Type> unfished_spawning_biomass; /*!< Derived quantity Spawning
                                           biomass assuming unfished*/
  fims::Vector<Type> proportion_mature_at_age; /*!< Derived quantity: Proportion
                                          mature at age */
  fims::Vector<Type>
      total_landings_weight; /*!< Derived quantity: Total landings in weight*/
  fims::Vector<Type>
      total_landings_numbers; /*!< Derived quantity: Total landings in numbers*/
  fims::Vector<Type> expected_recruitment; /*!< Expected recruitment */
  fims::Vector<Type> sum_selectivity;      /*!< TODO: add documentation */
  /// recruitment
  int recruitment_id = -999; /*!< id of recruitment model object*/
  std::shared_ptr<fims_popdy::RecruitmentBase<Type>>
      recruitment; /*!< shared pointer to recruitment module */

  // growth
  int growth_id = -999; /*!< id of growth model object*/
  std::shared_ptr<fims_popdy::GrowthBase<Type>>
      growth; /*!< shared pointer to growth module */

  // maturity
  int maturity_id = -999; /*!< id of maturity model object*/
  std::shared_ptr<fims_popdy::MaturityBase<Type>>
      maturity; /*!< shared pointer to maturity module */

  // fleet
  std::set<uint32_t> fleet_ids; /*!< id of fleet model object*/
  std::vector<std::shared_ptr<fims_popdy::Fleet<Type>>>
      fleets; /*!< shared pointer to fleet module */

  // Define objective function object to be able to REPORT and ADREPORT

  std::map<std::string, fims::Vector<Type>>
      derived_quantities; /*!< derived quantities for specific model type, i.e.
                             caa, surplus production, etc */
  /**
   * @brief Iterator for the derived quantities.
   *
   */
  typedef typename std::map<std::string, fims::Vector<Type>>::iterator
      derived_quantities_iterator;
  // this -> means you're referring to a class member (member of self)

  Population() { this->id = Population::id_g++; }

  /**
   * @brief Initialize values. Called once at the start of model run.
   *
   * @param nyears number of years in the population
   * @param nseasons number of seasons in the population
   * @param nages number of ages in the population
   */
  void Initialize(int nyears, int nseasons, int nages) {
    this->nyears = nyears;
    this->nseasons = nseasons;
    this->nages = nages;

    // size all the vectors to length of nages
    nfleets = fleets.size();
    total_landings_weight.resize(nyears);
    total_landings_numbers.resize(nyears);
    years.resize(nyears);
    mortality_F.resize(nyears * nages);
    mortality_Z.resize(nyears * nages);
    proportion_mature_at_age.resize((nyears + 1) * nages);
    proportion_female.resize(nages);
    weight_at_age.resize(nages);
    unfished_numbers_at_age.resize((nyears + 1) * nages);
    biomass.resize((nyears + 1));
    unfished_biomass.resize((nyears + 1));
    unfished_spawning_biomass.resize((nyears + 1));
    spawning_biomass.resize((nyears + 1));
    expected_recruitment.resize((nyears + 1));
    M.resize(nyears * nages);
    ages.resize(nages);
    log_init_naa.resize(nages);
    log_M.resize(nyears * nages);
  }

  /**
   * @brief Prepare to run the population loop. Called at each model iteration,
   * and used to zero out derived quantities, values that were summed, etc.
   *
   */
  void Prepare() {
    for (size_t fleet = 0; fleet < this->fleets.size(); fleet++) {
      this->fleets[fleet]->Prepare();
    }

    std::fill(biomass.begin(), biomass.end(), static_cast<Type>(0.0));
    std::fill(unfished_biomass.begin(), unfished_biomass.end(),
              static_cast<Type>(0.0));
    std::fill(unfished_spawning_biomass.begin(),
              unfished_spawning_biomass.end(), static_cast<Type>(0.0));
    std::fill(spawning_biomass.begin(), spawning_biomass.end(),
              static_cast<Type>(0.0));
    std::fill(total_landings_weight.begin(), total_landings_weight.end(),
              static_cast<Type>(0.0));
    std::fill(total_landings_numbers.begin(), total_landings_numbers.end(),
              static_cast<Type>(0.0));
    std::fill(expected_recruitment.begin(), expected_recruitment.end(),
              static_cast<Type>(0.0));
    std::fill(proportion_mature_at_age.begin(), proportion_mature_at_age.end(),
              static_cast<Type>(0.0));
    std::fill(mortality_Z.begin(), mortality_Z.end(), static_cast<Type>(0.0));
    std::fill(proportion_female.begin(), proportion_female.end(),
              static_cast<Type>(0.5));

    // Transformation Section
    for (size_t age = 0; age < this->nages; age++) {
      this->weight_at_age[age] = growth->evaluate(ages[age]);
      for (size_t year = 0; year < this->nyears; year++) {
        size_t i_age_year = age * this->nyears + year;
        this->M[i_age_year] = fims_math::exp(this->log_M[i_age_year]);
        // mortality_F is a fims::Vector and therefore needs to be filled
        // within a loop
        this->mortality_F[i_age_year] = static_cast<Type>(0.0);
      }
    }
  }

  /**
   * @brief Calculates initial numbers at age for index and age
   *
   * The formula used is:
   * \f[
   * N_{a,0} = \exp(\log N_{a,0})
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param a age index
   */
  inline void CalculateInitialNumbersAA(
      size_t i_age_year, size_t a) {  // inline all function unless complicated
    this->numbers_at_age[i_age_year] = fims_math::exp(this->log_init_naa[a]);
  }

  /**
   * @brief Calculates total mortality at an index, year, and age
   *
   * This function calculates total mortality \f$Z\f$ for a specific
   * age and year, combining natural mortality \f$M\f$ and fishing
   * mortality \f$F\f$ from all fleets.
   *
   * The fishing mortality \f$F_{f,a,y}\f$ for each fleet \f$f\f$ is computed
   * using age-specific selectivity \f$S_f(a)\f$ and fleet-specific, annual
   * fishing mortality \f$F_{f,y}\f$:
   * \f[
   * F_{f,a,y} = F_{f,y} \times S_f(a)
   * \f]
   *
   * Total fishing mortality at age \f$a\f$ and year \f$y\f$ is the sum over
   * fleets: \f[ F_{a,y} = \sum_{f=1}^{N_{fleets}} F_{f,a,y} \f]
   *
   * Total mortality \f$Z_{f,a,y}\f$ is the sum of natural and fishing
   * mortality: \f[ Z_{a,y} = M_{a,y} + F_{a,y} \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param year year index
   * @param age age index
   */
  void CalculateMortality(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      this->mortality_F[i_age_year] +=
          this->fleets[fleet_]->Fmort[year] *
          // evaluate is a member function of the selectivity class
          this->fleets[fleet_]->selectivity->evaluate(ages[age]);
    }

    this->mortality_Z[i_age_year] =
        this->M[i_age_year] + this->mortality_F[i_age_year];
  }

  /**
   * @brief Calculates numbers at age at year and age specific indices
   *
   * This function calculates numbers at age by applying total mortality \f$Z\f$
   * to individuals from the previous time step. It also handles the
   * accumulation of a plus group.
   *
   * Standard update:
   * \f[
   * N_{a,y} = N_{a-1,y-1} * \exp(-Z_{a-1,y-1})
   * \f]
   *
   * Plus group update (if \f$a = A\f$):
   * \f[
   * N_{A,y} = N_{A,y} + N_{A,y-1} * \exp(-Z_{A,y-1})
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param i_agem1_yearm1 dimension folded index for age-1 and year-1
   * @param age age index
   */
  inline void CalculateNumbersAA(size_t i_age_year, size_t i_agem1_yearm1,
                                 size_t age) {
    // using Z from previous age/year
    this->numbers_at_age[i_age_year] =
        this->numbers_at_age[i_agem1_yearm1] *
        (fims_math::exp(-this->mortality_Z[i_agem1_yearm1]));

    // Plus group calculation
    if (age == (this->nages - 1)) {
      this->numbers_at_age[i_age_year] =
          this->numbers_at_age[i_age_year] +
          this->numbers_at_age[i_agem1_yearm1 + 1] *
              (fims_math::exp(-this->mortality_Z[i_agem1_yearm1 + 1]));
    }
  }

  /**
   * @brief Calculates unfished numbers at age at year and age specific indices
   *
   * This function computes unfished numbers at age by applying survival
   * through time using only natural mortality, without any fishing pressure.
   * It also accounts for accumulation of the plus group.
   *
   * Standard update:
   * \f[
   * N^U_{a,y} = N^U_{a-1,y-1} * \exp(-M_{a-1,y-1})
   * \f]
   *
   * Plus group update (if \f$a = A\f$):
   * \f[
   * N^U_{A,y} = N^U_{A,y} + N^U_{A,y-1} * \exp(-M_{A,y-1})
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param i_agem1_yearm1 dimension folded index for age-1 and year-1
   * @param age age index
   */
  inline void CalculateUnfishedNumbersAA(size_t i_age_year,
                                         size_t i_agem1_yearm1, size_t age) {
    // using M from previous age/year
    this->unfished_numbers_at_age[i_age_year] =
        this->unfished_numbers_at_age[i_agem1_yearm1] *

        (fims_math::exp(-this->M[i_agem1_yearm1]));

    // Plus group calculation
    if (age == (this->nages - 1)) {
      this->unfished_numbers_at_age[i_age_year] =
          this->unfished_numbers_at_age[i_age_year] +
          this->unfished_numbers_at_age[i_agem1_yearm1 + 1] *
              (fims_math::exp(-this->M[i_agem1_yearm1 + 1]));
    }
  }

  /**
   * @brief Calculates biomass
   *
   * Adds the biomass at age to the total biomass for a given year \f$y\f$ by
   * multiplying numbers at age \f$a\f$ by weight at age \f$w_a\f$:
   *
   * \f[
   * B_y += N_{a,y} * w_a
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year biomass is being aggregated for
   * @param age the age whose biomass is being added into total biomass
   */
  void CalculateBiomass(size_t i_age_year, size_t year, size_t age) {
    this->biomass[year] +=
        this->numbers_at_age[i_age_year] * this->weight_at_age[age];
  }

  /**
   * @brief Adds to existing yearly unfished biomass estimates the
   *  biomass for a specified year and age
   *
   * Updates unfished biomass \f$B^U_y\f$ by adding the biomass
   * of age \f$a\f$ in year \f$y\f$, calculated as:
   *
   * \f[
   * B^U_y += N^U_{a,y} * w_a
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year of unfished biomass to add
   * @param age the age of unfished biomass to add
   */
  void CalculateUnfishedBiomass(size_t i_age_year, size_t year, size_t age) {
    this->unfished_biomass[year] +=
        this->unfished_numbers_at_age[i_age_year] * this->weight_at_age[age];
  }

  /**
   * @brief Calculates spawning biomass
   *
   * This function computes yearly \f$y\f$ spawning biomass \f$SB_y\f$ by
   * summing the contributions from each age \f$a\f$, accounting for the
   * proportion female, proportion mature, and weight at age \f$w_a\f$:
   *
   * \f[
   * SB_y += N_{a,y} * w_a * p_{female,a} * p_{mature,a}
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year spawning biomass is being aggregated for
   * @param age the age whose biomass is being added into total spawning biomass
   */
  void CalculateSpawningBiomass(size_t i_age_year, size_t year, size_t age) {
    this->spawning_biomass[year] +=
        this->proportion_female[age] * this->numbers_at_age[i_age_year] *
        this->proportion_mature_at_age[i_age_year] * this->weight_at_age[age];
  }

  /**
   * @brief Adds to existing yearly unfished spawning biomass estimates the
   *  biomass for a specified year and age
   *
   * Updates the unfished spawning biomass \f$SB^U_y\f$ by adding the
   * biomass of age \f$a\f$ in year \f$y\f$, accounting for proportion
   * female, maturity, and weight at age \f$w_a\f$:
   *
   * \f[
   * SB^U_y += N^U_{a,y} * w_a * p_{female,a} * p_{mature,a}
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year of unfished spawning biomass to add
   * @param age the age of unfished spawning biomass to add
   */
  void CalculateUnfishedSpawningBiomass(size_t i_age_year, size_t year,
                                        size_t age) {
    this->unfished_spawning_biomass[year] +=
        this->proportion_female[age] *
        this->unfished_numbers_at_age[i_age_year] *
        this->proportion_mature_at_age[i_age_year] * this->weight_at_age[age];
  }

  /**
   * @brief Calculates equilibrium spawning biomass per recruit
   *
   * This function calculates the spawning biomass per recruit \f$\phi_0\f$
   * at equilibrium, assuming an unfished stock. The biomass is calculated as
   * the sum of the biomass contributions from each age \f$a\f$:
   *
   * \f[
   * \phi_0 = \sum_{a=0}^{A} N_a * p_{female,a} * p_{mature,a} * w_a
   * \f]
   *
   * The numbers at age \f$N_a\f$ are calculated recursively with natural
   * mortality: \f[ N_a = N_{a-1} * \exp(-M_a) \quad \text{for } a = 1, \ldots,
   * A-1 \f]
   *
   * Plus group update:
   * \f[
   * N_A = \frac{N_{A-1} * \exp(-M_{A-1})}{1 - \exp(-M_A)}
   * \f]
   *
   * @return Type
   */
  Type CalculateSBPR0() {
    std::vector<Type> numbers_spr(this->nages, 1.0);
    Type phi_0 = static_cast<Type>(0.0);
    phi_0 += numbers_spr[0] * this->proportion_female[0] *
             this->proportion_mature_at_age[0] *
             this->growth->evaluate(ages[0]);
    for (size_t a = 1; a < (this->nages - 1); a++) {
      numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-this->M[a]);
      phi_0 += numbers_spr[a] * this->proportion_female[a] *
               this->proportion_mature_at_age[a] *
               this->growth->evaluate(ages[a]);
    }

    numbers_spr[this->nages - 1] =
        (numbers_spr[nages - 2] * fims_math::exp(-this->M[nages - 2])) /
        (1 - fims_math::exp(-this->M[this->nages - 1]));
    phi_0 += numbers_spr[this->nages - 1] *
             this->proportion_female[this->nages - 1] *
             this->proportion_mature_at_age[this->nages - 1] *
             this->growth->evaluate(ages[this->nages - 1]);
    return phi_0;
  }

  /**
   * @brief Calculates expected recruitment for a given year
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
   * Where \f$f()\f$ evaluates mean recruitment based on spawning biomass
   * and \f$\phi_0\f$, and \f$g(y-1)\f$ evaluates recruitment deviations.
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year recruitment is being calculated for
   * @param i_dev index to log_recruit_dev of vector length nyears-1
   */
  void CalculateRecruitment(size_t i_age_year, size_t year, size_t i_dev) {
    Type phi0 = CalculateSBPR0();

    if (i_dev == this->nyears) {
      this->numbers_at_age[i_age_year] = this->recruitment->evaluate_mean(
          this->spawning_biomass[year - 1], phi0);
      /*the final year of the time series has no data to inform recruitment
      devs, so this value is set to the mean recruitment.*/
    } else {
      recruitment->log_expected_recruitment[year - 1] =
          fims_math::log(this->recruitment->evaluate_mean(
              this->spawning_biomass[year - 1], phi0));
      /*the log_recruit_err vector does not include a value for year == 0
          and is of length nyears - 1 where the first position of the vector
          corresponds to the second year of the time series. The first year is
          informed by the init_naa parameter*/
      this->numbers_at_age[i_age_year] =
          fims_math::exp(recruitment->process->evaluate_process(year - 1));
    }
    this->expected_recruitment[year] = this->numbers_at_age[i_age_year];
  }

  /**
   * @brief Calculates expected proportion of individuals mature at a selected
   * age
   *
   * This function evaluates the maturity ogive at the specified age
   * to estimate the proportion of individuals that are mature:
   *
   * \f[
   * p_{mature,a} = \text{maturity}(a)
   * \f]
   *
   * @param i_age_year dimension folded index for age and year
   * @param age the age of maturity
   */
  void CalculateMaturityAA(size_t i_age_year, size_t age) {
    // this->maturity is pointing to the maturity module, which has
    //  an evaluate function. -> can be nested.

    this->proportion_mature_at_age[i_age_year] =
        this->maturity->evaluate(ages[age]);
  }

  /**
   * @brief Adds to existing expected total landings by fleet
   *
   * This function updates fleet specific and total expected landings
   * for a given year and age by using age specific landings from each
   * fleet and summing over ages.
   *
   * @param year the year of expected total landings
   * @param age the age of landings that is being added into total landings
   */
  void CalculateLandings(size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      size_t i_age_year = year * this->nages + age;

      this->total_landings_weight[year] +=
          this->fleets[fleet_]->landings_weight_at_age[i_age_year];

      fleets[fleet_]->landings_weight[year] +=
          this->fleets[fleet_]->landings_weight_at_age[i_age_year];

      this->total_landings_numbers[year] +=
          this->fleets[fleet_]->landings_numbers_at_age[i_age_year];

      fleets[fleet_]->landings_numbers[year] +=
          this->fleets[fleet_]->landings_numbers_at_age[i_age_year];
    }
  }

  /**
   * @brief Calculates landings in numbers at age for each fleet for a given
   * year and age
   *
   * This function uses the Baranov Catch Equation to calculate expected
   * landings in numbers at age for each fleet:
   *
   * \f[
   * C_{f,a,y} = \frac{F_{f,y} * S_f(a)}{Z_{a,y}} * N_{a,y} *
   * \left( 1 - \exp(-Z_{a,y}) \right)
   * \f]
   *
   * where:
   * - \f$C_{f,a,y}\f$ is the catch (landings) for fleet \f$f\f$ at age \f$a\f$
   * in year \f$y\f$
   * - \f$F_{f,y}\f$ is the fleet-specific fishing mortality in year \f$y\f$
   * - \f$S_f(a)\f$ is the selectivity at age \f$a\f$ for fleet \f$f\f$
   * - \f$Z_{a,y}\f$ is the total mortality at age \f$a\f$ and year \f$y\f$
   * - \f$N_{a,y}\f$ is the number of individuals at age \f$a\f$ and year
   * \f$y\f$
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year expected landings composition is being calculated for
   * @param age the age composition is being calculated for
   */
  void CalculateLandingsNumbersAA(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      // Baranov Catch Equation
      this->fleets[fleet_]->landings_numbers_at_age[i_age_year] +=
          (this->fleets[fleet_]->Fmort[year] *
           this->fleets[fleet_]->selectivity->evaluate(ages[age])) /
          this->mortality_Z[i_age_year] * this->numbers_at_age[i_age_year] *
          (1 - fims_math::exp(-(this->mortality_Z[i_age_year])));
    }
  }

  /**
   * @brief Calculates expected landings weight at age for each fleet
   *
   * This function computes the expected landings at age in weight by
   * multiplying the expected landings numbers at age by the corresponding
   * weight at age:
   *
   * \f[
   * CW_{f,a,y} = C_{f,a,y} \times w_a
   * \f]
   *
   * where \f$CW_{f,a,y}\f$ is the catch weight for fleet \f$f\f$ at age
   * \f$a\f$ in year \f$y\f$.
   *
   * @param year the year of expected landings weight at age
   * @param age the age of expected landings weight at age
   */
  void CalculateLandingsWeightAA(size_t year, size_t age) {
    int i_age_year = year * this->nages + age;
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      this->fleets[fleet_]->landings_weight_at_age[i_age_year] =
          this->fleets[fleet_]->landings_numbers_at_age[i_age_year] *
          this->weight_at_age[age];
    }
  }

  /**
   * @brief Adds to the expected population indices by fleet
   *
   * This function updates the population indices for each fleet by adding the
   * age- and year-specific index weights and numbers to the corresponding
   * annual totals. The updated index weight and index numbers for a given
   * fleet and year are calculated as:
   *
   * \f[
   * IW_{f,y} += IWAA_{a,y}
   * \f]
   * \f[
   * IN_{f,y} += INAA_{a,y}
   * \f]
   *
   * where:
   * - \f$IW_{f,y}\f$ is the total index weight for fleet \f$f\f$ in year
   * \f$y\f$
   * - \f$IN_{f,y}\f$ is the total index numbers for fleet \f$f\f$ in year
   * \f$y\f$
   * - \f$IWAA_{a,y}\f$ is the index weight at age \f$a\f$ in year \f$y\f$
   * - \f$INAA_{a,y}\f$ is the index numbers at age \f$a\f$ in year \f$y\f$
   *
   * @param i_age_year dimension-folded index for age and year
   * @param year the year of the population index
   * @param age the age of the index being added to the population index
   */
  void CalculateIndex(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      fleets[fleet_]->index_weight[year] +=
          this->fleets[fleet_]->index_weight_at_age[i_age_year];

      fleets[fleet_]->index_numbers[year] +=
          this->fleets[fleet_]->index_numbers_at_age[i_age_year];
    }
  }

  /**
   * @brief Calculates index sample in numbers at age for each fleet
   *
   * This function calculates the expected index in numbers at age for each
   * fleet, using catchability, selectivity, and population numbers at age:
   *
   * \f[
   * I_{f,a,y} += q_{f,y} \times S_f(a) \times N_{a,y}
   * \f]
   *
   * where:
   * - \f$I_{f,a,y}\f$ is the index numbers for fleet \f$f\f$ at age \f$a\f$ in
   * year \f$y\f$
   * - \f$q_{f,y}\f$ is the catchability coefficient for fleet \f$f\f$ at year
   * \f$y\f$
   * - \f$S_f(a)\f$ is the selectivity at age \f$a\f$ for fleet \f$f\f$
   * - \f$N_{a,y}\f$ is the population numbers at age \f$a\f$ and year \f$y\f$
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year the expected index is being calculated for
   * @param age the age index is being calculated for
   */
  void CalculateIndexNumbersAA(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      this->fleets[fleet_]->index_numbers_at_age[i_age_year] +=
          (this->fleets[fleet_]->q.get_force_scalar(year) *
           this->fleets[fleet_]->selectivity->evaluate(ages[age])) *
          this->numbers_at_age[i_age_year];
    }
  }

  /**
   * @brief Calculates expected index weight at age for each fleet
   *
   * This function computes the expected index weight at age by multiplying the
   * expected index numbers at age by the corresponding weight at age:
   *
   * \f[
   * IWAA_{f,a,y} = I_{f,a,y} \times w_a
   * \f]
   *
   * where \f$IWAA_{f,a,y}\f$ is the index weight for fleet \f$f\f$ at age
   * \f$a\f$ in year \f$y\f$.
   *
   * @param year the year of expected index weight at age
   * @param age the age of expected index weight at age
   */
  void CalculateIndexWeightAA(size_t year, size_t age) {
    int i_age_year = year * this->nages + age;
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      this->fleets[fleet_]->index_weight_at_age[i_age_year] =
          this->fleets[fleet_]->index_numbers_at_age[i_age_year] *
          this->weight_at_age[age];
    }
  }

  /**
   * @brief Executes the population loop
   *
   */
  void Evaluate() {
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

     year loop is extended to <= nyears because SB is calculated as the start
     of the year value and by extending one extra year we get estimates of the
     population structure at the end of the final year. An alternative approach
     would be to keep initial numbers at age in it's own vector and each year to
     include the population structure at the end of the year. This is likely a
     null point given that we are planning to modify to an event/stanza based
     structure in later milestones which will eliminate this confusion by
     explicitly referencing the exact date (or period of averaging) at which any
     calculation or output is being made.
     */
    for (size_t y = 0; y <= this->nyears; y++) {
      for (size_t a = 0; a < this->nages; a++) {
        /*
         index naming defines the dimensional folding structure
         i.e. i_age_year is referencing folding over years and ages.
         */
        size_t i_age_year = y * this->nages + a;
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
        if (y < this->nyears) {
          /*
           First thing we need is total mortality aggregated across all fleets
           to inform the subsequent landings and change in numbers at age
           calculations. This is only calculated for years < nyears as these are
           the model estimated years with data. The year loop extends to
           y=nyears so that population numbers at age and SB can be calculated
           at the end of the last year of the model
           */
          CalculateMortality(i_age_year, y, a);
        }
        CalculateMaturityAA(i_age_year, a);
        /* if statements needed because some quantities are only needed
        for the first year and/or age, so these steps are included here.
         */
        if (y == 0) {
          // Initial numbers at age is a user input or estimated parameter
          // vector.
          CalculateInitialNumbersAA(i_age_year, a);

          if (a == 0) {
            this->unfished_numbers_at_age[i_age_year] =
                fims_math::exp(this->recruitment->log_rzero[0]);
          } else {
            CalculateUnfishedNumbersAA(i_age_year, a - 1, a);
          }

          /*
           Fished and unfished biomass vectors are summing biomass at
           age across ages.
           */

          CalculateBiomass(i_age_year, y, a);

          CalculateUnfishedBiomass(i_age_year, y, a);

          /*
           Fished and unfished spawning biomass vectors are summing biomass at
           age across ages to allow calculation of recruitment in the next year.
           */

          CalculateSpawningBiomass(i_age_year, y, a);

          CalculateUnfishedSpawningBiomass(i_age_year, y, a);

          /*
           Expected recruitment in year 0 is numbers at age 0 in year 0.
           */

          this->expected_recruitment[i_age_year] =
              this->numbers_at_age[i_age_year];

        } else {
          if (a == 0) {
            // Set the nrecruits for age a=0 year y (use pointers instead of
            // functional returns) assuming fecundity = 1 and 50:50 sex ratio
            CalculateRecruitment(i_age_year, y, y);
            this->unfished_numbers_at_age[i_age_year] =
                fims_math::exp(this->recruitment->log_rzero[0]);

          } else {
            size_t i_agem1_yearm1 = (y - 1) * nages + (a - 1);
            CalculateNumbersAA(i_age_year, i_agem1_yearm1, a);
            CalculateUnfishedNumbersAA(i_age_year, i_agem1_yearm1, a);
          }
          CalculateBiomass(i_age_year, y, a);
          CalculateSpawningBiomass(i_age_year, y, a);

          CalculateUnfishedBiomass(i_age_year, y, a);
          CalculateUnfishedSpawningBiomass(i_age_year, y, a);
        }

        /*
        Here composition, total landings, and index values are calculated for
        all years with reference data. They are not calculated for y=nyears as
        there is this is just to get final population structure at the end of
        the terminal year.
         */
        if (y < this->nyears) {
          CalculateLandingsNumbersAA(i_age_year, y, a);
          CalculateLandingsWeightAA(y, a);
          CalculateLandings(y, a);

          CalculateIndexNumbersAA(i_age_year, y, a);
          CalculateIndexWeightAA(y, a);
          CalculateIndex(i_age_year, y, a);
        }
      }
    }
  }
   virtual void create_report_vectors(
    std::map<std::string, fims::Vector<fims::Vector<Type>> >& report_vectors){
    report_vectors["log_init_naa"].emplace_back(this->log_init_naa);
    report_vectors["log_M"].emplace_back(this->log_M);
    }
    virtual void get_report_vector_count(
    std::map<std::string, size_t>& report_vector_count){
      report_vector_count["log_init_naa"] += 1;
      report_vector_count["log_M"] += 1;
    }
};
template <class Type>
uint32_t Population<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
