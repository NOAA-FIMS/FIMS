#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateIndex_works)
    {
        
        std::vector<double> expected_index(nyears * nfleets, 0);
        // set up an arbitrary year/age combo to test
        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;
       
        // calculate index numbers at age in population module
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);
        population.CalculateIndex(index_ya, year, age);
        
        // The test checks a single age in a single year, not an index. 
        // It was developed to test CalculateIndex() function while
        // the integration test loops over all ages to test the index.
        for (int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
        {
            if(population.fleets[fleet_]->is_survey){
            int index_yf = year * population.nfleets + fleet_;
            
            // Currently q is not a vector and not changing over years. 
            // When testing time varying q, better to test entire vector.
            // If not possible to test entire vector, test middle or second to last 
            // than earlier years (collapses to mean in early years)
            expected_index[index_yf] += population.numbers_at_age[index_ya]*
                                        population.fleets[fleet_]->q*
                                        population.fleets[fleet_]->selectivity->evaluate(population.ages[age])*
                                        population.growth->evaluate(population.ages[age]);
    
        EXPECT_GT(population.fleets[fleet_]->expected_index[year], 0);
        EXPECT_GT(expected_index[index_yf], 0);
        EXPECT_EQ(expected_index[index_yf], population.fleets[fleet_]->expected_index[year]);
            }
        }
    }
}