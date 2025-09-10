#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(CAAEvaluateTestFixture, CalculateIndex_works)
    {
        
        std::vector<double> index_expected(nyears * nfleets, 0);
       
        // calculate index numbers at age in population module
        catch_at_age_model->CalculateIndexNumbersAA(population, i_age_year, year, age);
        catch_at_age_model->CalculateIndexWeightAA(population, year, age);
        catch_at_age_model->CalculateIndex(population, i_age_year, year, age);

        // The test checks a single age in a single year, not an index. 
        // It was developed to test CalculateIndex() function while
        // the integration test loops over all ages to test the index.
        uint32_t pop_id = population->GetId();
        auto& dq_pop = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);
        for (int fleet_ = 0; fleet_ < catch_at_age_model->populations[pop_id]->nfleets; fleet_++)
        {
            int index_yf = year * catch_at_age_model->populations[pop_id]->nfleets + fleet_;
            auto &pop = catch_at_age_model->populations[pop_id];

            // Currently q is not a vector and not changing over years. 
            // When testing time varying q, better to test entire vector.
            // If not possible to test entire vector, test middle or second to last 
            // than earlier years (collapses to mean in early years)
            index_expected[index_yf] += dq_pop["numbers_at_age"][i_age_year]*
                                        pop->fleets[fleet_]->q[0]*
                                        pop->fleets[fleet_]->selectivity->evaluate(pop->ages[age])*
                                        pop->growth->evaluate(pop->ages[age]);
            uint32_t fleet_id = pop->fleets[fleet_]->GetId();
            auto& dq_fleet = catch_at_age_model->GetFleetDerivedQuantities(fleet_id);
            EXPECT_GT(dq_fleet["index_weight"][year], 0);
            EXPECT_GT(index_expected[index_yf], 0);
            EXPECT_EQ(index_expected[index_yf], dq_fleet["index_weight"][year]);

        }
    }
}