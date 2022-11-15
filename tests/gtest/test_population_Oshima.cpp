#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateCatchWeightAA_works)
    {

        for (size_t year = 0; year < population.nyears; year++)
        {
            for (size_t age = 0; age < population.nages; age++)
            {
                //size_t index_yaf = year * population.nages + age * population.nfleets + age * population.nfleets + fleet_index;
                // don't need to create index_yaf here because the function only needs year and age
                population.CalculateCatchWeightAA(year, age);
                // dimensions based on the catch_weight_at_age object in population.hpp line 132
                std::vector<double> catch_weight_at_age(nyears * nages * nfleets, 0);

                for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                {
                    size_t index_yaf = year * population.nages * population.nfleets + age * population.nfleets + fleet_index;
                //    catch_weight_at_age[index_yaf] += population.catch_numbers_at_age[index_yaf] *
                //                             population.weight_at_age[age];
                // Not sure how to write the catch_numbers_at_age part, because it is another function or 
                // will we create something in the population test fixture?

                    catch_weight_at_age[index_yaf] += CalculateCatchNumbersAA->evaluate(index_yaf) *
                                                      population.weight_at_age[age]
                }
                EXPECT_EQ(population.CalculateCatchWeightAA[index_yaf], catch_weight_at_age[index_yaf]);

            }
        }

