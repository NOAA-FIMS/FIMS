#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{

    TEST_F(SPEvaluateTestFixture, CalculateBiomass_works)
    {
        std::vector<double> biomass(nyears, 0);
        // calculate biomass in in suplus production module
        for(size_t year_ = 0; year_ < nyears; year_++) {
            surplus_production_model->CalculateCatch(population, year_);
            surplus_production_model->CalculateDepletion(population, year_);
        }

        surplus_production_model->CalculateIndex(population, year);
        surplus_production_model->CalculateBiomass(population, year);

        auto& dq_pop = surplus_production_model->population_derived_quantities[population->GetId()];

        biomass[year] = population->depletion->depletion[year] * 
                exp(population->depletion->log_K[0]);
        EXPECT_EQ( biomass[year], dq_pop["biomass"][year]);

    }
}