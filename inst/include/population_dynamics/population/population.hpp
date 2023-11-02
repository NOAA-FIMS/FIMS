/*
 * File:   population.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Population module file
 * The purpose of this file is to define the Population class and its fields
 * and methods.
 *
 *
 */
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_HPP

#include "../../common/model_object.hpp"
#include "../fleet/fleet.hpp"
#include "../growth/growth.hpp"
#include "../recruitment/recruitment.hpp"
//#include "subpopulation.hpp"
#include "../../interface/interface.hpp"
#include "../maturity/maturity.hpp"

namespace fims_popdy {
/*TODO:
Review, add functions to evaluate, push vectors back to fleet (or point to fleet
directly?)
 */

/**
 * @brief Population class. Contains subpopulations
 * that are divided into generic partitions (eg. sex, area).
 */
template <typename Type>
struct Population : public fims_model_object::FIMSObject<Type> {
  using ParameterVector =
      typename fims::ModelTraits<Type>::ParameterVector; /*!< the vector of
                                                      population parameters*/
  static uint32_t id_g; /*!< reference id for population object*/
  size_t nyears;        /*!< total number of years in the fishery*/
  size_t nseasons;      /*!< total number of seasons in the fishery*/
  size_t nages;         /*!< total number of ages in the population*/
  size_t nfleets;       /*!< total number of fleets in the fishery*/

  // constants
  const Type proportion_female =
      0.5; /*!< Sex proportion fixed at 50/50 for M1*/

  // parameters are estimated; after initialize in create_model, push_back to
  // parameter list - in information.hpp (same for initial F in fleet)
  std::vector<Type> log_init_naa; /*!< estimated parameter: log numbers at age*/
  ParameterVector log_M; /*!< estimated parameter: log Natural Mortality*/

  // Transformed values
  std::vector<Type> M; /*!< transformed parameter: Natural Mortality*/

  std::vector<double> ages;    /*!< vector of the ages for referencing*/
  std::vector<double> years;   /*!< vector of years for referencing*/
  ParameterVector mortality_F; /*!< vector of fishing mortality summed across
                                    fleet by year and age*/
  std::vector<Type>
      mortality_Z; /*!< vector of total mortality by year and age*/

  // derived quantities
  std::vector<Type>
      weight_at_age; /*!< Derived quantity: expected weight at age */
  // fecundity removed because we don't need it yet
  std::vector<Type> numbers_at_age; /*!< Derived quantity: population expected
                                 numbers at age in each year*/
  std::vector<Type>
      unfished_numbers_at_age; /*!< Derived quantity: population expected
                                unfished numbers at age in each year*/
  std::vector<Type>
      biomass; /*!< Derived quantity: total population biomass in each year*/
  std::vector<Type> spawning_biomass; /*!< Derived quantity: Spawning_biomass*/
  std::vector<Type> unfished_biomass; /*!< Derived quanity
                                            biomass assuming unfished*/
  std::vector<Type> unfished_spawning_biomass; /*!< Derived quanity Spawning
                                            biomass assuming unfished*/
  std::vector<Type> proportion_mature_at_age;  /*!< Derived quantity: Proportion
                                             matura at age */
  std::vector<Type> expected_numbers_at_age;   /*!< Expected values: Numbers at
                                              age (thousands?? millions??) */
  std::vector<Type> expected_catch;            /*!< Expected values: Catch*/
  std::vector<Type> expected_recruitment;      /*!< Expected recruitment */
  /// recruitment
  int recruitment_id = -999; /*!< id of recruitment model object*/
  std::shared_ptr<RecruitmentBase<Type>>
      recruitment; /*!< shared pointer to recruitment module */

  // growth
  int growth_id = -999; /*!< id of growth model object*/
  std::shared_ptr<GrowthBase<Type>>
      growth; /*!< shared pointer to growth module */

  // maturity
  int maturity_id = -999; /*!< id of maturity model object*/
  std::shared_ptr<MaturityBase<Type>>
      maturity; /*!< shared pointer to maturity module */

  // fleet
  int fleet_id = -999; /*!< id of fleet model object*/
  std::vector<std::shared_ptr<Fleet<Type>>>
      fleets; /*!< shared pointer to fleet module */

