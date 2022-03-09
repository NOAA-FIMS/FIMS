# Top level CMakeLists.txt: setting up top level project dependencies and options
cmake_minimum_required(VERSION 3.20)
project(FIMS
  CXX
) # CXX is the language name


# find the R binary
cmake_minimum_required (VERSION 3.0)

project(FIMS)

MESSAGE("generating make files for ${CMAKE_PROJECT_NAME}")

MESSAGE(STATUS "Looking for R executable")

		EXECUTE_PROCESS(
			COMMAND which R 
			OUTPUT_VARIABLE R_EXECUTABLE)

MESSAGE(STATUS "R_EXECUTABLE is ${R_EXECUTABLE}") 


EXECUTE_PROCESS(
		COMMAND R "--slave" "--no-save" "-e" "cat(R.home())"
		OUTPUT_VARIABLE R_HOME)

MESSAGE(STATUS "R_HOME is ${R_HOME}")
SET(R_INCLUDE ${R_HOME}/include)
include_directories("${R_INCLUDE}")

SET(R_LIB ${R_HOME}/lib/libR)
#link_directories(${R_LIB}/lib)

#set TMB home
SET(TMB_HOME ${R_HOME}/library/TMB)

MESSAGE(STATUS "TMB_HOME is ${TMB_HOME}")

SET(TMB_INCLUDE ${TMB_HOME}/include)

find_library(_LIB_TMB
	NAMES TMB.so
	PATHS ${TMB_HOME}/libs
	NO_CACHE
	REQUIRED)

#link_directories(${TMB_HOME}/lib/TMB.so)

MESSAGE(STATUS "TMB_INCLUDE is ${TMB_INCLUDE}")
MESSAGE(STATUS "_LIB_TMB is ${_LIB_TMB}")

include_directories("${TMB_INCLUDE}")

#set Rcpp home
SET(RCPP_HOME ${R_HOME}/library/Rcpp)

SET(RCPP_INCLUDE ${RCPP_HOME}/include)
#link_directories(${R_HOME}/library/Rcpp/lib)
find_library(_LIB_RCPP
	NAMES Rcpp.so
	PATHS ${RCPP_HOME}/libs
	NO_CACHE
	REQUIRED)


MESSAGE(STATUS "RCPP_INCLUDE is ${RCPP_INCLUDE}")
MESSAGE(STATUS "R_LIB is ${R_LIB}")


MESSAGE(STATUS "_LIB_RCPP is ${_LIB_RCPP}")
include_directories("${RCPP_INCLUDE}")

link_directories(${TMB_HOME}/libs)
link_directories(${RCPP_HOME}/libs)
link_directories(${R_HOME}/lib)



#target_link_options(TARGETS PRIVATE "-L${_LIB_RCPP}")



#set Eigen home
SET(EIGEN_HOME ${R_HOME}/library/RcppEigen)

MESSAGE(STATUS "EIGEN_HOME is ${EIGEN_HOME}")

SET(EIGEN_INCLUDE ${EIGEN_HOME}/include)

MESSAGE(STATUS "EIGEN_INCLUDE is ${EIGEN_INCLUDE}")

include_directories("${EIGEN_INCLUDE}")

include(GenerateExportHeader)
include_directories("${CMAKE_CURRENT_BINARY_DIR}") # FIMS_export.h

add_library(FIMS SHARED src/fims_tmb_objective_function.cpp)
generate_export_header(FIMS)

install(TARGETS FIMS DESTINATION libs)


target_link_libraries(FIMS R)



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

add_definitions(-DTMB_MODEL)

add_subdirectory(tests/gtest)




