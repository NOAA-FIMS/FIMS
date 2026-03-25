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

gtest_discover_tests(population_dynamics_maturity_logistic DISCOVERY_TIMEOUT 60)

# test_fims_math_exp.cpp
add_executable(fims_math_exp
  test_fims_math_exp.cpp
)

target_link_libraries(fims_math_exp
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_exp DISCOVERY_TIMEOUT 60)

# test_fims_math_log.cpp
add_executable(fims_math_log
  test_fims_math_log.cpp
)

target_link_libraries(fims_math_log
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_log DISCOVERY_TIMEOUT 60)

# test_fims_math_logistic.cpp
add_executable(fims_math_logistic
  test_fims_math_logistic.cpp
)

target_link_libraries(fims_math_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_logistic DISCOVERY_TIMEOUT 60)

# test_fims_math_double_logistic.cpp
add_executable(fims_math_double_logistic
  test_fims_math_double_logistic.cpp
)

target_link_libraries(fims_math_double_logistic
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_double_logistic DISCOVERY_TIMEOUT 60)

# test_fims_math_ad_fab_min_max.cpp
add_executable(fims_math_fabs_min_max
  test_fims_math_ad_fabs_min_max.cpp
)

target_link_libraries(fims_math_fabs_min_max
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_fabs_min_max DISCOVERY_TIMEOUT 60)

# test_growth.cpp
add_executable(growth_evaluate
  test_growth.cpp
)

target_link_libraries(growth_evaluate
  gtest_main
  fims_test
)

gtest_discover_tests(growth_evaluate DISCOVERY_TIMEOUT 60)

# test_info_setup_random_effects.cpp
add_executable(info_setup_random_effects
  test_info_setup_random_effects.cpp
)
target_link_libraries(info_setup_random_effects
  gtest_main
  fims_test
)
gtest_discover_tests(info_setup_random_effects DISCOVERY_TIMEOUT 60)

# test_info_setup_priors.cpp
 add_executable(info_setup_priors
   test_info_setup_priors.cpp
 )
 target_link_libraries(info_setup_priors
   gtest_main
   fims_test
 )
 gtest_discover_tests(info_setup_priors DISCOVERY_TIMEOUT 60)

# test_population_dynamics_recruitment_base.cpp
add_executable(population_dynamics_recruitment_base
test_population_dynamics_recruitment_base.cpp
)

target_link_libraries(population_dynamics_recruitment_base
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_recruitment_base DISCOVERY_TIMEOUT 60)

# test_population_dynamics_recruitment_process.cpp
add_executable(population_dynamics_recruitment_process
test_population_dynamics_recruitment_process.cpp
)

target_link_libraries(population_dynamics_recruitment_process
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_recruitment_process DISCOVERY_TIMEOUT 60)

# test_sr_beverton_holt.cpp
add_executable(sr_beverton_holt
test_sr_beverton_holt.cpp
)

target_link_libraries(sr_beverton_holt
  gtest_main
  fims_test
)

gtest_discover_tests(sr_beverton_holt DISCOVERY_TIMEOUT 60)

# test_population_dynamics_fleet.cpp
add_executable(population_dynamics_fleet_initialize_prepare
  test_population_dynamics_fleet_initialize_prepare.cpp
)

target_link_libraries(population_dynamics_fleet_initialize_prepare
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_fleet_initialize_prepare DISCOVERY_TIMEOUT 60)

# test_population_dynamics_population.cpp
add_executable(population_dynamics_population_initialize_prepare
  test_population_dynamics_population_initialize_prepare.cpp
)

target_link_libraries(population_dynamics_population_initialize_prepare
  gtest_main
  fims_test
)

gtest_discover_tests(population_dynamics_population_initialize_prepare DISCOVERY_TIMEOUT 60)
# test_population_Li.cpp
add_executable(population_unfished
  test_population_Unfished_Initial.cpp
)

target_link_libraries(population_unfished
  gtest_main
  fims_test
)

gtest_discover_tests(population_unfished DISCOVERY_TIMEOUT 60)
#test_population_stawitz.cpp
add_executable(population_NumbersAtAge
  test_population_NumbersAtAge.cpp
)

