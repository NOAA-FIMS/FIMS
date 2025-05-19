#ifndef FIMS_MODELS_CATACH_AT_AGE_HPP
#define FIMS_MODELS_CATACH_AT_AGE_HPP

#include <set>
#include <regex>

#include "fishery_model_base.hpp"

namespace fims_popdy
{

    /**
     * A proxy class for fleet objects. This class is used to access the derived quantities of
     * the fleet object from a population object. This is used to reduce the
     * overhead of hashing for the derived quantities map.
     */
    template <typename Type>
    struct CAAFleetProxy
    {
        std::shared_ptr<fims_popdy::Fleet<Type>> fleet;
        fims::Vector<Type> &catch_at_age;
        fims::Vector<Type> &catch_numbers_at_age;
        fims::Vector<Type> &catch_numbers_at_length;
        fims::Vector<Type> &proportion_catch_numbers_at_age;
        fims::Vector<Type> &proportion_catch_numbers_at_length;
        fims::Vector<Type> &age_length_conversion_matrix;
        fims::Vector<Type> &catch_weight_at_age;
        fims::Vector<Type> &catch_index;
        fims::Vector<Type> &age_composition;
        fims::Vector<Type> &length_composition;
        fims::Vector<Type> &expected_catch;
        fims::Vector<Type> &expected_index;
        fims::Vector<Type> &log_expected_index;

        CAAFleetProxy(std::shared_ptr<fims_popdy::Fleet<Type>> fleet) : fleet(fleet),
                                                                        catch_at_age(fleet->derived_quantities["catch_at_age"]),
                                                                        catch_numbers_at_age(fleet->derived_quantities["catch_numbers_at_age"]),
                                                                        catch_numbers_at_length(fleet->derived_quantities["catch_numbers_at_length"]),
                                                                        proportion_catch_numbers_at_age(fleet->derived_quantities["proportion_catch_numbers_at_age"]),
                                                                        proportion_catch_numbers_at_length(fleet->derived_quantities["proportion_catch_numbers_at_length"]),
                                                                        age_length_conversion_matrix(fleet->derived_quantities["age_length_conversion_matrix"]),
                                                                        catch_weight_at_age(fleet->derived_quantities["catch_weight_at_age"]),
                                                                        catch_index(fleet->derived_quantities["catch_index"]),
                                                                        age_composition(fleet->derived_quantities["age_composition"]),
                                                                        length_composition(fleet->derived_quantities["length_composition"]),
                                                                        expected_catch(fleet->derived_quantities["expected_catch"]),
                                                                        expected_index(fleet->derived_quantities["expected_index"]),
                                                                        log_expected_index(fleet->derived_quantities["log_expected_index"])
        {
        }
    };

    /**
     *  A proxy class for population objects. This class is used to access the derived quantities of
     *  the population object from the CatachAtAge object. This is used to reduce the
     *  overhead of hashing for the derived quantities map.
     */
    template <typename Type>
    struct CAAPopulationProxy
    {

        std::shared_ptr<fims_popdy::Population<Type>> population;
        std::vector<CAAFleetProxy<Type>> fleets;
        fims::Vector<Type> &mortality_F;
        fims::Vector<Type> &mortality_Z;
        fims::Vector<Type> &weight_at_age;
        fims::Vector<Type> &numbers_at_age;
        fims::Vector<Type> &unfished_numbers_at_age;
        fims::Vector<Type> &biomass;
        fims::Vector<Type> &spawning_biomass;
        fims::Vector<Type> &unfished_biomass;
        fims::Vector<Type> &unfished_spawning_biomass;
        fims::Vector<Type> &proportion_mature_at_age;
        fims::Vector<Type> &expected_catch;
        fims::Vector<Type> &expected_recruitment;
        fims::Vector<Type> &sum_selectivity;

        CAAPopulationProxy(std::shared_ptr<fims_popdy::Population<Type>> population) : population(population),
                                                                                       mortality_F(population->derived_quantities["mortality_F"]),
                                                                                       mortality_Z(population->derived_quantities["mortality_Z"]),
                                                                                       weight_at_age(population->derived_quantities["weight_at_age"]),
                                                                                       numbers_at_age(population->derived_quantities["numbers_at_age"]),
                                                                                       unfished_numbers_at_age(population->derived_quantities["unfished_numbers_at_age"]),
                                                                                       biomass(population->derived_quantities["biomass"]),
                                                                                       spawning_biomass(population->derived_quantities["spawning_biomass"]),
                                                                                       unfished_biomass(population->derived_quantities["unfished_biomass"]),
                                                                                       unfished_spawning_biomass(population->derived_quantities["unfished_spawning_biomass"]),
                                                                                       proportion_mature_at_age(population->derived_quantities["proportion_mature_at_age"]),
                                                                                       expected_catch(population->derived_quantities["expected_catch"]),
                                                                                       expected_recruitment(population->derived_quantities["expected_recruitment"]),
                                                                                       sum_selectivity(population->derived_quantities["sum_selectivity"])

