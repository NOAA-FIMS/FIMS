#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{

    TEST_F(SPEvaluateTestFixture, CalculateDepletion_works)
    {
        std::vector<double> depletion_expected(nyears, 0);
        std::vector<double> log_expected_depletion(nyears, 0);

        for(size_t year = 0; year < nyears; year++) {
            surplus_production_model->CalculateCatch(population, year);
            surplus_production_model->CalculateDepletion(population, year);
        }

        auto& dq_pop = surplus_production_model->population_derived_quantities[population->GetId()];
        depletion_expected[year-1] = fims_math::exp(
            surplus_production_model->populations[0]->depletion->log_depletion[year-2]);
        log_expected_depletion[year] = 
            log(surplus_production_model->populations[0]->depletion->evaluate_mean(
                depletion_expected[year-1], dq_pop["observed_catch"][year-1])
        );

        EXPECT_EQ(depletion_expected[year-1], 
            surplus_production_model->populations[0]->depletion->depletion[year-1]);
        EXPECT_EQ(log_expected_depletion[year], 
            surplus_production_model->populations[0]->depletion->log_expected_depletion[year]);
        EXPECT_EQ(fims_math::exp(
                surplus_production_model->populations[0]->depletion->log_init_depletion[0]), 
            surplus_production_model->populations[0]->depletion->depletion[0]);
    }

}