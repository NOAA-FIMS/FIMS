# Top level CMakeLists.txt: setting up top level project dependencies and options
cmake_minimum_required(VERSION 3.20)
project(FIMS
  CXX
) # CXX is the language name

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

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
    inst/include/interface
)

target_compile_definitions(fims_test 
  INTERFACE
    STD_LIB
)

add_subdirectory(tests/gtest)