  // Define objective function object to be able to REPORT and ADREPORT
#ifdef TMB_MODEL
  ::objective_function<Type>
      *of;  // :: references global namespace, defined in src/FIMS.cpp,
            // available anywhere in the R package
#endif

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
    expected_catch.resize(nyears * nfleets);
    years.resize(nyears);
    mortality_F.resize(nyears * nages);
    mortality_Z.resize(nyears * nages);
    proportion_mature_at_age.resize((nyears + 1) * nages);
    weight_at_age.resize(nages);
    unfished_numbers_at_age.resize((nyears + 1) * nages);
    numbers_at_age.resize((nyears + 1) * nages);
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
    POPULATION_LOG << " population prepare " << this->nages << std::endl;
    POPULATION_LOG << "nfleets: " << this->nfleets << std::endl;
    POPULATION_LOG << "nseasons: " << this->nseasons << std::endl;
    POPULATION_LOG << "nyears: " << this->nyears << std::endl;

    for (size_t fleet = 0; fleet < this->nfleets; fleet++) {
      this->fleets[fleet]->Prepare();
    }

    std::fill(biomass.begin(), biomass.end(), 0.0);
    std::fill(unfished_spawning_biomass.begin(),
              unfished_spawning_biomass.end(), 0.0);
    std::fill(spawning_biomass.begin(), spawning_biomass.end(), 0.0);
    std::fill(expected_catch.begin(), expected_catch.end(), 0.0);
    std::fill(expected_recruitment.begin(), expected_recruitment.end(), 0.0);
    std::fill(proportion_mature_at_age.begin(), proportion_mature_at_age.end(),
              0.0);
    std::fill(mortality_Z.begin(), mortality_Z.end(), 0.0);

    // Transformation Section
    for (size_t age = 0; age < this->nages; age++) {
      this->weight_at_age[age] = growth->evaluate(ages[age]);
      for (size_t year = 0; year < this->nyears; year++) {
        size_t i_age_year = age * this->nyears + year;
        this->M[i_age_year] = fims_math::exp(this->log_M[i_age_year]);
        // mortality_F is a ParameterVector and therefore needs to be filled
        // within a loop
        this->mortality_F[i_age_year] = 0.0;
      }
    }
  }

  /**
   * life history calculations
   */

