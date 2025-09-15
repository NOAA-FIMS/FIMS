#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{

    TEST_F(SPEvaluateTestFixture, CalculateRPs_works)
    {
        
        std::vector<double> fmsy(1, 0);
        std::vector<double> bmsy(1, 0);
        std::vector<double> msy(1, 0);
        // calculate biomass in in suplus production module
        for(size_t year_ = 0; year_ < nyears; year_++) {
            surplus_production_model->CalculateCatch(population, year_);
            surplus_production_model->CalculateDepletion(population, year_);
        }

        surplus_production_model->CalculateIndex(population, year);
        surplus_production_model->CalculateBiomass(population, year);
        surplus_production_model->CalculateReferencePoints(population);
        auto& dq_pop = surplus_production_model->population_derived_quantities[population->GetId()];

        fmsy[0] = exp(population->depletion->log_r[0]) *
            pow(exp(population->depletion->log_m[0])-1,-1) *
            (1-1/exp(population->depletion->log_m[0]));
        bmsy[0] = exp(population->depletion->log_K[0]) *
            pow(exp(population->depletion->log_m[0]),
                -1/(exp(population->depletion->log_m[0])-1));
        msy[0] = fmsy[0]*bmsy[0];

        EXPECT_DOUBLE_EQ( fmsy[0], dq_pop["fmsy"][0]);
        EXPECT_DOUBLE_EQ( bmsy[0], dq_pop["bmsy"][0]);
        EXPECT_DOUBLE_EQ( msy[0], dq_pop["msy"][0]);
    }

       TEST_F(SPEvaluateTestFixture, CalculateHRs_works)
    {
        
        std::vector<double> harvest_rate(nyears, 0);
        // calculate biomass in in suplus production module
        for(size_t year_ = 0; year_ < nyears; year_++) {
            surplus_production_model->CalculateCatch(population, year_);
            surplus_production_model->CalculateDepletion(population, year_);
        }

        surplus_production_model->CalculateIndex(population, year);
        surplus_production_model->CalculateBiomass(population, year);
        surplus_production_model->CalculateHarvestRate(population, year);
        auto& dq_pop = surplus_production_model->population_derived_quantities[population->GetId()];

        harvest_rate[year] = dq_pop["observed_catch"][year] / dq_pop["biomass"][year];
        EXPECT_EQ( harvest_rate[year], dq_pop["harvest_rate"][year]);

    }
}