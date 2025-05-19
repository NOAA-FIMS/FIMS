#ifndef FIMS_MODELS_CATACH_AT_AGE_HPP
#define FIMS_MODELS_CATACH_AT_AGE_HPP

#include <set>
#include <regex>

#include "fishery_model_base.hpp"

namespace fims_popdy {

    template<typename Type>
    class CatchAtAge : public FisheryModelBase<Type> {
    public:
        size_t nyears = 0; //max of all populations
        size_t nages = 0; //max of all populations


    public:
        std::set<uint32_t> population_ids;
        std::vector<std::shared_ptr<fims_popdy::Population<Type> > > populations;

        CatchAtAge() : FisheryModelBase<Type>() {
        }

        virtual void Initialize() {
            this->nyears = 0;
            this->nages = 0;
            for (size_t i = 0; i < this->populations.size(); i++) {
                this->nyears = std::max(this->nyears, this->populations[i]->nyears);
                this->nages = std::max(this->nages, this->populations[i]->nages);

                this->populations[i]->derived_quantities["mortality_F"] =
                        fims::Vector<Type>(this->populations[i]->nyears *
                        this->populations[i]->nages);

                this->populations[i]->derived_quantities["mortality_Z"] =
                        fims::Vector<Type>(this->populations[i]->nyears *
                        this->populations[i]->nages);

                this->populations[i]->derived_quantities["weight_at_age"] =
                        fims::Vector<Type>(this->populations[i]->nages);

                this->populations[i]->derived_quantities["numbers_at_age"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1) *
                        this->populations[i]->nages);

                this->populations[i]->derived_quantities["unfished_numbers_at_age"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1) *
                        this->populations[i]->nages);
                this->populations[i]->derived_quantities["biomass"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1));