        {

            // fill the fleets vector with fleet proxies
            for (size_t i = 0; i < population->fleets.size(); i++)
            {
                this->fleets.push_back(CAAFleetProxy<Type>(population->fleets[i]));
            }
        }
    };

    // TODO: add a function to compute length composition
    template <typename Type>
    class CatchAtAge : public FisheryModelBase<Type>
    {
    public:
        std::vector<CAAPopulationProxy<Type>> populations_proxies;
        std::vector<CAAFleetProxy<Type>> fleets_proxies;
        std::string name_m;
        std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type>>> fleets; // unique instances to eliminate duplicate initialization
        typedef typename std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator fleet_iterator;
        typedef typename std::map<std::string, fims::Vector<Type>>::iterator derived_quantities_iterator;

    public:
        /**
         * Constructor for the CatchAtAge class. This constructor initializes the
         * name of the model and sets the id of the model.
         */
        CatchAtAge() : FisheryModelBase<Type>()
        {
            std::stringstream ss;
            ss << "caa_" << this->GetId() << "_";
            this->name_m = ss.str();
        }

        /**
         * This function is called once at the beginning of the model run. It initializes the derived
         * quantities for the populations and fleets.
         */
        virtual void Initialize()
        {

            for (size_t i = 0; i < this->populations.size(); i++)
            {

                this->populations[i]->derived_quantities["mortality_F"] =
                    fims::Vector<Type>(this->populations[i]->nyears *
                                       this->populations[i]->nages);

                this->populations[i]->derived_quantities["mortality_Z"] =
                    fims::Vector<Type>(this->populations[i]->nyears *
                                       this->populations[i]->nages);

                this->populations[i]->derived_quantities["weight_at_age"] =
                    fims::Vector<Type>(this->populations[i]->nyears * this->populations[i]->nages);

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

                this->populations[i]->derived_quantities["sum_selectivity"] =
                    fims::Vector<Type>(this->populations[i]->nyears * this->populations[i]->nages);

                for (size_t j = 0; j < this->populations[i]->fleets.size(); j++)
                {

                    this->fleets[this->populations[i]->fleets[j]->id] = this->populations[i]->fleets[j];
                }
                // push back the population proxy
                this->populations_proxies.push_back(CAAPopulationProxy<Type>(this->populations[i]));
            }

            for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end(); ++it)
            {

                // initialize derive quantities
                (*it).second->derived_quantities["catch_at_age"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nages);

                (*it).second->derived_quantities["catch_numbers_at_age"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nages);

                (*it).second->derived_quantities["catch_numbers_at_length"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nlengths);

                (*it).second->derived_quantities["proportion_catch_numbers_at_age"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nages);

                (*it).second->derived_quantities["proportion_catch_numbers_at_length"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nlengths);

                (*it).second->derived_quantities["catch_weight_at_age"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nages);

                (*it).second->derived_quantities["catch_index"] =
                    fims::Vector<Type>((*it).second->nyears);

                (*it).second->derived_quantities["expected_catch"] =
                    fims::Vector<Type>((*it).second->nyears);

                (*it).second->derived_quantities["expected_index"] =
                    fims::Vector<Type>((*it).second->nyears);

                (*it).second->derived_quantities["log_expected_index"] =
                    fims::Vector<Type>((*it).second->nyears);

                (*it).second->derived_quantities["age_composition"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nages);

                (*it).second->derived_quantities["length_composition"] =
                    fims::Vector<Type>((*it).second->nyears *
                                       (*it).second->nlengths);
                // add fleet proxy
                this->fleets_proxies.push_back(CAAFleetProxy<Type>((*it).second));
            }
        }

        /**
         * This function is used to convert the derived quantities of a population or fleet
         * to a JSON string. This function is used to create the JSON output for the
         * CatchAtAge model.
         */
        std::string DerivedQuantityToJSON(derived_quantities_iterator it)
        {
            fims::Vector<Type> &dq = (*it).second;
            std::stringstream ss;

            ss << "{\n";
            ss << "\"name\":\"" << (*it).first << "\",\n";
            ss << "\"values\":[";

            if (dq.size() > 0)
            {
                for (size_t i = 0; i < dq.size() - 1; i++)
                {
                    ss << dq[i] << ", ";
                }
                ss << dq[dq.size() - 1] << "]\n";
            }
            else
            {
                ss << "]\n";
            }

            ss << "}";
            return ss.str();
        }

