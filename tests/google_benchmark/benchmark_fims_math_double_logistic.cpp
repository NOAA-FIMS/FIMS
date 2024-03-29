#include "benchmark/benchmark.h"
#include "common/fims_math.hpp"

void BM_fims_math_double_logistic(benchmark::State &state)
{
  for (auto _ : state) {
      // inflection_point_asc, slope_asc, inflection_point_desc, slope_desc, x
      fims_math::double_logistic(4.0, 2.5, 9.5, 0.5, 7.0);
  }
    
}
BENCHMARK(BM_fims_math_double_logistic);