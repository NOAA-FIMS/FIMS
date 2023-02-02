# Second level CMakeLists.txt: register individual benchmarks

# benchmark_fims_math_logistic.cpp
add_executable(benchmark_fims_math_logistic
  benchmark_fims_math_logistic.cpp
)

target_link_libraries(benchmark_fims_math_logistic
  fims_test
  benchmark_main
)

# benchmark_fims_math_double_logistic.cpp
add_executable(benchmark_fims_math_double_logistic
  benchmark_fims_math_double_logistic.cpp
)

target_link_libraries(benchmark_fims_math_double_logistic
  fims_test
  benchmark_main
)