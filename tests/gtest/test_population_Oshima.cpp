#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

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

std::ofstream out("debug_Oshima.txt");
        
        // vector for storing expected recruitment 
        std::vector<double> expect_recruitment(year * population.nages, 0);
        // vector for rec devs - set rec devs to 1
        std::vector<double> recruit_deviations(population.nyears, 1);
      
         for(int year = 0; year < population.nyears; year++)
         {
             for(int age = 0; age < population.nages; age++)
             {
                 int index_ya = year * population.nages + age;
                 population.CalculateMaturityAA(index_ya, age);
                 population.CalculateMortality(index_ya, year, age);
                 if(year == 0){
                    // for the first year, the population.numbers_at_age is calculated
                    // with CalculateInitialNumbersAA
                     population.CalculateInitialNumbersAA(index_ya, age);
                     if(age == 0)
                     {
                         // if age is 0, then unfished numbers AA are rzero
                         population.unfished_numbers_at_age[index_ya] = population.recruitment->rzero;
                     }
                     else
                     {
                         // if age is > 0 then use CalculateUnfishedNumbersAA 
                         population.CalculateUnfishedNumbersAA(index_ya,age-1,age);
                         
                     }
                     population.CalculateSpawningBiomass(index_ya, year, age);
                     population.CalculateUnfishedSpawningBiomass(index_ya,year,age);
                 }
                 else{
                     int index_ya2 = (year -1) * population.nages + (age -1);
                     population.CalculateNumbersAA(index_ya, index_ya2, age); 
                     population.CalculateUnfishedNumbersAA(index_ya, index_ya2, age);
                 }
                 // print out unfished numbers at age and numbers at age for debugging
                 out<<"unfished numbers at age = "<<population.unfished_numbers_at_age[index_ya]<<"\n";
                 out<<"numbers at age = "<<population.numbers_at_age[index_ya]<<"\n";

             }
         }
        
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + (age - 1);
        
        out <<" index_ya: "<<index_ya<<" index_ya2: "<<index_ya2<<"\n";
        // print out values for debugging to see what they are at given year/age index
        out <<" NAA= "<<population.numbers_at_age[index_ya]<<"\n";
        out <<" SB= "<<population.spawning_biomass[index_ya]<<"\n";
        out<<"unfished numbers at age ya2 = "<<population.unfished_numbers_at_age[index_ya2]<<"\n";
        out <<" UFSB= "<<population.unfished_spawning_biomass[index_ya]<<"\n";
        
        // trying to create an ssbzero variable to use in the recruitment equation by
        // multiplying the initial numbers-at-age and the population weight-at-age
        // not sure if this will work for age 0 (don't know what weight-at-age for age 0 is)
        double ssbzero = population.recruitment->rzero * population.weight_at_age[age];
        out <<" SSB zero = "<<population.recruitment->rzero<<" * "<<population.weight_at_age[age]<<"\n";
        out<<"SSBzero:"<<ssbzero<<"\n";
        
        // calculate recruitment in population module
        population.CalculateRecruitment(index_ya, year);
        out<<"pop numbers at age = "<<population.numbers_at_age[index_ya]<<"\n";
        out<<"pop numbers at age for year+1= "<<population.numbers_at_age[index_ya2]<<"\n";
    
        out<<"recdevs:"<<recruit_deviations[year]<<"\n";
        // calculate expected recruitment based on bev-holt equation
        expect_recruitment[index_ya] = 
        (0.8 * rzero * steep * population.spawning_biomass[index_ya]) / 
        (0.2 * ssbzero * (1.0 - steep) + population.spawning_biomass[index_ya] * (steep - 0.2)) * recruit_deviations[year]; 
        out<<"expected rec = "<<expect_recruitment[index_ya]<<"\n";
        out<<"expected numbers in year+1 = "<<population.numbers_at_age[year+1]<<"\n";
        out<<"expected numbers aa = "<<population.numbers_at_age[year+1]<<"\n";
      
        // testing that expected recruitment and population.numbers_at_age match
        EXPECT_EQ(population.numbers_at_age[index_ya], expect_recruitment[index_ya]);
    }
}
