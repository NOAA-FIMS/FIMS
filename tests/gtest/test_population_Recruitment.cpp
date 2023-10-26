#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

//Not working
namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateRecruitment_works)
    {
        // calculating recruitment for year 4
        int year = 4;
        int age = 0; // just calculating for age 0 
        // specifying steepness and rzero values for calculations
        // values from test_population_test_fixture.hpp 
        double steep = 0.75;
        double rzero = 1000000.0;
        
        // vector for storing expected recruitment 
        std::vector<double> expect_recruitment(year * population.nages, 0);
        // vector for rec devs - set rec devs to 1
        std::vector<double> log_recruit_devs(population.nyears, 1);
      
         for(int year = 0; year < population.nyears; year++)
         {
             for(int age = 0; age < population.nages; age++)
             {
                 int i_age_year = year * population.nages + age;
                 population.CalculateMaturityAA(i_age_year, age);
                 population.CalculateMortality(i_age_year, year, age);
                 if(year == 0){
                    // for the first year, the population.numbers_at_age is calculated
                    // with CalculateInitialNumbersAA
                     population.CalculateInitialNumbersAA(i_age_year, age);
                     if(age == 0)
                     {
                         // if age is 0, then unfished numbers AA are rzero
                         population.unfished_numbers_at_age[i_age_year] = population.recruitment->rzero;
                     }
                     else
                     {
                         // if age is > 0 then use CalculateUnfishedNumbersAA 
                         population.CalculateUnfishedNumbersAA(i_age_year,age-1,age);
                         
                     }
                     population.CalculateSpawningBiomass(i_age_year, year, age);
                     population.CalculateUnfishedSpawningBiomass(i_age_year,year,age);
                 }
                 else{
                     int i_agem1_yearm1 = (year -1) * population.nages + (age -1);
                     population.CalculateNumbersAA(i_age_year, i_agem1_yearm1, age); 
                     population.CalculateUnfishedNumbersAA(i_age_year, i_agem1_yearm1, age);
                 }

             }
         }
        
        int i_age_year = year * population.nages + age;
        int i_agem1_yearm1 = (year - 1) * population.nages + (age - 1);
        
        // trying to create an ssbzero variable to use in the recruitment equation by
        // multiplying the initial numbers-at-age and the population weight-at-age
        // not sure if this will work for age 0 (don't know what weight-at-age for age 0 is)
        double ssbzero = population.recruitment->rzero * population.weight_at_age[age];
        
        // calculate recruitment in population module
        population.CalculateRecruitment(i_age_year, year);
        
        // calculate expected recruitment based on bev-holt equation
        expect_recruitment[i_age_year] = 
        (0.8 * rzero * steep * population.spawning_biomass[i_age_year]) / 
        (0.2 * ssbzero * (1.0 - steep) + population.spawning_biomass[i_age_year] * (steep - 0.2)) * fims::exp(log_recruit_devs[year]); 
      
        // testing that expected recruitment and population.numbers_at_age match
        EXPECT_EQ(population.numbers_at_age[i_age_year], expect_recruitment[i_age_year]);
    }
}
