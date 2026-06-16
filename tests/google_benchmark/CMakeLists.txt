# CMake configuration for Google Benchmark-based performance tests

# benchmark_Population_CatchAtAge_CatchNumbersAtAge.cpp
add_executable(benchmark_Population_CatchAtAge_CatchNumbersAtAge
  benchmark_Population_CatchAtAge_CatchNumbersAtAge.cpp
)

target_link_libraries(benchmark_Population_CatchAtAge_CatchNumbersAtAge
  benchmark::benchmark_main
  fims_test
  GTest::gtest
)
