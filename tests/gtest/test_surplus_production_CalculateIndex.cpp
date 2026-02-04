#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{

    TEST_F(SPEvaluateTestFixture, CalculateIndex_works)
    {
        std::vector<double> index(nyears, 0);
        std::vector<double> log_index(nyears, 0);
        for(size_t year_ = 0; year_ < nyears; year_++) {
            surplus_production_model->CalculateCatch(population, year_);
            surplus_production_model->CalculateDepletion(population, year_);
        }

        surplus_production_model->CalculateIndex(surplus_production_model->populations[0], year);
        for (int fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
            auto& fleet = population->fleets[fleet_];
            auto& fleet_dq = surplus_production_model->fleet_derived_quantities[fleet->GetId()];
            index[year] = population->depletion->depletion[year] * fleet->q[0] * 
                exp(population->depletion->log_K[0]);
            log_index[year] = fims_math::log(index[year]);
            EXPECT_DOUBLE_EQ(fleet_dq["log_index_expected"][year], log_index[year]);
            EXPECT_DOUBLE_EQ(fleet_dq["index_expected"][year], index[year]);
        }
    }
}