#include "benchmark/benchmark.h"
#include "population_dynamics/recruitment/functors/recruitment_base.hpp"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

void BM_population_dynamics_recruitment_base_PrepareBiasAdjustment(benchmark::State &state)
{
  for (auto _ : state) {
    fims::SRBevertonHolt<double> recruit;
    recruit.sigma_recruit = 0.7;
    recruit.use_recruit_bias_adjustment = true;
    recruit.recruit_bias_adjustment = {0.0, 0.0, 0.0};
    recruit.recruit_bias_adjustment_fraction = {2.0, 2.0, 2.0};
    recruit.PrepareBiasAdjustment();
  }
    
}
BENCHMARK(BM_population_dynamics_recruitment_base_PrepareBiasAdjustment);

void BM_population_dynamics_recruitment_base_PrepareBiasAdjustmentBenchmark(benchmark::State &state)
{
  for (auto _ : state) {
    fims::SRBevertonHolt<double> recruit;
    recruit.sigma_recruit = 0.7;
    recruit.use_recruit_bias_adjustment = true;
    recruit.recruit_bias_adjustment = {0.0, 0.0, 0.0};
    recruit.recruit_bias_adjustment_fraction = {2.0, 2.0, 2.0};
    recruit.PrepareBiasAdjustmentBenchmark();
  }
    
}
BENCHMARK(BM_population_dynamics_recruitment_base_PrepareBiasAdjustmentBenchmark);