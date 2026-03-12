 # CMake configuration for Google Benchmark-based performance tests

 add_executable(benchmark_population_CatchNumbersAtAge
   benchmark_population_CatchNumbersAtAge.cpp
 )

 target_link_libraries(benchmark_population_CatchNumbersAtAge
   benchmark::benchmark_main
   fims_test
   GTest::gtest
 )

