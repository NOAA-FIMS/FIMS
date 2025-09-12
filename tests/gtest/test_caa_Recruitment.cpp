#include "gtest/gtest.h"
#include "../../tests/gtest/test_caa_test_fixture.hpp"

//Not working
namespace
{

    TEST_F(CAAEvaluateTestFixture, CalculateRecruitment_works)
    {
        // calculating spawning biomass for year 4
        int sb_year = 4;
        int sb_age = 0;
        int sb_i_age_year = sb_year * population->nages + sb_age;
        int sb_i_agem1_yearm1 = (sb_year - 1) * population->nages + sb_age - 1;

        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->population_derived_quantities[pop_id];

        catch_at_age_model->CalculateMortality(population, sb_i_age_year, sb_year, sb_age);
        catch_at_age_model->CalculateNumbersAA(population, sb_i_age_year, sb_i_agem1_yearm1, sb_age);
        for (size_t year = 0; year < nyears; year++) {
           for (size_t age = 0; age < nages; age++){
               int i_age_year = year * population->nages + age;
               catch_at_age_model->CalculateMaturityAA(population, i_age_year, age);
           }
        }
        catch_at_age_model->CalculateSpawningBiomass(population, sb_i_age_year, sb_year, sb_age);

        // calculating phi0
        double phi0 = catch_at_age_model->CalculateSBPR0(population);

        // calculating recruitment for year 5
        int r_year = 5;
        int r_age = 0; // just calculating for age 0 
        int r_i_age_year = r_year * population->nages + r_age;
        // specifying steepness and rzero values for calculations
        // values from test_population_test_fixture.hpp 
        double steep = 0.75;
        double rzero = 1000000;
    
        // vector for storing expected recruitment 
        std::vector<double> expect_recruitment(population->nyears * population->nages, 0.0);

        /*the log_recruit_dev vector does not include a value for year == 0
        and is of length nyears - 1 where the first position of the vector
        corresponds to the second year of the time series.*/
        expect_recruitment[r_i_age_year] = 
        (0.8 * rzero * steep * dq["spawning_biomass"][sb_year]) /
        (0.2 * phi0 * rzero * (1.0 - steep) + dq["spawning_biomass"][sb_year] * (steep - 0.2)) * fims_math::exp(population->recruitment->log_recruit_devs[r_year-1]); 

        // calculate recruitment in population module
        catch_at_age_model->CalculateRecruitment(population, r_i_age_year, r_year, r_year);
        
        // testing that expected recruitment and population numbers_at_age match
        // EXPECT_DOUBLE_EQ() verifies that the two double values are approximately equal, to within 4 ULPs from each other.
        EXPECT_DOUBLE_EQ(dq["numbers_at_age"][r_i_age_year], expect_recruitment[r_i_age_year]);
        // testing that population numbers_at_age > 0.0
        EXPECT_GT(dq["numbers_at_age"][r_i_age_year], 0.0);
    }
}
