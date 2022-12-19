#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateRecruitment_works)
    {
        // calculating recruitment for year 4
        int year = 4;
        int age = 6; // just calculating for age 5 to test
        // specifying steepness and rzero values for calculations
        // values from test_population_test_fixture.hpp 
        double steep = 0.75;
        double rzero = 1000000.0;

std::ofstream out("debug_Oshima.txt");
        
        // vector for storing expected recruitment 
        // check dims on this
        std::vector<double> expect_recruitment(year * population.nages, 0);
        std::vector<double> recruit_deviations(population.nyears, 1);
        // temporary storage for recruitment to use in loop 
        int index_ya = year * population.nages + age;
        int index_ya1 = (year + 1) * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + (age - 1);
         
        out <<" index_ya: "<<index_ya<<" index_ya2: "<<index_ya2<<" index_ya1: "<<index_ya1<<"\n";
        
        //population.CalculateInitialNumbersAA(index_ya,age);
        //out <<" NAA initial= "<<population.numbers_at_age[index_ya]<<"\n";
        // trying to create an ssbzero variable to use in the recruitment equation by
        // multiplying the initial numbers-at-age and the population weight-at-age
        double ssbzero = population.recruitment->rzero * population.weight_at_age[age];
        out <<" SSB zero = "<<population.recruitment->rzero<<" * "<<population.weight_at_age[age]<<"\n";
        out<<"SSBzero:"<<ssbzero<<"\n";
        // calculate all steps needed to calculate recruitment in population module
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);
        out <<" NAA= "<<population.numbers_at_age[index_ya]<<"\n";
        population.CalculateMaturityAA(index_ya, age);
        out<<"proportion_female"<<population.proportion_female<<"\n";
        out<<"proportion_mature"<<population.proportion_mature_at_age[index_ya]<<"\n";
        out<<"waa"<<population.weight_at_age[age]<<"\n";
        population.CalculateSpawningBiomass(index_ya, year, age);
        out <<" SB= "<<population.spawning_biomass[year]<<"\n";
        out <<" SB= "<<population.spawning_biomass[index_ya]<<"\n";

        population.CalculateUnfishedNumbersAA(index_ya, index_ya2, age);
        out <<" M= "<<population.M[index_ya2]<<"\n";
        //out <<" proportion mature = "<<population.proportion_mature_at_age[index_ya]<<"\n";
        //out <<" prop mature age= "<<population.proportion_mature_at_age[age]<<"\n";
        //out <<" UFNAA= "<<population.unfished_numbers_at_age[index_ya2]<<"\n";
        //out <<" UFNAA= "<<population.unfished_numbers_at_age[0]<<"\n";
        population.CalculateUnfishedSpawningBiomass(index_ya, year, age);
        //out <<" UFSB= "<<population.unfished_spawning_biomass[index_ya]<<"\n";

        // calculate recruitment in population module
        population.CalculateRecruitment(index_ya, year+1);
        out<<"pop numbers at age = "<<population.numbers_at_age[index_ya]<<"\n";
       //double ssbzero = population.numbers_at_age[index_ya] * population.weight_at_age[age];
       
        out <<"population spawning biomass at age 5 = "<<population.spawning_biomass[index_ya]<<"\n";
        out<<"recdevs:"<<recruit_deviations[year]<<"\n";
        expect_recruitment[index_ya] = 
        (0.8 * rzero * steep * population.spawning_biomass[index_ya]) / 
        (0.2 * ssbzero * (1.0 - steep) + population.spawning_biomass[index_ya] * (steep - 0.2)) * recruit_deviations[year]; 
        out<<"expected rec = "<<expect_recruitment[index_ya]<<"\n";
        out<<"expected numbers in year+1 = "<<population.numbers_at_age[year+1]<<"\n";
        out<<"expected numbers aa = "<<population.numbers_at_age[index_ya1]<<"\n";
      

        EXPECT_EQ(population.numbers_at_age[index_ya], expect_recruitment[index_ya]);
    }
}

         

// for(int year = 0; year < population.nyears; year++)
//         {
//             for(int age = 0; age < population.nages; age++)
//             {
//                 int index_ya = year * population.nages + age;
//                 // for the first year, the population.numbers_at_age is calculated
//                 // with CalculateInitialNumbersAA
//                 if(year == 0){
//                     population.CalculateInitialNumbersAA(index_ya, age);
//                     if(age == 0)
//                     {
//                         // if age is 0, then unfished numbers AA are rzero
//                         population.unfished_numbers_at_age[index_ya] = population.recruitment->rzero;
//                     }
//                     else
//                     {
//                         // if age is > 0 then use CalculateUnfishedNumbersAA 
//                         population.CalculateUnfishedNumbersAA(index_ya,age-1,age);
//                     }
//                     population.CalculateSpawningBiomass(index_ya, year, age);
//                     population.CalculateUnfishedSpawningBiomass(index_ya,year,age);
//                 }
//                 else{
//                     int index_ya2 = (year -1) * population.nages + (age -1);
//                     population.CalculateNumbersAA(index_ya, index_ya2, age); 
//                     population.CalculateUnfishedNumbersAA(index_ya, index_ya2, age);
//                 }

//             }
//         }
