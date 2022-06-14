#include "benchmark/benchmark.h"
#include "population_dynamics/recruitment/functors/recruitment_base.hpp"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

void BM_population_dynamics_recruitment_base_PrepareBiasAdjustment(benchmark::State &state)
{
  for (auto _ : state) {
    fims::SRBevertonHolt<double> recruit;
    recruit.sigma_recruit = 0.7;
    recruit.use_recruit_bias_adjustment = true;
    std::vector<double> bias_adjustment_vec(50, 0.0);
    recruit.recruit_bias_adjustment = bias_adjustment_vec;
    std::vector<double> bias_adjustment_fraction_vec(50, 2.0);
    recruit.recruit_bias_adjustment_fraction = bias_adjustment_fraction_vec;
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
    std::vector<double> bias_adjustment_vec(50, 0.0);
    recruit.recruit_bias_adjustment = bias_adjustment_vec;
    std::vector<double> bias_adjustment_fraction_vec(50, 2.0);
    recruit.recruit_bias_adjustment_fraction = bias_adjustment_fraction_vec;
    recruit.PrepareBiasAdjustmentBenchmark();
  }
    
}
BENCHMARK(BM_population_dynamics_recruitment_base_PrepareBiasAdjustmentBenchmark);