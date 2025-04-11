#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationEvaluateTestFixture, CalculateIndex_works)
    {
        
        std::vector<double> index_expected(nyears * nfleets, 0);
       
        // calculate index numbers at age in population module
        population.CalculateIndexNumbersAA(i_age_year, year, age);
        population.CalculateIndexWeightAA(year, age);
        population.CalculateIndex(i_age_year, year, age);

        // The test checks a single age in a single year, not an index. 
        // It was developed to test CalculateIndex() function while
        // the integration test loops over all ages to test the index.
        for (int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
        {
            int index_yf = year * population.nfleets + fleet_;
            
            // Currently q is not a vector and not changing over years. 
            // When testing time varying q, better to test entire vector.
            // If not possible to test entire vector, test middle or second to last 
            // than earlier years (collapses to mean in early years)
            index_expected[index_yf] += population.numbers_at_age[i_age_year]*
                                        population.fleets[fleet_]->q[0]*
                                        population.fleets[fleet_]->selectivity->evaluate(population.ages[age])*
                                        population.growth->evaluate(population.ages[age]);
    
        EXPECT_GT(population.fleets[fleet_]->index_weight[year], 0);
        EXPECT_GT(index_expected[index_yf], 0);
        EXPECT_EQ(index_expected[index_yf], population.fleets[fleet_]->index_weight[year]);
            
        }
    }
}