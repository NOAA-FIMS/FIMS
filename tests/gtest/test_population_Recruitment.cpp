#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

//Not working
namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateRecruitment_works)
    {
        // calculating spawning biomass for year 4
        int sb_year = 4;
        int sb_age = 0;
        int sb_i_age_year = sb_year * population.nages + sb_age;
        int sb_i_agem1_yearm1 = (sb_year - 1) * population.nages + sb_age - 1;

        population.CalculateMortality(sb_i_age_year, sb_year, sb_age);
        population.CalculateNumbersAA(sb_i_age_year, sb_i_agem1_yearm1, sb_age);
        for (size_t year = 0; year < nyears; year++) {
           for (size_t age = 0; age < nages; age++){
               int i_age_year = year * population.nages + age;
               population.CalculateMaturityAA(i_age_year, age);
           }
        }
        population.CalculateSpawningBiomass(sb_i_age_year, sb_year, sb_age);

        // calculating phi0
        double phi0 = population.CalculateSBPR0();
    
        // calculating recruitment for year 5
        int r_year = 5;
        int r_age = 0; // just calculating for age 0 
        int r_i_age_year = r_year * population.nages + r_age;
        // specifying steepness and rzero values for calculations
        // values from test_population_test_fixture.hpp 
        double steep = 0.75;
        double rzero = 1000000;
    
        // vector for storing expected recruitment 
        std::vector<double> expect_recruitment(population.nyears * population.nages, 0.0);
        
        expect_recruitment[r_i_age_year] = 
        (0.8 * rzero * steep * population.spawning_biomass[sb_year]) / 
        (0.2 * phi0 * rzero * (1.0 - steep) + population.spawning_biomass[sb_year] * (steep - 0.2)) * population.recruitment->recruit_deviations[r_year]; 
      
        // calculate recruitment in population module
        population.CalculateRecruitment(r_i_age_year, r_year);
        
        // testing that expected recruitment and population.numbers_at_age match
        // EXPECT_DOUBLE_EQ() verifies that the two double values are approximately equal, to within 4 ULPs from each other.
        EXPECT_DOUBLE_EQ(population.numbers_at_age[r_i_age_year], expect_recruitment[r_i_age_year]);
        // testing that population.numbers_at_age > 0.0
        EXPECT_GT(population.numbers_at_age[r_i_age_year], 0.0);
    }
}
