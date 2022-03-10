# Third level CMakeLists.txt: register individual tests

include(GoogleTest)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.6.0.zip
)
FetchContent_MakeAvailable(googlebenchmark)

enable_testing()

add_library(fims_test INTERFACE)

target_include_directories(fims_test
  INTERFACE
    ../inst/include/interface
)

target_compile_definitions(fims_test 
  INTERFACE
    STD_LIB
)

add_subdirectory(gtest)



# Add tests: specify add_executable and target_link_libraries 
# test_place_holder1.cpp
add_executable(test_place_holder1
  gtest/test_place_holder1.cpp
)

target_link_libraries(test_place_holder1
  gtest_main
  fims_test
)

gtest_discover_tests(test_place_holder1)

# test_place_holder.cpp
add_executable(test_place_holder2
  gtest/test_place_holder2.cpp
)

target_link_libraries(test_place_holder2
  gtest_main
  fims_test
)

# target_compile_definitions(test_place_holder2 
#   PUBLIC
#     STD_LIB
# )

gtest_discover_tests(test_place_holder2)

# Benchmarking examples
add_executable(benchmark_place_holder1
  gtest/benchmark_place_holder1.cpp
)

target_link_libraries(benchmark_place_holder1
  fims_test
  benchmark_main
)
