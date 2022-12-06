#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateRecruitment_works)
    {
        // calculating recruitment for year 4
        int year = 4;
        // specifying steepness and rzero values for calculations
        // values from test_population_test_fixture.hpp 
        double steep = 0.75;
        double rzero = 1000000.0;
        double rec_dev = 0.2;
        // vector for storing expeceted recruitment 
        // check dims on this
        std::vector<double> expect_recruitment(year * population.nages, 0);
        // temporary storage for recruitment to use in loop 
        double recruitment_temp;
        //int age = 6; Need to include all ages to calculate recruitment?
        int index_ya = year * population.nages;
        //int index_ya2 = (year - 1) * population.nages; should I be using this one instead 
        //since the index for ssb and ssbzero are year-1 in recruitment equation?

        // calculate recruitment in population module
        population.CalculateRecruitment(index_ya, year);
        
// for each age, age 0 to max age
        for (size_t age = 0; age < nages; age++){
              // create index for year 4, and age
              // Why do we need to do this in the loop?
               int index_ya = year * population.nages + age;
               // for year-age, calculate the spawning biomass
               population.CalculateSpawningBiomass(index_ya, year, age);
               // for year-age calculate the unfished spawning biomass
               population.CalculateInitialNumbersAA(index_ya, age);
               // trying to create an ssbzero variable to use in the recruitment equation by
               // multiplying the initial numbers-at-age and the population weight-at-age
               double ssbzero = population.numbers_at_age[index_ya] * population.weight_at_age[age];
               recruitment_temp = 
               (0.8 * rzero * steep * population.spawning_biomass[age]) / 
               (0.2 * ssbzero * (1.0 - steep) + population.spawning_biomass[age] * (steep - 0.2)) *
               rec_dev;
               expect_recruitment[index_ya] += recruitment_temp;
           }

        EXPECT_EQ(population.numbers_at_age[index_ya], expect_recruitment[index_ya]);
    }
}

// Issues: 
// how to get recruit_deviations 
//    see lines 118-125 in test_population_test_fixture.hpp
//    but don't think that will work because rec_devs isn't a variable in population.hpp
// index should be year-1 for spawners and ssbzero
// 