target_link_libraries(population_NumbersAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(population_NumbersAtAge DISCOVERY_TIMEOUT 60)
#test_maturityAA.cpp
add_executable(population_MaturityAtAge
  test_population_MaturityAtAge.cpp
)

target_link_libraries(population_MaturityAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(population_MaturityAtAge DISCOVERY_TIMEOUT 60)
#test_population_CatchNumbersAtAge.cpp
add_executable(population_CatchNumbersAtAge
  test_population_CatchNumbersAtAge.cpp
)

target_link_libraries(population_CatchNumbersAtAge
  gtest_main
  fims_test
)

gtest_discover_tests(population_CatchNumbersAtAge DISCOVERY_TIMEOUT 60)
#test_CalculateCatch.cpp
add_executable(population_Catch
  test_population_Catch.cpp
)

target_link_libraries(population_Catch
  gtest_main
  fims_test
)

gtest_discover_tests(population_Catch DISCOVERY_TIMEOUT 60)
#test_B_and_SB.cpp
add_executable(population_B_and_SB
  test_population_B_and_SB.cpp
)

target_link_libraries(population_B_and_SB
  gtest_main
  fims_test
)

gtest_discover_tests(population_B_and_SB DISCOVERY_TIMEOUT 60)
#test_CalculateIndex_works.cpp
add_executable(population_Index
  test_population_Index.cpp
)

target_link_libraries(population_Index
  gtest_main
  fims_test
)

gtest_discover_tests(population_Index DISCOVERY_TIMEOUT 60)

# test_population_Recruitment.cpp
add_executable(population_Recruitment
  test_population_Recruitment.cpp
)

target_link_libraries(population_Recruitment
  gtest_main
  fims_test
)

gtest_discover_tests(population_Recruitment DISCOVERY_TIMEOUT 60)
# test_fims_math_ad_fab_min_max.cpp
add_executable(fims_math_logit
  test_fims_math_logit.cpp
)

target_link_libraries(fims_math_logit
  gtest_main
  fims_test
)

gtest_discover_tests(fims_math_logit DISCOVERY_TIMEOUT 60)
# test_DoubleLogistic_DoubleLogisticSelectivity_Evaluate.cpp
add_executable(DoubleLogistic_DoubleLogisticSelectivity_Evaluate
  test_DoubleLogistic_DoubleLogisticSelectivity_Evaluate.cpp
)

target_link_libraries(DoubleLogistic_DoubleLogisticSelectivity_Evaluate
  gtest_main
  fims_test
)

gtest_discover_tests(DoubleLogistic_DoubleLogisticSelectivity_Evaluate DISCOVERY_TIMEOUT 60)
# test_Logistic_LogisticSelectivity_Evaluate.cpp
add_executable(Logistic_LogisticSelectivity_Evaluate
  test_Logistic_LogisticSelectivity_Evaluate.cpp
)

target_link_libraries(Logistic_LogisticSelectivity_Evaluate
  gtest_main
  fims_test
)

gtest_discover_tests(Logistic_LogisticSelectivity_Evaluate DISCOVERY_TIMEOUT 60)

# test_FIMSJson_JsonParser_WriteToFile.cpp
add_executable(FIMSJson_JsonParser_WriteToFile
  test_FIMSJson_JsonParser_WriteToFile.cpp
)

target_link_libraries(FIMSJson_JsonParser_WriteToFile
  gtest_main
  fims_test
)

gtest_discover_tests(FIMSJson_JsonParser_WriteToFile DISCOVERY_TIMEOUT 60)

# test_FIMSJson_JsonParser_Parse.cpp
add_executable(FIMSJson_JsonParser_Parse
  test_FIMSJson_JsonParser_Parse.cpp
)

target_link_libraries(FIMSJson_JsonParser_Parse
  gtest_main
  fims_test
)

gtest_discover_tests(FIMSJson_JsonParser_Parse DISCOVERY_TIMEOUT 60)

# test_fims_math_ddiric_multinom.cpp
add_executable(FimsMathDdiricMultinomTest
  test_fims_math_ddiric_multinom.cpp
)

target_link_libraries(FimsMathDdiricMultinomTest
  gtest_main
  fims_test
)

gtest_discover_tests(FimsMathDdiricMultinomTest DISCOVERY_TIMEOUT 60)
