#include "benchmark/benchmark.h"
#include "common/fims_math.hpp"

void BM_fims_math_double_logistic(benchmark::State &state)
{
  for (auto _ : state) {
      // median1, slope1, median2, slope2, x
      fims::double_logistic(4.0, 2.5, 9.5, 0.5, 7.0);
  }
    
}
BENCHMARK(BM_fims_math_double_logistic);