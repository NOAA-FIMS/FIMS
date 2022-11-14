#include "benchmark/benchmark.h"
#include "common/fims_math.hpp"

void BM_fims_math_logistic(benchmark::State &state)
{
  for (auto _ : state) {
      // median, slope, x
      fims::logistic(4.0, 2.5, 7.0);
  }
    
}
BENCHMARK(BM_fims_math_logistic);