#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{

    TEST_F(SPEvaluateTestFixture, CalculateDepletion_works)
    {
        std::vector<double> log_expected_depletion(nyears, 0);

        for(size_t year = 0; year < nyears; year++) {
            surplus_production_model->CalculateCatch(population, year);
            surplus_production_model->CalculateDepletion(population, year);
        }

        auto& dq_pop = surplus_production_model->GetPopulationDerivedQuantities(population->GetId());
        double depletion_ym1 = surplus_production_model->populations[0]->depletion->depletion[year-1];
        double catch_ym1 = dq_pop["observed_catch"][year-1];

        log_expected_depletion[0] = surplus_production_model->populations[0]->depletion->log_init_depletion[0];

        log_expected_depletion[year] =  
          fims_math::log( fims_math::ad_max(depletion_ym1 + 
            (surplus_production_model->populations[0]->depletion->r[0] / 
                (surplus_production_model->populations[0]->depletion->m[0] - 1.0)) * 
            depletion_ym1 *
            (1.0 - fims_math::pow(depletion_ym1, 
                surplus_production_model->populations[0]->depletion->m[0] - 
                1.0)) -
           catch_ym1 / surplus_production_model->populations[0]->depletion->K[0],
           0.001)
          );


        EXPECT_EQ(log_expected_depletion[year], 
            surplus_production_model->populations[0]->depletion->log_expected_depletion[year]);
        EXPECT_EQ(log_expected_depletion[0],
            surplus_production_model->populations[0]->depletion->log_expected_depletion[0]);
    }

}