  /**
   * @brief Calculates initial numbers at age for index and age
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
   * @param i_age_year dimension folded index for age and year
   * @param year year index
   * @param age age index
   */
  void CalculateMortality(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      if (this->fleets[fleet_]->is_survey == false) {
        this->mortality_F[i_age_year] +=
            this->fleets[fleet_]->Fmort[year] *
            // evaluate is a member function of the selectivity class
            this->fleets[fleet_]->selectivity->evaluate(ages[age]);
        POPULATION_LOG << " selectivity at age " << ages[age] << " for fleet " << fleet_ << " is "
                 << this->fleets[fleet_]->selectivity->evaluate(ages[age])
                 << " apical fishing mortality F for the fleet in year " << year << " is "
                 << this->fleets[fleet_]->Fmort[year] << std::endl;
      }
    }
    POPULATION_LOG << "M in calculate mortality is " << this->M[i_age_year]
             << std::endl;
    this->mortality_Z[i_age_year] =
        this->M[i_age_year] + this->mortality_F[i_age_year];
  }

  /**
   * @brief Calculates numbers at age at year and age specific indices
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
    POPULATION_LOG << " z at i_agem1_yearm1 = " << i_agem1_yearm1 << " is "
             << this->mortality_Z[i_agem1_yearm1] << std::endl;
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
    POPULATION_LOG << "survival rate at index " << i_agem1_yearm1 << " is "
             << fims_math::exp(-(this->M[i_agem1_yearm1])) << std::endl;

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
   * @param i_age_year dimension folded index for age and year
   * @param year the year biomass is being aggregated for
   * @param age the age who's biomass is being added into total biomass
   */
  void CalculateBiomass(size_t i_age_year, size_t year, size_t age) {
    this->biomass[year] +=
        this->numbers_at_age[i_age_year] * this->weight_at_age[age];
    POPULATION_LOG << " age " << ages[age] << std::endl;
    POPULATION_LOG << "growth evaluate: " << this->weight_at_age[age]
             << " biomass inputs----- +++\n";
  }

  /**
   * @brief Adds to existing yearly unfished biomass estimates the
   *  biomass for a specified year and age
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year of unfished biomass to add
   * @param age the age of unfished biomass to add
   */
  void CalculateUnfishedBiomass(size_t i_age_year, size_t year, size_t age) {
    this->unfished_biomass[year] += this->unfished_numbers_at_age[i_age_year] *
                                    this->weight_at_age[age];
  }

  /**
   * @brief Calculates spawning biomass
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year spawning biomass is being aggregated for
   * @param age the age who's biomass is being added into total spawning biomass
   */
  void CalculateSpawningBiomass(size_t i_age_year, size_t year, size_t age) {
    this->spawning_biomass[year] += this->proportion_female *
                                    this->numbers_at_age[i_age_year] *
                                    this->proportion_mature_at_age[i_age_year] *
                                    this->weight_at_age[age];
    POPULATION_LOG << " proportion female " << this->proportion_female << " "
             << " mature age " << age << " is "
             << this->proportion_mature_at_age[i_age_year] << " "
             << " numbers at age " << this->numbers_at_age[i_age_year] << " "
             << " growth " << this->weight_at_age[age] << " "
             << " spawning biomass " << this->spawning_biomass[year] << " "
             << " spawning biomass inputs----- +++\n";
  }

  /**
   * @brief Adds to existing yearly unfished spawning biomass estimates the
   *  biomass for a specified year and age
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year of unfished spawning biomass to add
   * @param age the age of unfished spawning biomass to add
   */
  void CalculateUnfishedSpawningBiomass(size_t i_age_year, size_t year,
                                        size_t age) {
    this->unfished_spawning_biomass[year] +=
        this->proportion_female * this->unfished_numbers_at_age[i_age_year] *
        this->proportion_mature_at_age[i_age_year] *
        this->weight_at_age[age];
  }

  /**
   * @brief Calculates equilibrium spawning biomass per recruit
   *
   * @return Type
   */
  Type CalculateSBPR0() {
    std::vector<Type> numbers_spr(this->nages, 1.0);
    Type phi_0 = 0.0;
    phi_0 += numbers_spr[0] * this->proportion_female *
             this->proportion_mature_at_age[0] *
             this->growth->evaluate(ages[0]);
    for (size_t a = 1; a < (this->nages - 1); a++) {
      numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-this->M[a]);
      phi_0 += numbers_spr[a] * this->proportion_female *
               this->proportion_mature_at_age[a] *
               this->growth->evaluate(ages[a]);
    }

    numbers_spr[this->nages - 1] =
        (numbers_spr[nages - 2] * fims_math::exp(-this->M[nages - 2])) /
        (1 - fims_math::exp(-this->M[this->nages - 1]));
    phi_0 += numbers_spr[this->nages - 1] * this->proportion_female *
             this->proportion_mature_at_age[this->nages - 1] *
             this->growth->evaluate(ages[this->nages - 1]);
    return phi_0;
  }

  /**
   * @brief Calculates expected recruitment for a given year
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year recruitment is being calculated for
   */
  void CalculateRecruitment(size_t i_age_year, size_t year) {
    POPULATION_LOG << "recruitment 1" << std::endl;
    Type phi0 = CalculateSBPR0();
    POPULATION_LOG << "recruitment 2" << std::endl;
    POPULATION_LOG << "phi0 = " << phi0 << std::endl;
    POPULATION_LOG << "spawning biomass = " << this->spawning_biomass[year]
             << std::endl;
    POPULATION_LOG << "rec devs = " << this->recruitment->recruit_deviations[year - 1]
             << std::endl;
    POPULATION_LOG << "rec eval = "
             << this->recruitment->evaluate(this->spawning_biomass[year - 1],
                                            phi0)
             << std::endl;
    this->numbers_at_age[i_age_year] =
        this->recruitment->evaluate(this->spawning_biomass[year - 1], phi0) *
        this->recruitment->recruit_deviations[year];
    this->expected_recruitment[year] = this->numbers_at_age[i_age_year];
    POPULATION_LOG << " numbers at age at indexya " << i_age_year << " is "
             << this->numbers_at_age[i_age_year] << std::endl;
  }

  /**
   * @brief Adds to exiting expected total catch by fleet in weight
   *
   * @param year the year of expected total catch
   * @param age the age of catch that is being added into total catch
   */
  void CalculateCatch(size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      if (this->fleets[fleet_]->is_survey == false) {
        size_t index_yf = year * this->nfleets +
                          fleet_;  // index by fleet and years to dimension fold
        size_t i_age_year = year * this->nages + age;

        POPULATION_LOG << " fleet " << fleet_ << " year " << year << " age " << age
                 << std::endl;
        this->expected_catch[index_yf] +=
            this->fleets[fleet_]->catch_weight_at_age[i_age_year];

        POPULATION_LOG << "expected catch: " << this->expected_catch[index_yf]
                 << std::endl;
        POPULATION_LOG << "----------------------------------------------"
                 << std::endl;

        fleets[fleet_]->expected_catch[year] +=
            this->fleets[fleet_]->catch_weight_at_age[i_age_year];
      }
    }
  }

  /**
   * @brief Adds to the expected population indices by fleet
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year of the population index
   * @param age the age of the index that is added into population index
   */
  void CalculateIndex(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      Type index_;
      // I = qN (N is total numbers), I is an index in numbers
      if (this->fleets[fleet_]->is_survey == false) {
        index_ = this->fleets[fleet_]->catch_numbers_at_age[i_age_year] *
                 this->weight_at_age[age];
      } else {
        POPULATION_LOG << "fleet " << fleet_ << " is a survey" << std::endl;
        index_ = this->fleets[fleet_]->q *
                 this->fleets[fleet_]->selectivity->evaluate(ages[age]) *
                 this->numbers_at_age[i_age_year] *
                 this->weight_at_age[age];  // this->weight_at_age[age];
      }
      fleets[fleet_]->expected_index[year] += index_;
      POPULATION_LOG << " expected index in year  " << year << " is "
               << fleets[fleet_]->expected_index[year] << std::endl;
    }
  }

  /**
   * @brief Calculates catch in numbers at age for each fleet for a given year
   * and age, then adds the value to the expected catch in numbers at age for
   * each fleet
   *
   * @param i_age_year dimension folded index for age and year
   * @param year the year of expected catch composition is being calculated for
   * @param age the age composition is being calculated for
   */
  void CalculateCatchNumbersAA(size_t i_age_year, size_t year, size_t age) {
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      // make an intermediate value in order to set multiple members (of
      // current and fleet objects) to that value.
      Type catch_;  // catch_ is used to avoid using the c++ keyword catch
                    // Baranov Catch Equation
      if (this->fleets[fleet_]->is_survey == false) {
        catch_ = (this->fleets[fleet_]->Fmort[year] *
                  this->fleets[fleet_]->selectivity->evaluate(ages[age])) /
                 this->mortality_Z[i_age_year] *
                 this->numbers_at_age[i_age_year] *
                 (1 - fims_math::exp(-(this->mortality_Z[i_age_year])));
      } else {
        catch_ = (this->fleets[fleet_]->selectivity->evaluate(ages[age])) *
                 this->numbers_at_age[i_age_year];
      }
      POPULATION_LOG << " F " << fleet_ << "  " << this->fleets[fleet_]->Fmort[year]
               << std::endl;
      POPULATION_LOG << " selectivity "
               << this->fleets[fleet_]->selectivity->evaluate(ages[age])
               << std::endl;
      POPULATION_LOG << " catch " << catch_ << std::endl;
      // this->catch_numbers_at_age[i_age_yearf] += catch_;
      // catch_numbers_at_age for the fleet module has different
      // dimensions (year/age, not year/fleet/age)
      this->fleets[fleet_]->catch_numbers_at_age[i_age_year] += catch_;
    }
  }

  /**
   * @brief Calculates expected catch weight at age for each fleet for a given
   * year and age
   *
   * @param year the year of expected catch weight at age
   * @param age the age of expected catch weight at age
   */
  void CalculateCatchWeightAA(size_t year, size_t age) {
    int i_age_year = year * this->nages + age;
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      POPULATION_LOG << " fleet " << fleet_ << std::endl;
      POPULATION_LOG << " catchnaa "
               << this->fleets[fleet_]->catch_numbers_at_age[year] << std::endl;
      POPULATION_LOG << " weight " << this->weight_at_age[age] << std::endl;
      this->fleets[fleet_]->catch_weight_at_age[i_age_year] =
          this->fleets[fleet_]->catch_numbers_at_age[i_age_year] *
          this->weight_at_age[age];  
      POPULATION_LOG << " catch_waa "
               << this->fleets[fleet_]->catch_weight_at_age[i_age_year]
               << std::endl;
    }
  }

  /**
   * @brief Calculates expected proportion of individuals mature at a selected
   * ageage
   *
   * @param i_age_year dimension folded index for age and year
   * @param age the age of maturity
   */
  void CalculateMaturityAA(size_t i_age_year, size_t age) {
    // this->maturity is pointing to the maturity module, which has
    //  an evaluate function. -> can be nested.
    POPULATION_LOG << " age " << age << std::endl;
    POPULATION_LOG << " ages size " << this->ages.size() << std::endl;
    POPULATION_LOG << " i_age_year " << i_age_year << std::endl;
    POPULATION_LOG << "p mature" << this->proportion_mature_at_age[i_age_year]
             << std::endl;
    POPULATION_LOG << this->ages[age] << std::endl;
    this->proportion_mature_at_age[i_age_year] =
        this->maturity->evaluate(ages[age]);

    POPULATION_LOG << "p mature set to " << this->proportion_mature_at_age[i_age_year]
             << std::endl;
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
     here year 0 is the estimated initial stock structure and age 0 are recruits
     loops start at zero with if statements inside to specify unique code for
     initial structure and recruitment 0 loops. Could also have started loops at
     1 with initial structure and recruitment setup outside the loops.

     year loop is extended to <= nyears because SSB is calculted as the start of
     the year value and by extending one extra year we get estimates of the
     population structure at the end of the final year. An alternative approach
     would be to keep initial numbers at age in it's own vector and each year to
     include the population structure at the end of the year. This is likely a
     null point given that we are planning to modify to an event/stanza based
     structure in later milestones which will elimitate this confusion by
     explicity referencing the exact date (or period of averaging) at which any
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
         used to show expected stock structure at the end of the model period.
         This is because biomass in year i represents biomass at the start of
         the year.
         Should we add complexity to track more values such as start,
         mid, and end biomass in all years where, start biomass=end biomass of
         the previous year? Referenced above, this is probably not worth
         exploring as later milestone changes will eliminate this confusion.
         */
        if (y < this->nyears) {
          /*
           First thing we need is total mortality aggregated across all fleets
           to inform the subsequent catch and change in numbers at age
           calculations. This is only calculated for years < nyears as these are
           the model estimated years with data. The year loop extends to
           y=nyears so that population numbers at age and SSB can be calculated
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
            // this->numbers_at_age[i_age_year] = this->recruitment->rzero;
            this->unfished_numbers_at_age[i_age_year] =
                fims_math::exp(this->recruitment->log_rzero);
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
            POPULATION_LOG << "Recruitment: " << std::endl;
            CalculateRecruitment(i_age_year, y);
            this->unfished_numbers_at_age[i_age_year] =
                fims_math::exp(this->recruitment->log_rzero);

          } else {
            size_t i_agem1_yearm1 = (y - 1) * nages + (a - 1);
            CalculateNumbersAA(i_age_year, i_agem1_yearm1, a);
            CalculateUnfishedNumbersAA(i_age_year, i_agem1_yearm1, a);
          }
          CalculateBiomass(i_age_year, y, a);
          CalculateSpawningBiomass(i_age_year, y, a);

          POPULATION_LOG << "index age year: " << i_age_year << std::endl;
          CalculateUnfishedBiomass(i_age_year, y, a);
          CalculateUnfishedSpawningBiomass(i_age_year, y, a);
        }

        /*
        Here composition, total catch, and index values are calculated for all
        years with reference data. They are not calculated for y=nyears as there
        is this is just to get final population structure at the end of the
        terminal year.
         */
        if (y < this->nyears) {
          POPULATION_LOG << i_age_year << std::endl;
          CalculateCatchNumbersAA(i_age_year, y, a);

          CalculateCatchWeightAA(y, a);
          POPULATION_LOG << "year " << y << " and age " << a << std::endl;
          CalculateCatch(y, a);
          CalculateIndex(i_age_year, y, a);
        }
      }
    }

    POPULATION_LOG << "NAA\n";
    for (size_t i = 0; i < nyears; i++) {
      for (size_t j = 0; j < nages; j++) {
        POPULATION_LOG << numbers_at_age[i * nages + j] << "\t";
      }
      POPULATION_LOG << "\n";
    }

    POPULATION_LOG << "CAA\n";
    for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
      for (size_t i = 0; i < nyears; i++) {
        for (size_t j = 0; j < nages; j++) {
      POPULATION_LOG << "Fleet " << fleet_ + 1 << "\n";
          POPULATION_LOG << fleets[fleet_]->catch_numbers_at_age[i * nages + j]
                   << "\t";
        }
        POPULATION_LOG << "\n";
      }
    }
  }
};
template <class Type>
uint32_t Population<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
