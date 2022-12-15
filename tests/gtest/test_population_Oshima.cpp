#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateRecruitment_works)
    {
        // calculating recruitment for year 4
        int year = 4;
        int age = 5; // just calculating for age 5 to test
        // specifying steepness and rzero values for calculations
        // values from test_population_test_fixture.hpp 
        double steep = 0.75;
        double rzero = 1000000.0;
        double rec_dev = 0.0;
        // vector for storing expected recruitment 
        // check dims on this
std::ofstream out("debug_Oshima.txt");
        

        std::vector<double> expect_recruitment(year * population.nages, 0);
        // temporary storage for recruitment to use in loop 
        double recruitment_temp;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + (age - 1);

out <<" index_ya: "<<index_ya<<" index_ya2: "<<index_ya2<<" steepness: "<<steep<<"\n";

        // calculate all steps needed to calculate recruitment in population module
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);
        population.CalculateMaturityAA(index_ya, age);
        population.CalculateSpawningBiomass(index_ya, year, age);

        out <<" SSB zero = "<<population.numbers_at_age[index_ya]<<" * "<<population.weight_at_age[age];
        population.CalculateUnfishedNumbersAA(index_ya, index_ya2, age);
        population.CalculateUnfishedSpawningBiomass(index_ya, year, age);
        population.CalculateInitialNumbersAA(index_ya,age);
        // calculate recruitment in population module
        population.CalculateRecruitment(index_ya, year);
        
        // for each age, age 0 to max age
        //for (size_t age = 0; age < nages; age++){
              // create index for year 4, and age
          //     int index_ya = year * population.nages + age;
               // for year-age, calculate the spawning biomass
          //     population.CalculateSpawningBiomass(index_ya, year, age);
               // for year-age calculate the unfished spawning biomass
          //     population.CalculateInitialNumbersAA(index_ya, age);
               // trying to create an ssbzero variable to use in the recruitment equation by
               // multiplying the initial numbers-at-age and the population weight-at-age
        double ssbzero = population.numbers_at_age[index_ya] * population.weight_at_age[age];
        //recruitment_temp = 
        //(0.8 * rzero * steep * population.spawning_biomass[age]) / 
        //(0.2 * ssbzero * (1.0 - steep) + population.spawning_biomass[age] * (steep - 0.2)) * rec_dev; // not sure how to implement rec_dev part into the equation
        //expect_recruitment[index_ya] += recruitment_temp;
          // }

       // EXPECT_EQ(population.numbers_at_age[index_ya], expect_recruitment[index_ya]);
    }
}

// Issues: 
// how to get recruit_deviations 
//    see lines 118-125 in test_population_test_fixture.hpp
//    but don't think that will work because rec_devs isn't a variable in population.hpp
// index should be year-1 for spawners and ssbzero
// 