        /**
         * This function is used to convert the derived quantities of a population or fleet
         * to a JSON string.
         */
        std::string ToJSON()
        {
            std::stringstream ss;
            ss << "{\n";
            ss << "\"populations\" : [\n";
            for (size_t p = 0; p < this->populations.size(); p++)
            {
                ss << "{\n";
                ss << "\"id\": " << this->populations[p]->id << ",\n";

                ss << "\"derived_quantities\": [\n";
                if (this->populations[p]->derived_quantities.size() > 0)
                {
                    typename fims_popdy::Population<Type>::derived_quantities_iterator it;
                    typename fims_popdy::Population<Type>::derived_quantities_iterator end_it;
                    typename fims_popdy::Population<Type>::derived_quantities_iterator second_to_last;

                    it = this->populations[p]->derived_quantities.begin();
                    end_it = this->populations[p]->derived_quantities.end();
                    second_to_last = this->populations[p]->derived_quantities.end();
                    if (it != end_it)
                    {
                        second_to_last--;
                    }

                    for (; it != second_to_last; ++it)
                    {

                        ss << this->DerivedQuantityToJSON(it) << ",\n";
                    }
                    //                    ++it;
                    ss << this->DerivedQuantityToJSON(second_to_last) << "\n]}\n";
                }
            }
            ss << "],\n";
            fleet_iterator fit;
            ss << "\"fleets\": [\n";
            for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit)
            {
                ss << "{\"id\": " << (*fit).second->id << ",\n";
                if ((*fit).second->derived_quantities.size() > 0)
                {
                    typename fims_popdy::Fleet<Type>::derived_quantities_iterator it;
                    typename fims_popdy::Fleet<Type>::derived_quantities_iterator end_it;
                    typename fims_popdy::Fleet<Type>::derived_quantities_iterator second_to_last;

                    it = (*fit).second->derived_quantities.begin();
                    end_it = (*fit).second->derived_quantities.end();
                    second_to_last = (*fit).second->derived_quantities.end();
                    if (it != end_it)
                    {
                        second_to_last--;
                    }

                    for (; it != second_to_last; ++it)
                    {
                        std::cout << (*it).first << std::endl;
                        ss << this->DerivedQuantityToJSON(it) << ",\n";
                    }
                    std::cout << (*second_to_last).first << std::endl;

                    ss << this->DerivedQuantityToJSON(second_to_last) << "\n]}\n";
                }
            }
            ss << "}";

