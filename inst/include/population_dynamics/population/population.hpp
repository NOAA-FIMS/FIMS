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
    /*TODO:
     Review, add functions to evaluate, push vectors back to fleet (or point to
     fleet directly?)
     */

    /**
     * @brief Population class. Contains subpopulations
     * that are divided into generic partitions (eg. sex, area).
     */
    template <typename Type>
    struct Population : public fims_model_object::FIMSObject<Type> {
        static uint32_t id_g; /*!< reference id for population object*/
        size_t nyears; /*!< total number of years in the fishery*/
        size_t nseasons; /*!< total number of seasons in the fishery*/
        size_t nages; /*!< total number of ages in the population*/
        size_t nfleets; /*!< total number of fleets in the fishery*/

        // parameters are estimated; after initialize in create_model, push_back to
        // parameter list - in information.hpp (same for initial F in fleet)
        fims::Vector<Type>
        log_init_naa; /*!< estimated parameter: natural log of numbers at age*/
        fims::Vector<Type> log_M; /*!< estimated parameter: natural log of Natural Mortality*/
        fims::Vector<Type>proportion_female = fims::Vector<Type>(1, static_cast<Type>(0.5)); /*!< proportion female by age */

        // Transformed values
        fims::Vector<Type> M; /*!< transformed parameter: natural mortality*/

        fims::Vector<double> ages; /*!< vector of the ages for referencing*/
        fims::Vector<double> years; /*!< vector of years for referencing*/
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
        fims::Vector<Type> unfished_biomass; /*!< Derived quanity
                                        biomass assuming unfished*/
        fims::Vector<Type> unfished_spawning_biomass; /*!< Derived quanity Spawning
                                                 biomass assuming unfished*/
        fims::Vector<Type> proportion_mature_at_age; /*!< Derived quantity: Proportion
                                                mature at age */
        fims::Vector<Type> total_landings_weight; /*!< Derived quantity: Total landings in weight*/
        fims::Vector<Type> total_landings_numbers; /*!< Derived quantity: Total landings in numbers*/
        fims::Vector<Type> expected_recruitment; /*!< Expected recruitment */
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
        int fleet_id = -999; /*!< id of fleet model object*/
        std::vector<std::shared_ptr<fims_popdy::Fleet<Type>>>
        fleets; /*!< shared pointer to fleet module */

        // Define objective function object to be able to REPORT and ADREPORT

#ifdef TMB_MODEL
        ::objective_function<Type>
        *of; // :: references global namespace, defined in src/FIMS.cpp,
        // available anywhere in the R package
#endif

        // this -> means you're referring to a class member (member of self)

        Population() {
            this->id = Population::id_g++;
        }

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
            std::fill(unfished_spawning_biomass.begin(),
                    unfished_spawning_biomass.end(), static_cast<Type>(0.0));
            std::fill(spawning_biomass.begin(), spawning_biomass.end(), static_cast<Type>(0.0));
            std::fill(total_landings_weight.begin(), total_landings_weight.end(), static_cast<Type>(0.0));
            std::fill(total_landings_numbers.begin(), total_landings_numbers.end(), static_cast<Type>(0.0));
            std::fill(expected_recruitment.begin(), expected_recruitment.end(), static_cast<Type>(0.0));
            std::fill(proportion_mature_at_age.begin(), proportion_mature_at_age.end(),
                    static_cast<Type>(0.0));
            std::fill(mortality_Z.begin(), mortality_Z.end(), static_cast<Type>(0.0));
            std::fill(proportion_female.begin(), proportion_female.end(), static_cast<Type>(0.5));

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
         * life history calculations
         */

        /**
         * @brief Calculates initial numbers at age for index and age
         *
         * @param i_age_year dimension folded index for age and year
         * @param a age index
         */
        inline void CalculateInitialNumbersAA(
                size_t i_age_year, size_t a) { // inline all function unless complicated
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
         * @param i_age_year dimension folded index for age and year
         * @param year the year biomass is being aggregated for
         * @param age the age who's biomass is being added into total biomass
         */
        void CalculateBiomass(size_t i_age_year, size_t year, size_t age) {
            this->biomass[year] +=
                    this->numbers_at_age[i_age_year] * this->weight_at_age[age];
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
            this->unfished_biomass[year] +=
                    this->unfished_numbers_at_age[i_age_year] * this->weight_at_age[age];
        }

        /**
         * @brief Calculates spawning biomass
         *
         * @param i_age_year dimension folded index for age and year
         * @param year the year spawning biomass is being aggregated for
         * @param age the age who's biomass is being added into total spawning biomass
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
         * @param i_age_year dimension folded index for age and year
         * @param year the year recruitment is being calculated for
         * @param i_dev index to log_recruit_dev of vector length nyears-1
         */
        void CalculateRecruitment(size_t i_age_year, size_t year, size_t i_dev) {
            Type phi0 = CalculateSBPR0();

            if (i_dev == this->nyears) {
                this->numbers_at_age[i_age_year] =
                        this->recruitment->evaluate(this->spawning_biomass[year - 1], phi0);
                /*the final year of the time series has no data to inform recruitment
                devs, so this value is set to the mean recruitment.*/
            } else {
                this->numbers_at_age[i_age_year] =
                        this->recruitment->evaluate(this->spawning_biomass[year - 1], phi0) *
                        /*the log_recruit_dev vector does not include a value for year == 0
                        and is of length nyears - 1 where the first position of the vector
                        corresponds to the second year of the time series.*/
                        fims_math::exp(this->recruitment->log_recruit_devs[i_dev - 1]);
                this->expected_recruitment[year] = this->numbers_at_age[i_age_year];
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

            this->proportion_mature_at_age[i_age_year] =
                    this->maturity->evaluate(ages[age]);

        }

        /**
         * @brief Adds to existing expected total landings by fleet
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
         * @brief Calculates landings in numbers at age for each fleet for a given year
         * and age, then adds the value to the expected landings in numbers at age for
         * each fleet
         *
         * @param i_age_year dimension folded index for age and year
         * @param year the year of expected landings composition is being calculated for
         * @param age the age composition is being calculated for
         */
        void CalculateLandingsNumbersAA(size_t i_age_year, size_t year, size_t age) {
            for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {
                // Baranov Catch Equation
                this->fleets[fleet_]->landings_numbers_at_age[i_age_year] +=
                (this->fleets[fleet_]->Fmort[year] *
                    this->fleets[fleet_]->selectivity->evaluate(ages[age])) /
                    this->mortality_Z[i_age_year] *
                    this->numbers_at_age[i_age_year] *
                    (1 - fims_math::exp(-(this->mortality_Z[i_age_year])));
            }
        }

        /**
         * @brief Calculates expected landings weight at age for each fleet for a given
         * year and age
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
         * @param i_age_year dimension folded index for age and year
         * @param year the year of the population index
         * @param age the age of the index that is added into population index
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
         * @brief Calculates index sample in numbers at age for each fleet for
         * a given year and age, then adds the value to the expected index in
         * numbers at age for each fleet
         *
         * @param i_age_year dimension folded index for age and year
         * @param year the year the expected index is being calculated for
         * @param age the age index is being calculated for
         */
        void CalculateIndexNumbersAA(size_t i_age_year, size_t year, size_t age) {
            for (size_t fleet_ = 0; fleet_ < this->nfleets; fleet_++) {

                this->fleets[fleet_]->index_numbers_at_age[i_age_year] +=
                (this->fleets[fleet_]->q.get_force_scalar(year) *
                this->fleets[fleet_]->selectivity->evaluate(ages[age]))*
                this->numbers_at_age[i_age_year];
            }
        }

        /**
         * @brief Calculates expected index weight at age for each fleet for
         * a given year and age
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
             here year 0 is the estimated initial population structure and age 0 are recruits
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
                     used to show expected population structure at the end of the model period.
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
                         to inform the subsequent landings and change in numbers at age
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
                    Here composition, total landings, and index values are calculated for all
                    years with reference data. They are not calculated for y=nyears as there
                    is this is just to get final population structure at the end of the
                    terminal year.
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
    };
    template <class Type>
    uint32_t Population<Type>::id_g = 0;

} // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
