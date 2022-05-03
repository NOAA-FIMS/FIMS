# Top level CMakeLists.txt: setting up top level project dependencies and options
cmake_minimum_required(VERSION 3.14)
project(FIMS
  CXX
) # CXX is the language name

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)

# Set up GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
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

# Set up doxygen: download doxygen 1.9.3 and add it to the PATH
option(BUILD_DOC "Build documentation" ON)
IF(BUILD_DOC)
  find_package(Doxygen
               REQUIRED dot)

  # Disable all preprocessing so doxygen will read all statements 
  # when evaluating conditional compilation statements (e.g., #ifdef in fims_math.h)
  set(DOXYGEN_ENABLE_PREPROCESSING NO) 
  set(DOXYGEN_QUIET YES)
  set(DOXYGEN_HAVE_DOT YES)

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

enable_testing()

add_library(fims_test INTERFACE)

target_include_directories(fims_test
  INTERFACE
    inst/include
    ${rcpp_SOURCE_DIR}/inst/include
    ${R_HOME}/include
)


#SET(R_LIB ${R_HOME}/lib/libR)

add_definitions(-DSTD_LIB)

target_compile_definitions(fims_test 
  INTERFACE
    STD_LIB
)

add_subdirectory(tests/gtest)