# Second level CMakeLists.txt: register individual tests

include(GoogleTest)

# Add tests: specify add_executable and target_link_libraries

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

# test_population_dynamics_growth.cpp
add_executable(population_dynamics_growth
  test_population_dynamics_growth.cpp
)

target_link_libraries(population_dynamics_growth
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_growth)

# test_info_setup_random_effects.cpp
add_executable(info_setup_random_effects
  test_info_setup_random_effects.cpp
)
target_link_libraries(info_setup_random_effects
  gtest_main
  fims_test
)
gtest_discover_tests(info_setup_random_effects)

# test_info_setup_priors.cpp
 add_executable(info_setup_priors
   test_info_setup_priors.cpp
 )
 target_link_libraries(info_setup_priors
   gtest_main
   fims_test
 )
 gtest_discover_tests(info_setup_priors)

# test_population_dynamics_recruitment_base.cpp
add_executable(population_dynamics_recruitment_base
test_population_dynamics_recruitment_base.cpp
)

target_link_libraries(population_dynamics_recruitment_base
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_recruitment_base)

# test_population_dynamics_recruitment_process.cpp
add_executable(population_dynamics_recruitment_process
test_population_dynamics_recruitment_process.cpp
)

target_link_libraries(population_dynamics_recruitment_process
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_recruitment_process)

# test_sr_beverton_holt.cpp
add_executable(sr_beverton_holt
test_sr_beverton_holt.cpp
)

target_link_libraries(sr_beverton_holt
  gtest_main
  fims_test
)

gtest_discover_tests(sr_beverton_holt)

# test_caa_fleet.cpp
add_executable(caa_fleet_initialize_prepare
  test_caa_fleet_initialize_prepare.cpp
)

target_link_libraries(caa_fleet_initialize_prepare
  gtest_main
  fims_test
)

gtest_discover_tests(caa_fleet_initialize_prepare)

# test_caa_population.cpp
add_executable(caa_population_initialize_prepare
  test_caa_population_initialize_prepare.cpp
)

target_link_libraries(caa_population_initialize_prepare
  gtest_main
  fims_test
)

gtest_discover_tests(caa_population_initialize_prepare)

# test_caa_Unfished_Initial.cpp
add_executable(caa_unfished
  test_caa_Unfished_Initial.cpp
)

target_link_libraries(caa_unfished
  gtest_main
  fims_test
)

gtest_discover_tests(caa_unfished)

#test_caa_NumbersAtAge.cpp
add_executable(caa_NumbersAtAge
  test_caa_NumbersAtAge.cpp
)

target_link_libraries(caa_NumbersAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(caa_NumbersAtAge)

#test_maturityAA.cpp
add_executable(caa_MaturityAtAge
  test_caa_MaturityAtAge.cpp
)

target_link_libraries(caa_MaturityAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(caa_MaturityAtAge)

#test_caa_CatchNumbersAtAge.cpp
add_executable(caa_CatchNumbersAtAge
  test_caa_CatchNumbersAtAge.cpp
)

target_link_libraries(caa_CatchNumbersAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(caa_CatchNumbersAtAge)

#test_CalculateCatch.cpp
add_executable(caa_Catch
  test_caa_Catch.cpp
)

target_link_libraries(caa_Catch
  gtest_main
  fims_test
)

gtest_discover_tests(caa_Catch)

#test_B_and_SB.cpp
add_executable(caa_B_and_SB
  test_caa_B_and_SB.cpp
)

target_link_libraries(caa_B_and_SB
  gtest_main
  fims_test
)

gtest_discover_tests(caa_B_and_SB)

#test_CalculateIndex_works.cpp
add_executable(caa_Index
  test_caa_Index.cpp
)

target_link_libraries(caa_Index
  gtest_main
  fims_test
)

gtest_discover_tests(caa_Index)

# test_caa_Recruitment.cpp
add_executable(caa_Recruitment
  test_caa_Recruitment.cpp
)

target_link_libraries(caa_Recruitment
  gtest_main
  fims_test
)

gtest_discover_tests(caa_Recruitment)

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
# add_executable(population_integration
#   integration_test_population.cpp
# )

#  target_link_libraries(population_integration
#   gtest_main
#   fims_test
#  )

#  gtest_discover_tests(population_integration)

# test_population_dynamics_selectivity_logistic.cpp
add_executable(population_dynamics_selectivity_logistic
  test_population_dynamics_selectivity_logistic.cpp
)

target_link_libraries(population_dynamics_selectivity_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_selectivity_logistic)


# test_FIMSJson_JsonParser_WriteToFile.cpp
add_executable(FIMSJson_JsonParser_WriteToFile
  test_FIMSJson_JsonParser_WriteToFile.cpp
)

target_link_libraries(FIMSJson_JsonParser_WriteToFile
  gtest_main
  fims_test
)

gtest_discover_tests(FIMSJson_JsonParser_WriteToFile)
