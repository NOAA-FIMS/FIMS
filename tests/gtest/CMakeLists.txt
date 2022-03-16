# Second level CMakeLists.txt: register individual tests

include(GoogleTest)

# Add tests: specify add_executable and target_link_libraries 

# test_fims_math.cpp
add_executable(fims_math
  test_fims_math.cpp
)

target_link_libraries(fims_math
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math)

# test_place_holder1.cpp
add_executable(test_place_holder1
  test_place_holder1.cpp
)

target_link_libraries(test_place_holder1
  gtest_main
  fims_test
)

gtest_discover_tests(test_place_holder1)

# test_place_holder.cpp
add_executable(test_place_holder2
  test_place_holder2.cpp
)

target_link_libraries(test_place_holder2
  gtest_main
  fims_test
)

gtest_discover_tests(test_place_holder2)

# Benchmarking examples
add_executable(benchmark_place_holder1
  benchmark_place_holder1.cpp
)

target_link_libraries(benchmark_place_holder1
  fims_test
  benchmark_main
)
