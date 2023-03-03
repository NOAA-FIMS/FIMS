# Second level CMakeLists.txt: register individual tests

include(GoogleTest)

# Add tests: specify add_executable and target_link_libraries

# test_population_dynamics_selectivity_logistic
add_executable(population_dynamics_selectivity_logistic
  test_population_dynamics_selectivity_logistic.cpp
)

target_link_libraries(population_dynamics_selectivity_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_selectivity_logistic)

# test_population_dynamics_maturity_logistic
add_executable(population_dynamics_maturity_logistic
  test_population_dynamics_maturity_logistic.cpp
)

target_link_libraries(population_dynamics_maturity_logistic
gtest_main
fims_test
)

# test_population_dynamics_selectivity_double_logistic
add_executable(population_dynamics_selectivity_double_logistic
  test_population_dynamics_selectivity_double_logistic.cpp
)

target_link_libraries(population_dynamics_selectivity_double_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_maturity_logistic)
gtest_discover_tests(population_dynamics_selectivity_double_logistic)

# test_fims_math_exp.cpp
add_executable(fims_math_exp
  test_fims_math_exp.cpp
)

target_link_libraries(fims_math_exp
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_exp)

# test_fims_math_log.cpp
add_executable(fims_math_log
  test_fims_math_log.cpp
)

target_link_libraries(fims_math_log
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_log)

# test_fims_math_logistic.cpp
add_executable(fims_math_logistic
  test_fims_math_logistic.cpp
)

target_link_libraries(fims_math_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_logistic)

# test_fims_math_double_logistic.cpp
add_executable(fims_math_double_logistic
  test_fims_math_double_logistic.cpp
)

target_link_libraries(fims_math_double_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_double_logistic)

# test_fims_math_ad_fab_min_max.cpp
add_executable(fims_math_fabs_min_max
  test_fims_math_ad_fabs_min_max.cpp
)

target_link_libraries(fims_math_fabs_min_max
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_fabs_min_max)

# test_growth.cpp
add_executable(growth_evaluate
  test_growth.cpp
)

target_link_libraries(growth_evaluate
  gtest_main
  fims_test
)

gtest_discover_tests(growth_evaluate)

# test_population_dynamics_recruitment_base.cpp
add_executable(population_dynamics_recruitment_base
test_population_dynamics_recruitment_base.cpp
)

target_link_libraries(population_dynamics_recruitment_base
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_recruitment_base)

# test_sr_beverton_holt.cpp
add_executable(sr_beverton_holt
test_sr_beverton_holt.cpp
)

target_link_libraries(sr_beverton_holt
  gtest_main
  fims_test
)

gtest_discover_tests(sr_beverton_holt)

# test_population_dynamics_fleet.cpp
add_executable(population_dynamics_fleet_initialize_prepare
  test_population_dynamics_fleet_initialize_prepare.cpp
)

target_link_libraries(population_dynamics_fleet_initialize_prepare
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_fleet_initialize_prepare)

# test_population_dynamics_population.cpp
add_executable(population_dynamics_population_initialize_prepare
  test_population_dynamics_population_initialize_prepare.cpp
)

target_link_libraries(population_dynamics_population_initialize_prepare
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_population_initialize_prepare)

# test_population_Li.cpp
add_executable(population_unfished
  test_population_Unfished_Initial.cpp
)

target_link_libraries(population_unfished
  gtest_main
  fims_test
)

gtest_discover_tests(population_unfished)

#test_population_stawitz.cpp
add_executable(population_NumbersAtAge
  test_population_NumbersAtAge.cpp
)

target_link_libraries(population_NumbersAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(population_NumbersAtAge)

#test_maturityAA.cpp
add_executable(population_MaturityAtAge
  test_population_MaturityAtAge.cpp
)

target_link_libraries(population_MaturityAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(population_MaturityAtAge)

#test_population_CatchNumbersAtAge.cpp
add_executable(population_CatchNumbersAtAge
  test_population_CatchNumbersAtAge.cpp
)

target_link_libraries(population_CatchNumbersAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(population_CatchNumbersAtAge)

#test_CalculateCatch.cpp
add_executable(population_Catch
  test_population_Catch.cpp
)

target_link_libraries(population_Catch
  gtest_main
  fims_test
)

gtest_discover_tests(population_Catch)

#test_CalculateSSB.cpp
add_executable(population_SSB
  test_population_SSB.cpp
)

target_link_libraries(population_SSB
  gtest_main
  fims_test
)

gtest_discover_tests(population_SSB)

#test_CalculateIndex_works.cpp
add_executable(population_Index
  test_population_Index.cpp
)

target_link_libraries(population_Index
  gtest_main
  fims_test
)

gtest_discover_tests(population_Index)

gtest_discover_tests(fims_math_fabs_min_max)

# test_fims_math_ad_fab_min_max.cpp
add_executable(fims_math_logit
  test_fims_math_logit.cpp
)

target_link_libraries(fims_math_logit
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_logit)


# test_integration
add_executable(population_integration
  integration_test_population.cpp
)

 target_link_libraries(population_integration
  gtest_main
  fims_test
 )

 gtest_discover_tests(population_integration)
