# Top level CMakeLists.txt: setting up top level project dependencies
# and options. CMake Referecen documentation can be found here:
# https://cmake.org/cmake/help/v3.22/
cmake_minimum_required(VERSION 3.14)
project(FIMS
  CXX
) # CXX is the language name

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# Set up GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker
# settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Set up Google benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.6.0.zip
)
FetchContent_MakeAvailable(googlebenchmark)

# Set up Gcov to get coverage results with Google Test
set(CMAKE_CXX_FLAGS --coverage)

# Set up doxygen: download doxygen 1.9.3 and add it to the PATH
option(BUILD_DOC "Build documentation" ON)
IF(BUILD_DOC)
  find_package(Doxygen)

  # Disable all preprocessing so doxygen will read all statements
  # when evaluating conditional compilation statements
  # (e.g., #ifdef in fims_math.h)
  # set(DOXYGEN_ENABLE_PREPROCESSING NO)
  set(DOXYGEN_QUIET YES)
  set(DOXYGEN_USE_MATHJAX YES)

  doxygen_add_docs(fims_doxygen
    inst/include
    src
    ALL
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Building documentation with Doxygen.")
ENDIF()

# Set up Rcpp
FetchContent_Declare(
  rcpp
  URL https://github.com/RcppCore/Rcpp/archive/refs/tags/1.0.8.zip
)
FetchContent_MakeAvailable(rcpp)

# Set up R
MESSAGE(STATUS "Looking for R executable")

EXECUTE_PROCESS(
  COMMAND which R
  OUTPUT_VARIABLE R_EXECUTABLE)

MESSAGE(STATUS "R_EXECUTABLE is ${R_EXECUTABLE}")

EXECUTE_PROCESS(
		COMMAND R "--slave" "--no-save" "-e" "cat(R.home())"
		OUTPUT_VARIABLE R_HOME)

MESSAGE(STATUS "R_HOME is ${R_HOME}")

EXECUTE_PROCESS(
		COMMAND R "--slave" "--no-save" "-e" "remotes::install_local(upgrade = 'always')")
EXECUTE_PROCESS(
		COMMAND R "--slave" "--no-save" "-e" "FIMS:::setup_gtest()")

# Enable testing for current directory and below
enable_testing()

# Creates an interface library fims_test
add_library(fims_test INTERFACE)

# Add include directories to the fims_test target
target_include_directories(fims_test
  INTERFACE
    inst/include
    ${rcpp_SOURCE_DIR}/inst/include
    ${R_HOME}/include
)

# Add compile definition STD_LIB to the fims_test target.
target_compile_definitions(fims_test
  INTERFACE
    STD_LIB
)

# Add a subdirectory to the build
add_subdirectory(tests/gtest)
add_subdirectory(tests/google_benchmark)