            return ss.str();
        }

        /**
         * This function is used to reset the derived quantities of a population or fleet
         * to a given value.
         */
        virtual void Prepare()
        {
            for (size_t p = 0; p < this->populations.size(); p++)
            {
                this->populations[p]->Prepare();

                typename fims_popdy::Population<Type>::derived_quantities_iterator it;
                for (it = this->populations[p]->derived_quantities.begin();
                     it != this->populations[p]->derived_quantities.end(); it++)
                {
                    fims::Vector<Type> &dq = (*it).second;
                    this->ResetVector(dq);
                }
            }

            for (size_t p = 0; p < this->populations.size(); p++)
            {
                std::shared_ptr<fims_popdy::Population<Type>> &population =
                    this->populations[p];
                // Transformation Section
                for (size_t age = 0; age < population->nages; age++)
                {
                    for (size_t year = 0; year < population->nyears; year++)
                    {
                        size_t i_age_year = age * population->nyears + year;
                        population->M[i_age_year] = fims_math::exp(population->log_M[i_age_year]);
                        // mortality_F is a fims::Vector and therefore needs to be filled
                        // within a loop
                        population->derived_quantities["mortality_F"][i_age_year] = 0.0;
                        population->derived_quantities["weight_at_age"][i_age_year] = population->growth->evaluate(population->ages[age]);
                    }
                }
            }
        }
        /**
         * This function is used to add a population id to the set of population ids.
         */
        void AddPopulation(uint32_t id)
        {
            this->population_ids.insert(id);
        }

        /**
         *
         */
        std::set<uint32_t> &GetPopulationIds()
        {
            return this->population_ids;
        }

        /**
         * This function is used to get the populations of the model. It returns a
         * vector of shared pointers to the populations.
         * @return std::vector<std::shared_ptr<fims_popdy::Population<Type>>>&
         */
        std::vector<std::shared_ptr<fims_popdy::Population<Type>>> &GetPopulations()
        {
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
            size_t i_age_year, size_t a)
        {

            population->derived_quantities["numbers_at_age"][i_age_year] =
                fims_math::exp(population->log_init_naa[a]);
        }

        void CalculateInitialNumbersAA(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year, size_t a)
        {

            population_proxy.numbers_at_age[i_age_year] =
                fims_math::exp(population_proxy.population->log_init_naa[a]);
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
         * @return void
         */
        void CalculateNumbersAA(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t i_agem1_yearm1,
            size_t age)
        {
            // using Z from previous age/year

            population->derived_quantities["numbers_at_age"][i_age_year] =
                population->derived_quantities["numbers_at_age"][i_agem1_yearm1] *
                (fims_math::exp(-population->derived_quantities["mortality_Z"][i_agem1_yearm1]));

            // Plus group calculation
            if (age == (population->nages - 1))
            {
                population->derived_quantities["numbers_at_age"][i_age_year] =
                    population->derived_quantities["numbers_at_age"][i_age_year] +
                    population->derived_quantities["numbers_at_age"][i_agem1_yearm1 + 1] *
                        (fims_math::exp(-population->derived_quantities["mortality_Z"][i_agem1_yearm1 + 1]));
            }
        }

        void CalculateNumbersAA(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t i_agem1_yearm1,
            size_t age)
        {
            // using Z from previous age/year

            population_proxy.numbers_at_age[i_age_year] =
                population_proxy.numbers_at_age[i_agem1_yearm1] *
                (fims_math::exp(-population_proxy.mortality_Z[i_agem1_yearm1]));

            // Plus group calculation
            if (age == (population_proxy.population->nages - 1))
            {
                population_proxy.numbers_at_age[i_age_year] =
                    population_proxy.numbers_at_age[i_age_year] +
                    population_proxy.numbers_at_age[i_agem1_yearm1 + 1] *
                        (fims_math::exp(-population_proxy.mortality_Z[i_agem1_yearm1 + 1]));
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
         * @return void
         */
        void CalculateUnfishedNumbersAA(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t i_agem1_yearm1,
            size_t age)
        {

            // using M from previous age/year
            population->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                population->derived_quantities["unfished_numbers_at_age"][i_agem1_yearm1] *
                (fims_math::exp(-population->M[i_agem1_yearm1]));

            // Plus group calculation
            if (age == (population->nages - 1))
            {
                population->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                    population->derived_quantities["unfished_numbers_at_age"][i_age_year] +
                    population->derived_quantities["unfished_numbers_at_age"][i_agem1_yearm1 + 1] *
                        (fims_math::exp(-population->M[i_agem1_yearm1 + 1]));
            }
        }

        void CalculateUnfishedNumbersAA(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t i_agem1_yearm1,
            size_t age)
        {

            // using M from previous age/year
            population_proxy.unfished_numbers_at_age[i_age_year] =
                population_proxy.unfished_numbers_at_age[i_agem1_yearm1] *
                (fims_math::exp(-population_proxy.population->M[i_agem1_yearm1]));

            // Plus group calculation
            if (age == (population_proxy.population->nages - 1))
            {
                population_proxy.unfished_numbers_at_age[i_age_year] =
                    population_proxy.unfished_numbers_at_age[i_age_year] +
                    population_proxy.unfished_numbers_at_age[i_agem1_yearm1 + 1] *
                        (fims_math::exp(-population_proxy.population->M[i_agem1_yearm1 + 1]));
            }
        }
        /**
         * * This method is used to calculate the mortality for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the mortality for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateMortality(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
            {
                if (population->fleets[fleet_]->is_survey == false)
                {
                    // evaluate is a member function of the selectivity class
                    Type s = population->fleets[fleet_]->selectivity->evaluate(population->ages[age]);

                    population->derived_quantities["mortality_F"][i_age_year] +=
                        population->fleets[fleet_]->Fmort[year] * s;

                    population->derived_quantities["sum_selectivity"][i_age_year] += s;
                }
            }
            population->derived_quantities["mortality_Z"][i_age_year] =
                population->M[i_age_year] + population->derived_quantities["mortality_F"][i_age_year];
        }

        void CalculateMortality(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population_proxy.population->nfleets; fleet_++)
            {
                if (population_proxy.fleets[fleet_].fleet->is_survey == false)
                {
                    // evaluate is a member function of the selectivity class
                    Type s = population_proxy.population->fleets[fleet_]->selectivity->evaluate(population_proxy.population->ages[age]);

                    population_proxy.mortality_F[i_age_year] +=
                        population_proxy.population->fleets[fleet_]->Fmort[year] * s;

                    population_proxy.sum_selectivity[i_age_year] += s;
                }
            }
            population_proxy.mortality_Z[i_age_year] =
                population_proxy.population->M[i_age_year] + population_proxy.mortality_F[i_age_year];
        }

        /**
         * * This method is used to calculate the biomass for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the biomass for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateBiomass(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            population->derived_quantities["biomass"][year] +=
                population->derived_quantities["numbers_at_age"][i_age_year] *
                population->derived_quantities["weight_at_age"][age];
        }

        void CalculateBiomass(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            population_proxy.biomass[year] +=
                population_proxy.numbers_at_age[i_age_year] *
                population_proxy.weight_at_age[age];
        }

        /**
         * * This method is used to calculate the unfished biomass for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the unfished biomass for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateUnfishedBiomass(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            population->derived_quantities["unfished_biomass"][year] +=
                population->derived_quantities["unfished_numbers_at_age"][i_age_year] *
                population->derived_quantities["weight_at_age"][age];
        }

        void CalculateUnfishedBiomass(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            population_proxy.unfished_biomass[year] +=
                population_proxy.unfished_numbers_at_age[i_age_year] *
                population_proxy.weight_at_age[age];
        }

        /**
         * * This method is used to calculate the spawning biomass for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the spawning biomass for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateSpawningBiomass(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            population->derived_quantities["spawning_biomass"][year] +=
                population->proportion_female[age] *
                population->derived_quantities["numbers_at_age"][i_age_year] *
                population->derived_quantities["proportion_mature_at_age"][i_age_year] *
                population->derived_quantities["weight_at_age"][age];
        }
        /**
         * * This method is used to calculate the spawning biomass for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the spawning biomass for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         */
        void CalculateSpawningBiomass(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            population_proxy.spawning_biomass[year] +=
                population_proxy.population->proportion_female[age] *
                population_proxy.numbers_at_age[i_age_year] *
                population_proxy.proportion_mature_at_age[i_age_year] *
                population_proxy.weight_at_age[age];
        }

        /**
         * This method is used to calculate the unfished spawning biomass for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the unfished spawning biomass for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateUnfishedSpawningBiomass(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {
            population->derived_quantities["unfished_spawning_biomass"][year] +=
                population->proportion_female[age] *
                population->derived_quantities["unfished_numbers_at_age"][i_age_year] *
                population->derived_quantities["proportion_mature_at_age"][i_age_year] *
                population->derived_quantities["weight_at_age"][age];
        }

        void CalculateUnfishedSpawningBiomass(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {
            population_proxy.unfished_spawning_biomass[year] +=
                population_proxy.population->proportion_female[age] *
                population_proxy.unfished_numbers_at_age[i_age_year] *
                population_proxy.proportion_mature_at_age[i_age_year] *
                population_proxy.weight_at_age[age];
        }

        /**
         * This method is used to calculate the spawning biomass per recruit for a population. It takes a
         * population object.
         */
        Type CalculateSBPR0(
            std::shared_ptr<fims_popdy::Population<Type>> &population)
        {
            std::vector<Type> numbers_spr(population->nages, 1.0);
            Type phi_0 = 0.0;
            phi_0 += numbers_spr[0] * population->proportion_female[0] *
                     population->derived_quantities["proportion_mature_at_age"][0] *
                     population->growth->evaluate(population->ages[0]);
            for (size_t a = 1; a < (population->nages - 1); a++)
            {
                numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-population->M[a]);
                phi_0 += numbers_spr[a] * population->proportion_female[a] *
                         population->derived_quantities["proportion_mature_at_age"][a] *
                         population->growth->evaluate(population->ages[a]);
            }

            numbers_spr[population->nages - 1] =
                (numbers_spr[population->nages - 2] * fims_math::exp(-population->M[population->nages - 2])) /
                (1 - fims_math::exp(-population->M[population->nages - 1]));
            phi_0 += numbers_spr[population->nages - 1] *
                     population->proportion_female[population->nages - 1] *
                     population->derived_quantities["proportion_mature_at_age"][population->nages - 1] *
                     population->growth->evaluate(population->ages[population->nages - 1]);

            return phi_0;
        }

        Type CalculateSBPR0(
            CAAPopulationProxy<Type> &population_proxy)
        {
            std::vector<Type> numbers_spr(population_proxy.population->nages, 1.0);
            Type phi_0 = 0.0;
            phi_0 += numbers_spr[0] * population_proxy.population->proportion_female[0] *
                     population_proxy.proportion_mature_at_age[0] *
                     population_proxy.population->growth->evaluate(population_proxy.population->ages[0]);
            for (size_t a = 1; a < (population_proxy.population->nages - 1); a++)
            {
                numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-population_proxy.population->M[a]);
                phi_0 += numbers_spr[a] * population_proxy.population->proportion_female[a] *
                         population_proxy.proportion_mature_at_age[a] *
                         population_proxy.population->growth->evaluate(population_proxy.population->ages[a]);
            }

            numbers_spr[population_proxy.population->nages - 1] =
                (numbers_spr[population_proxy.population->nages - 2] * fims_math::exp(-population_proxy.population->M[population_proxy.population->nages - 2])) /
                (1 - fims_math::exp(-population_proxy.population->M[population_proxy.population->nages - 1]));
            phi_0 += numbers_spr[population_proxy.population->nages - 1] *
                     population_proxy.population->proportion_female[population_proxy.population->nages - 1] *
                     population_proxy.proportion_mature_at_age[population_proxy.population->nages - 1] *
                     population_proxy.population->growth->evaluate(population_proxy.population->ages[population_proxy.population->nages - 1]);

            return phi_0;
        }
        /**
         * This method is used to calculate the recruitment for a population. 
         *
         */
        void CalculateRecruitment(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t i_dev)
        {

            Type phi0 = CalculateSBPR0(population);

            if (i_dev == population->nyears)
            {
                population->derived_quantities["numbers_at_age"][i_age_year] =
                    population->recruitment->evaluate(population->derived_quantities["spawning_biomass"][year - 1], phi0);
                /*the final year of the time series has no data to inform recruitment
                devs, so this value is set to the mean recruitment.*/
            }
            else
            {
                population->derived_quantities["numbers_at_age"][i_age_year] =
                    population->recruitment->evaluate(population->derived_quantities["spawning_biomass"][year - 1], phi0) *
                    /*the log_recruit_dev vector does not include a value for year == 0
                    and is of length nyears - 1 where the first position of the vector
                    corresponds to the second year of the time series.*/
                    fims_math::exp(population->recruitment->log_recruit_devs[i_dev - 1]);

                population->derived_quantities["expected_recruitment"][year] =
                    population->derived_quantities["numbers_at_age"][i_age_year];
            }
        }

        void CalculateRecruitment(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t i_dev)
        {

            Type phi0 = CalculateSBPR0(population_proxy);

            if (i_dev == population_proxy.population->nyears)
            {
                population_proxy.numbers_at_age[i_age_year] =
                    population_proxy.population->recruitment->evaluate(population_proxy.spawning_biomass[year - 1], phi0);
                /*the final year of the time series has no data to inform recruitment
                devs, so this value is set to the mean recruitment.*/
            }
            else
            {
                population_proxy.numbers_at_age[i_age_year] =
                    population_proxy.population->recruitment->evaluate(population_proxy.spawning_biomass[year - 1], phi0) *
                    /*the log_recruit_dev vector does not include a value for year == 0
                    and is of length nyears - 1 where the first position of the vector
                    corresponds to the second year of the time series.*/
                    fims_math::exp(population_proxy.population->recruitment->log_recruit_devs[i_dev - 1]);

                population_proxy.expected_recruitment[year] =
                    population_proxy.numbers_at_age[i_age_year];
            }
        }

        /**
         * This method is used to calculate the catch for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the catch for that population.
         * @param population
         * @param year
         * @param age
         * @return void
         */
        void CalculateCatch(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
            {
                if (population->fleets[fleet_]->is_survey == false)
                {
                    size_t index_yf = year * population->nfleets +
                                      fleet_; // index by fleet and years to dimension fold
                    size_t i_age_year = year * population->nages + age;

                    population->derived_quantities["expected_catch"][index_yf] +=
                        population->fleets[fleet_]->derived_quantities["catch_weight_at_age"][i_age_year];

                    population->fleets[fleet_]->derived_quantities["expected_catch"][year] +=
                        population->fleets[fleet_]->derived_quantities["catch_weight_at_age"][i_age_year];
                }
            }
        }

        void CalculateCatch(
            CAAPopulationProxy<Type> &population_proxy,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population_proxy.population->nfleets; fleet_++)
            {
                if (population_proxy.population->fleets[fleet_]->is_survey == false)
                {
                    size_t index_yf = year * population_proxy.population->nfleets +
                                      fleet_; // index by fleet and years to dimension fold
                    size_t i_age_year = year * population_proxy.population->nages + age;

                    population_proxy.fleets[fleet_].expected_catch[index_yf] +=
                        population_proxy.fleets[fleet_].catch_weight_at_age[i_age_year];

                    population_proxy.fleets[fleet_].expected_catch[year] +=
                        population_proxy.fleets[fleet_].catch_weight_at_age[i_age_year];
                }
            }
        }

        /**
         * This method is used to calculate the catch index for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the index for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateIndex(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
            {
                Type index_;
                // I = qN (N is total numbers), I is an index in numbers
                if (population->fleets[fleet_]->is_survey == false)
                {
                    index_ = population->fleets[fleet_]->catch_numbers_at_age[i_age_year] *
                             population->derived_quantities["weight_at_age"][age];
                }
                else
                {
                    index_ = population->fleets[fleet_]->q.get_force_scalar(year) *
                             population->fleets[fleet_]->selectivity->evaluate(population->ages[age]) *
                             population->derived_quantities["numbers_at_age"][i_age_year] *
                             population->derived_quantities["weight_at_age"][age]; // this->weight_at_age[age];
                }
                population->fleets[fleet_]->derived_quantities["expected_index"][year] += index_;
            }
        }

        void CalculateIndex(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population_proxy.population->nfleets; fleet_++)
            {
                Type index_;
                // I = qN (N is total numbers), I is an index in numbers
                if (population_proxy.population->fleets[fleet_]->is_survey == false)
                {
                    index_ = population_proxy.fleets[fleet_].catch_numbers_at_age[i_age_year] *
                             population_proxy.weight_at_age[age];
                }
                else
                {
                    index_ = population_proxy.fleets[fleet_].fleet->q.get_force_scalar(year) *
                             population_proxy.fleets[fleet_].fleet->selectivity->evaluate(population_proxy.population->ages[age]) *
                             population_proxy.numbers_at_age[i_age_year] *
                             population_proxy.weight_at_age[age]; // this->weight_at_age[age];
                }
                population_proxy.fleets[fleet_].expected_index[year] += index_;
            }
        }

        /**
         * This method is used to calculate the catch numbers at age for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the numbers at age for that population.
         * @param population
         * @param i_age_year
         * @param year
         * @param age
         * @return void
         */
        void CalculateCatchNumbersAA(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
            {
                // make an intermediate value in order to set multiple members (of
                // current and fleet objects) to that value.
                Type catch_; // catch_ is used to avoid using the c++ keyword catch
                // Baranov Catch Equation
                if (population->fleets[fleet_]->is_survey == false)
                {
                    catch_ = (population->fleets[fleet_]->Fmort[year] *
                              population->fleets[fleet_]->selectivity->evaluate(population->ages[age])) /
                             population->derived_quantities["mortality_Z"][i_age_year] *
                             population->derived_quantities["numbers_at_age"][i_age_year] *
                             (1 - fims_math::exp(-(population->derived_quantities["mortality_Z"][i_age_year])));
                }
                else
                {
                    catch_ = (population->fleets[fleet_]->selectivity->evaluate(population->ages[age])) *
                             population->derived_quantities["numbers_at_age"][i_age_year];
                }

                // this->catch_numbers_at_age[i_age_yearf] += catch_;
                // catch_numbers_at_age for the fleet module has different
                // dimensions (year/age, not year/fleet/age)
                population->fleets[fleet_]->derived_quantities["catch_numbers_at_age"][i_age_year] += catch_;
            }
        }

        void CalculateCatchNumbersAA(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t year,
            size_t age)
        {

            for (size_t fleet_ = 0; fleet_ < population_proxy.population->nfleets; fleet_++)
            {
                // make an intermediate value in order to set multiple members (of
                // current and fleet objects) to that value.
                Type catch_; // catch_ is used to avoid using the c++ keyword catch
                // Baranov Catch Equation
                if (population_proxy.fleets[fleet_].fleet->is_survey == false)
                {
                    catch_ = (population_proxy.fleets[fleet_].fleet->Fmort[year] *
                              population_proxy.fleets[fleet_].fleet->selectivity->evaluate(population_proxy.population->ages[age])) /
                             population_proxy.mortality_Z[i_age_year] *
                             population_proxy.numbers_at_age[i_age_year] *
                             (1 - fims_math::exp(-(population_proxy.mortality_Z[i_age_year])));
                }
                else
                {
                    catch_ = (population_proxy.fleets[fleet_].fleet->selectivity->evaluate(population_proxy.population->ages[age])) *
                             population_proxy.numbers_at_age[i_age_year];
                }

                // this->catch_numbers_at_age[i_age_yearf] += catch_;
                // catch_numbers_at_age for the fleet module has different
                // dimensions (year/age, not year/fleet/age)
                population_proxy.fleets[fleet_].catch_numbers_at_age[i_age_year] += catch_;
            }
        }

        /**
         * This method is used to calculate the catch weight at age for a population. It takes a
         * population object, the index of the age in the current year, the year,
         * and the age as input and calculates the weight at age for that population.
         * @param population
         * @param year
         * @param age
         * @return void
         */
        void CalculateCatchWeightAA(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t year,
            size_t age)
        {

            int i_age_year = year * population->nages + age;
            for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
            {

                population->fleets[fleet_]->derived_quantities["catch_weight_at_age"][i_age_year] =
                    population->fleets[fleet_]->derived_quantities["catch_numbers_at_age"][i_age_year] *
                    population->derived_quantities["weight_at_age"][age];
            }
        }

        void CalculateCatchWeightAA(
            CAAPopulationProxy<Type> &population_proxy,
            size_t year,
            size_t age)
        {

            int i_age_year = year * population_proxy.population->nages + age;
            for (size_t fleet_ = 0; fleet_ < population_proxy.population->nfleets; fleet_++)
            {

                population_proxy.fleets[fleet_].catch_weight_at_age[i_age_year] =
                    population_proxy.fleets[fleet_].catch_numbers_at_age[i_age_year] *
                    population_proxy.weight_at_age[age];
            }
        }

        /**
         * This method is used to calculate the maturity at age for a population. It takes a
         * population object, the index of the age in the current year, the age as input
         * and calculates the maturity at age for that population.
         * @param population
         * @param i_age_year
         * @param age
         * @return void
         */
        void CalculateMaturityAA(
            std::shared_ptr<fims_popdy::Population<Type>> &population,
            size_t i_age_year,
            size_t age)
        {
            population->derived_quantities["proportion_mature_at_age"][i_age_year] =
                population->maturity->evaluate(population->ages[age]);
        }

        void CalculateMaturityAA(
            CAAPopulationProxy<Type> &population_proxy,
            size_t i_age_year,
            size_t age)
        {
            population_proxy.proportion_mature_at_age[i_age_year] =
                population_proxy.population->maturity->evaluate(population_proxy.population->ages[age]);
        }

        /**
         * This method is used to calculate the proportions for a population. It takes a
         * population object, the index of the age in the current year, the age as input
         * and calculates the proportions for that population.
         * @param population
         * @param i_age_year
         * @param age
         * @return void
         */
        void ComputeProportions()
        {
            for (size_t p = 0; p < this->populations.size(); p++)
            {
                std::shared_ptr<fims_popdy::Population<Type>> &population =
                    this->populations[p];

                for (size_t year = 0; year < population->nyears; year++)
                {
                    for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
                    {

                        size_t index_yf = year * population->nfleets + fleet_;
                        Type sum_age = 0.0;
                        Type sum_length = 0.0;
                        for (size_t age = 0; age < population->nages; age++)
                        {
                            size_t i_age_year = year * population->nages + age;
                            sum_age += population->fleets[fleet_]->derived_quantities["catch_numbers_at_age"][i_age_year];
                        }

                        for (size_t age = 0; age < population->nages; age++)
                        {
                            size_t i_age_year = year * population->nages + age;
                            population->fleets[fleet_]->derived_quantities["proportion_catch_numbers_at_age"][i_age_year] =
                                population->fleets[fleet_]->derived_quantities["catch_numbers_at_age"][i_age_year] /
                                sum_age;
                        }

                        if (population->fleets[fleet_]->nlengths > 0)
                        {

                            for (size_t y = 0; y < population->fleets[fleet_]->nyears; y++)
                            {
                                fims::Vector<Type> &catch_numbers_at_length =
                                    population->fleets[fleet_]->derived_quantities["catch_numbers_at_length"];
                                fims::Vector<Type> &catch_numbers_at_age =
                                    population->fleets[fleet_]->derived_quantities["catch_numbers_at_age"];
                                fims::Vector<Type> &proportion_catch_numbers_at_length =
                                    population->fleets[fleet_]->derived_quantities["proportion_catch_numbers_at_length"];

                                sum_length = 0.0;
                                for (size_t l = 0; l < population->fleets[fleet_]->nlengths; l++)
                                {
                                    size_t i_length_year = y * population->fleets[fleet_]->nlengths + l;
                                    for (size_t a = 0; a < population->fleets[fleet_]->nages; a++)
                                    {
                                        size_t i_age_year = y * population->fleets[fleet_]->nages + a;
                                        size_t i_length_age = a * population->fleets[fleet_]->nlengths + l;
                                        catch_numbers_at_length[i_length_year] +=
                                            catch_numbers_at_age[i_age_year] *
                                            population->fleets[fleet_]->age_length_conversion_matrix[i_length_age];
                                    }
                                    sum_length += catch_numbers_at_length[i_length_year];
                                }
                                for (size_t l = 0; l < population->fleets[fleet_]->nlengths; l++)
                                {
                                    size_t i_length_year = y * population->fleets[fleet_]->nlengths + l;
                                    proportion_catch_numbers_at_length[i_length_year] =
                                        catch_numbers_at_length[i_length_year] / sum_length;
                                }
                            }
                        }
                    }
                }
            }
        }

        /**
         * * This method is used to evaluate the population dynamics model. 
         */
        virtual void Evaluate()
        {

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
            for (size_t p = 0; p < this->populations.size(); p++)
            {
                std::shared_ptr<fims_popdy::Population<Type>> &population =
                    this->populations[p];

                // CAAPopulationProxy<Type>& population = this->populations_proxies[p];

                for (size_t y = 0; y <= population->nyears; y++)
                {
                    for (size_t a = 0; a < population->nages; a++)
                    {
                        /*
                         index naming defines the dimensional folding structure
                         i.e. i_age_year is referencing folding over years and ages.
                         */
                        size_t i_age_year = y * population->nages + a;
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
                        if (y < population->nyears)
                        {
                            /*
                             First thing we need is total mortality aggregated across all fleets
                             to inform the subsequent catch and change in numbers at age
                             calculations. This is only calculated for years < nyears as these are
                             the model estimated years with data. The year loop extends to
                             y=nyears so that population numbers at age and SSB can be calculated
                             at the end of the last year of the model
                             */
                            CalculateMortality(population, i_age_year, y, a);
                        }
                        CalculateMaturityAA(population, i_age_year, a);
                        /* if statements needed because some quantities are only needed
                        for the first year and/or age, so these steps are included here.
                         */
                        if (y == 0)
                        {
                            // Initial numbers at age is a user input or estimated parameter
                            // vector.
                            CalculateInitialNumbersAA(population, i_age_year, a);

                            if (a == 0)
                            {
                                population->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                                    fims_math::exp(population->recruitment->log_rzero[0]);
                            }
                            else
                            {
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
                             age across ages to allow calculation of recruitment in the next year.
                             */

                            CalculateSpawningBiomass(population, i_age_year, y, a);

                            CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);

                            /*
                             Expected recruitment in year 0 is numbers at age 0 in year 0.
                             */

                            population->derived_quantities["expected_recruitment"][i_age_year] =
                                population->derived_quantities["numbers_at_age"][i_age_year];
                        }
                        else
                        {
                            if (a == 0)
                            {
                                // Set the nrecruits for age a=0 year y (use pointers instead of
                                // functional returns) assuming fecundity = 1 and 50:50 sex ratio
                                CalculateRecruitment(population, i_age_year, y, y);
                                population->derived_quantities["unfished_numbers_at_age"][i_age_year] =
                                    fims_math::exp(population->recruitment->log_rzero[0]);
                            }
                            else
                            {
                                size_t i_agem1_yearm1 = (y - 1) * population->nages + (a - 1);
                                CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, a);
                                CalculateUnfishedNumbersAA(population, i_age_year, i_agem1_yearm1, a);
                            }
                            CalculateBiomass(population, i_age_year, y, a);
                            CalculateSpawningBiomass(population, i_age_year, y, a);

                            CalculateUnfishedBiomass(population, i_age_year, y, a);
                            CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);
                        }

                        /*
                        Here composition, total catch, and index values are calculated for all
                        years with reference data. They are not calculated for y=nyears as there
                        is this is just to get final population structure at the end of the
                        terminal year.
                         */
                        if (y < population->nyears)
                        {
                            CalculateCatchNumbersAA(population, i_age_year, y, a);

                            CalculateCatchWeightAA(population, y, a);
                            CalculateCatch(population, y, a);
                            CalculateIndex(population, i_age_year, y, a);
                        }
                    }
                }
            }
            ComputeProportions();
        }
    };

}

#endif