                this->populations[i]->derived_quantities["spawning_biomass"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1));

                this->populations[i]->derived_quantities["unfished_biomass"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1));

                this->populations[i]->derived_quantities["unfished_spawning_biomass"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1));

                this->populations[i]->derived_quantities["proportion_mature_at_age"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1) *
                        this->populations[i]->nages);

                this->populations[i]->derived_quantities["expected_catch"] =
                        fims::Vector<Type>(this->populations[i]->nyears *
                        this->populations[i]->nfleets);

                this->populations[i]->derived_quantities["expected_recruitment"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1));
            }
        }

        void Prepare() {
            for (size_t p = 0; p < this->populations.size(); p++) {
                populations[p] ->Prepare();
            }
        }

        void AddPopulation(uint32_t id) {
            this->population_ids.insert(id);
        }

        std::set<uint32_t>& GetPopulationIds() {
            return population_ids;
        }

        std::vector<std::shared_ptr<fims_popdy::Population<Type> > >& GetPopulations() {
            return populations;
        }

        void CalculateInitialNumbersAA(size_t i_age_year, size_t a) {
            for (size_t p = 0; p < this->populations.size(); p++) {
                this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] =
                        fims_math::exp(this->populations[p]->log_init_naa[a]);
            }
        }

        void CalculateNumbersAA(
                size_t i_age_year,
                size_t y,
                size_t age) {
            // using Z from previous age/year
            for (size_t p = 0; p < this->populations.size(); p++) {

                size_t i_agem1_yearm1 = (y - 1) * this->populations[p]->nages + (age - 1);
                this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] =
                        this->populations[p]->derived_quantities["numbers_at_age"][i_agem1_yearm1] *
                        (fims_math::exp(-this->populations[p]->derived_quantities["mortality_Z"][i_agem1_yearm1]));

                // Plus group calculation
                if (age == (this->nages - 1)) {
                    this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] =
                            this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] +
                            this->populations[p]->derived_quantities["numbers_at_age"][i_agem1_yearm1 + 1] *
                            (fims_math::exp(-this->populations[p]->derived_quantities["mortality_Z"][i_agem1_yearm1 + 1]));
                }
            }
        }

        void CalculateUnfishedNumbersAA(size_t i_age_year,
                size_t y, size_t age) {

            for (size_t p = 0; p < this->populations.size(); p++) {

                size_t i_agem1_yearm1 = (y - 1) * this->populations[p]->nages + (age - 1);
                // using M from previous age/year
                this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                        this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_agem1_yearm1] *
                        (fims_math::exp(-this->populations[p]->M[i_agem1_yearm1]));

                // Plus group calculation
                if (age == (this->nages - 1)) {
                    this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                            this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_age_year] +
                            this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_agem1_yearm1 + 1] *
                            (fims_math::exp(-this->populations[p]->M[i_agem1_yearm1 + 1]));
                }
            }
        }

        void CalculateMortality(
                size_t i_age_year, size_t year, size_t age) {

            for (size_t p = 0; p < this->populations.size(); p++) {
                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {
                    if (this->populations[p]->fleets[fleet_]->is_survey == false) {
                        this->populations[p]->derived_quantities["mortality_F"][i_age_year] +=
                                this->populations[p]->fleets[fleet_]->Fmort[year] *
                                // evaluate is a member function of the selectivity class
                                this->populations[p]->fleets[fleet_]->selectivity->evaluate(this->populations[p]->ages[age]);
                    }
                }
            }
        }

        void CalculateBiomass(size_t i_age_year, size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {
                this->populations[p]->derived_quantities["biomass"][year] +=
                        this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] *
                        this->populations[p]->weight_at_age[age];
                std::cout << this->populations[p]->ages[age] << " " << this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] << " * "
                        << this->populations[p]->weight_at_age[age] << "\n";
            }
        }

        void CalculateUnfishedBiomass(size_t i_age_year, size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {
                this->populations[p]->derived_quantities["unfished_biomass"][year] +=
                        this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_age_year] *
                        this->populations[p]->weight_at_age[age];
            }

        }

        void CalculateSpawningBiomass(size_t i_age_year, size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {

                this->populations[p]->derived_quantities["spawning_biomass"][year] +=
                        this->populations[p]->proportion_female[age] *
                        this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] *
                        this->populations[p]->proportion_mature_at_age[i_age_year] *
                        this->populations[p]->weight_at_age[age];

                std::cout <<
                        this->populations[p]->proportion_female[age] << " * "
                        << this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] << " * "
                        << this->populations[p]->proportion_mature_at_age[i_age_year] << " * "
                        << this->populations[p]->weight_at_age[age] << std::endl;
            }

        }

        void CalculateUnfishedSpawningBiomass(size_t i_age_year, size_t year,
                size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {
                this->populations[p]->derived_quantities["unfished_spawning_biomass"][year] +=
                        this->populations[p]->proportion_female[age] *
                        this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_age_year] *
                        this->populations[p]->proportion_mature_at_age[i_age_year] *
                        this->populations[p]->weight_at_age[age];
            }
        }

        std::vector<Type> CalculateSBPR0() {

            std::vector<Type> phis;
            for (size_t p = 0; p < this->populations.size(); p++) {

                std::vector<Type> numbers_spr(this->nages, 1.0);
                Type phi_0 = 0.0;
                phi_0 += numbers_spr[0] * this->populations[p]->proportion_female[0] *
                        this->populations[p]->proportion_mature_at_age[0] *
                        this->populations[p]->growth->evaluate(this->populations[p]->ages[0]);
                for (size_t a = 1; a < (this->populations[p]->nages - 1); a++) {
                    numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-this->populations[p]->M[a]);
                    phi_0 += numbers_spr[a] * this->populations[p]->proportion_female[a] *
                            this->populations[p]->proportion_mature_at_age[a] *
                            this->populations[p]->growth->evaluate(this->populations[p]->ages[a]);
                }

                numbers_spr[this->populations[p]->nages - 1] =
                        (numbers_spr[this->populations[p]->nages - 2] * fims_math::exp(-this->populations[p]->M[this->populations[p]->nages - 2])) /
                        (1 - fims_math::exp(-this->populations[p]->M[this->populations[p]->nages - 1]));
                phi_0 += numbers_spr[this->populations[p]->nages - 1] *
                        this->populations[p]->proportion_female[this->populations[p]->nages - 1] *
                        this->populations[p]->proportion_mature_at_age[this->populations[p]->nages - 1] *
                        this->populations[p]->growth->evaluate(this->populations[p]->ages[this->populations[p]->nages - 1]);
                phis.push_back(phi_0);
                std::cout << "phi_0 = " << phi_0 << "\n";
            }

            return phis;
        }

        void CalculateRecruitment(size_t i_age_year, size_t year, size_t i_dev) {

            std::vector<Type> phi0 = CalculateSBPR0();
            for (size_t p = 0; p < this->populations.size(); p++) {

                //                std::cout<<this->populations[p]->derived_quantities["numbers_at_age"].size()<<"\n";
                if (i_dev == this->populations[p]->nyears) {
                    this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] =
                            this->populations[p]->recruitment->evaluate(
                            this->populations[p]->derived_quantities["spawning_biomass"][year - 1], phi0[p]);
                    /*the final year of the time series has no data to inform recruitment
                    devs, so this value is set to the mean recruitment.*/
                } else {

                    std::cout << "this->populations[p]->derived_quantities[\"spawning_biomass\"][year - 1] = " << this->populations[p]->derived_quantities["spawning_biomass"][year - 1] << std::endl;
                    this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] =
                            this->populations[p]->recruitment->evaluate(
                            this->populations[p]->derived_quantities["spawning_biomass"][year - 1], phi0[p]) *
                            /*the log_recruit_dev vector does not include a value for year == 0
                            and is of length nyears - 1 where the first position of the vector
                            corresponds to the second year of the time series.*/
                            fims_math::exp(this->populations[p]->recruitment->log_recruit_devs[i_dev - 1]);

                    this->populations[p]->derived_quantities["expected_recruitment"][year] =
                            this->populations[p]->derived_quantities["numbers_at_age"][i_age_year];


                }

            }
        }

        void CalculateCatch(size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {

                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {
                    if (this->populations[p]->fleets[fleet_]->is_survey == false) {
                        size_t index_yf = year * this->populations[p]->nfleets +
                                fleet_; // index by fleet and years to dimension fold
                        size_t i_age_year = year * this->populations[p]->nages + age;

                        this->populations[p]->derived_quantities["expected_catch"][index_yf] +=
                                this->populations[p]->fleets[fleet_]->catch_weight_at_age[i_age_year];

                        this->populations[p]->fleets[fleet_]->expected_catch[year] +=
                                this->populations[p]->fleets[fleet_]->catch_weight_at_age[i_age_year];
                    }
                }

            }
        }

        void CalculateIndex(size_t i_age_year, size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {

                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {
                    Type index_;
                    // I = qN (N is total numbers), I is an index in numbers
                    if (this->populations[p]->fleets[fleet_]->is_survey == false) {
                        index_ = this->populations[p]->fleets[fleet_]->catch_numbers_at_age[i_age_year] *
                                this->populations[p]->derived_quantities["weight_at_age"][age];
                    } else {
                        index_ = this->populations[p]->fleets[fleet_]->q.get_force_scalar(year) *
                                this->populations[p]->fleets[fleet_]->selectivity->evaluate(this->populations[p]->ages[age]) *
                                this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] *
                                this->populations[p]->derived_quantities["weight_at_age"][age]; // this->weight_at_age[age];
                    }
                    this->populations[p]->fleets[fleet_]->expected_index[year] += index_;
                }
            }
        }

        void CalculateCatchNumbersAA(size_t i_age_year, size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {

                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {
                    // make an intermediate value in order to set multiple members (of
                    // current and fleet objects) to that value.
                    Type catch_; // catch_ is used to avoid using the c++ keyword catch
                    // Baranov Catch Equation
                    if (this->populations[p]->fleets[fleet_]->is_survey == false) {
                        catch_ = (this->populations[p]->fleets[fleet_]->Fmort[year] *
                                this->populations[p]->fleets[fleet_]->selectivity->evaluate(this->populations[p]->ages[age])) /
                                this->populations[p]->derived_quantities["mortality_Z"][i_age_year] *
                                this->populations[p]->derived_quantities["numbers_at_age"][i_age_year] *
                                (1 - fims_math::exp(-(this->populations[p]->derived_quantities["mortality_Z"][i_age_year])));
                    } else {
                        catch_ = (this->populations[p]->fleets[fleet_]->selectivity->evaluate(this->populations[p]->ages[age])) *
                                this->populations[p]->derived_quantities["numbers_at_age"][i_age_year];
                    }

                    // this->catch_numbers_at_age[i_age_yearf] += catch_;
                    // catch_numbers_at_age for the fleet module has different
                    // dimensions (year/age, not year/fleet/age)
                    this->populations[p]->fleets[fleet_]->catch_numbers_at_age[i_age_year] += catch_;
                }

            }
        }

        void CalculateCatchWeightAA(size_t year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {

                int i_age_year = year * this->populations[p]->nages + age;
                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {

                    this->populations[p]->fleets[fleet_]->catch_weight_at_age[i_age_year] =
                            this->populations[p]->fleets[fleet_]->catch_numbers_at_age[i_age_year] *
                            this->populations[p]->derived_quantities["weight_at_age"][age];

                }

            }
        }

        void CalculateMaturityAA(size_t i_age_year, size_t age) {
            for (size_t p = 0; p < this->populations.size(); p++) {
                this->populations[p]->proportion_mature_at_age[i_age_year] =
                        this->populations[p]->maturity->evaluate(this->populations[p]->ages[age]);
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
                std::cout << "\n";
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
                            for (size_t p = 0; p < this->populations.size(); p++) {
                                this->populations[p]->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                                        fims_math::exp(this->populations[p]->recruitment->log_rzero[0]);
                            }
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
                        for (size_t p = 0; p < this->populations.size(); p++) {
                            this->populations[p]->derived_quantities["expected_recruitment"][i_age_year] =
                                    this->populations[p]->derived_quantities["numbers_at_age"][i_age_year];
                        }
                    } else {
                        if (a == 0) {
                            // Set the nrecruits for age a=0 year y (use pointers instead of
                            // functional returns) assuming fecundity = 1 and 50:50 sex ratio
                            CalculateRecruitment(i_age_year, y, y);
                            for (size_t p = 0; p < this->populations.size(); p++) {
                                this->populations[p]->unfished_numbers_at_age[i_age_year] =
                                        fims_math::exp(this->populations[p]->recruitment->log_rzero[0]);
                            }

                        } else {
#warning this segment was restructured by matthew
                            //because populations may differ in the values of nyearsm nages
                            //the calculation for the i_agem1_yearm1 is now in the methods 
                            //listed below, within the population loop
                            //size_t i_agem1_yearm1 = (y - 1) * this->nages + (a - 1);
                            CalculateNumbersAA(i_age_year, y, a);
                            CalculateUnfishedNumbersAA(i_age_year, y, a);

                        }
                        CalculateBiomass(i_age_year, y, a);
                        CalculateSpawningBiomass(i_age_year, y, a);

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
                        CalculateCatchNumbersAA(i_age_year, y, a);

                        CalculateCatchWeightAA(y, a);
                        CalculateCatch(y, a);
                        CalculateIndex(i_age_year, y, a);
                    }
                }
            }

        }


    };


}



#endif
