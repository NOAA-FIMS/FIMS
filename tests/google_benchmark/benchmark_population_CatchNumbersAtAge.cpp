// Benchmark for the workload in test_population_CatchNumbersAtAge.cpp
// Times: CalculateLandingsNumbersAA + CalculateLandingsWeightAA. Setup reused
// from CAAEvaluateTestFixture.

#include "benchmark/benchmark.h"
#include "gtest/gtest.h"

#include "../gtest/test_population_test_fixture.hpp"

namespace {

struct BenchCAAEvaluate : public CAAEvaluateTestFixture {
  void Init() { SetUp(); }
  void TestBody() override {}

  // Runs the same production calls as the test (resets accumulators first).
  double RunBenchmarkedCode() {
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; ++fleet_) {
      auto fleet_id = population->fleets[fleet_]->GetId();
      auto& fdq = catch_at_age_model->GetFleetDerivedQuantities(fleet_id);
      fdq["landings_numbers_at_age"][i_age_year] = 0.0;
    }
    catch_at_age_model->CalculateLandingsNumbersAA(population, i_age_year,
                                                   year, age);
    catch_at_age_model->CalculateLandingsWeightAA(population, year, age);
    auto fleet0_id = population->fleets[0]->GetId();
    auto& fdq0 = catch_at_age_model->GetFleetDerivedQuantities(fleet0_id);
    return fdq0["landings_weight_at_age"][i_age_year];
  }
};

static void BM_CatchNumbersAtAge(benchmark::State& state) {
  BenchCAAEvaluate fx;
  fx.Init();

  for (auto _ : state) {
    double result = fx.RunBenchmarkedCode();
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_CatchNumbersAtAge);

}  // namespace

