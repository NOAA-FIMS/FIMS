#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "transformations/functors/parameter_transformations_base.hpp"
#include "transformations/functors/parameter_transformations_log.hpp"
#include <memory>

namespace
{

  // Test LogTransformation using double values
  // Tests the transformation from log scale to natural scale: natural = exp(log)
  TEST(LogTransformation, TransformLogToNatural)
  {
    fims_transformations::LogTransformation<double> log_transform;
    
    // Test with log value of 0.0 (natural = exp(0) = 1.0)
    double log_val = 0.0;
    double natural_val = 0.0;
    log_transform.Transform(log_val, natural_val);
    EXPECT_DOUBLE_EQ(natural_val, 1.0);
    
    // Test with log value of 1.0 (natural = exp(1) ≈ 2.71828)
    log_val = 1.0;
    natural_val = 0.0;
    log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 2.71828, 0.0001);
    
    // Test with log value of 2.0 (natural = exp(2) ≈ 7.38906)
    log_val = 2.0;
    natural_val = 0.0;
    log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 7.38906, 0.0001);
    
    // Test with negative log value -1.0 (natural = exp(-1) ≈ 0.36788)
    log_val = -1.0;
    natural_val = 0.0;
    log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 0.36788, 0.0001);

    // Test with large positive log value
    log_val = 10.0;
    natural_val = 0.0;
    log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 22026.4658, 0.001);
  }

  // Test InverseLogTransformation using double values
  // Tests the transformation from natural scale to log scale: log = log(natural)
  TEST(InverseLogTransformation, TransformNaturalToLog)
  {
    fims_transformations::InverseLogTransformation<double> inverse_log_transform;
    
    // Test with natural value of 1.0 (log = log(1) = 0.0)
    double log_val = 0.0;
    double natural_val = 1.0;
    inverse_log_transform.Transform(log_val, natural_val);
    EXPECT_DOUBLE_EQ(log_val, 0.0);
    
    // Test with natural value of 2.71828 (log = log(e) ≈ 1.0)
    log_val = 0.0;
    natural_val = 2.71828;
    inverse_log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(log_val, 1.0, 0.0001);
    
    // Test with natural value of 7.38906 (log = log(exp(2)) ≈ 2.0)
    log_val = 0.0;
    natural_val = 7.38906;
    inverse_log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(log_val, 2.0, 0.0001);
    
    // Test with natural value of 0.36788 (log = log(exp(-1)) ≈ -1.0)
    log_val = 0.0;
    natural_val = 0.36788;
    inverse_log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(log_val, -1.0, 0.0001);

    // Test with large natural value
    log_val = 0.0;
    natural_val = 1000.0;
    inverse_log_transform.Transform(log_val, natural_val);
    EXPECT_NEAR(log_val, 6.907755, 0.0001);
  }

  // Test that LogTransformation is a valid pointer to ParameterTransformationBase
  TEST(LogTransformation, IsParameterTransformationBase)
  {
    auto log_transform = std::make_shared<fims_transformations::LogTransformation<double>>();
    std::shared_ptr<fims_transformations::ParameterTransformationBase<double>> base_ptr = log_transform;
    
    EXPECT_TRUE(base_ptr != nullptr);
    
    // Verify the transformation still works through the base pointer
    double log_val = 0.0;
    double natural_val = 0.0;
    base_ptr->Transform(log_val, natural_val);
    EXPECT_DOUBLE_EQ(natural_val, 1.0);
  }

  // Test that InverseLogTransformation is a valid pointer to ParameterTransformationBase
  TEST(InverseLogTransformation, IsParameterTransformationBase)
  {
    auto inverse_log_transform = std::make_shared<fims_transformations::InverseLogTransformation<double>>();
    std::shared_ptr<fims_transformations::ParameterTransformationBase<double>> base_ptr = inverse_log_transform;
    
    EXPECT_TRUE(base_ptr != nullptr);
    
    // Verify the transformation still works through the base pointer
    double log_val = 0.0;
    double natural_val = 2.71828;
    base_ptr->Transform(log_val, natural_val);
    EXPECT_NEAR(log_val, 1.0, 0.0001);
  }

  // Test CreateLogTransformation factory function
  // When natural_initial is set and log_initial is default, should return InverseLogTransformation
  TEST(CreateLogTransformation, ReturnsInverseWhenNaturalSet)
  {
    // Natural value is set, log value is default (-999)
    auto transform = fims_transformations::CreateLogTransformation<double>(-999.0, 5.0);
    
    EXPECT_TRUE(transform != nullptr);
    
    // Test that it performs inverse transformation (natural -> log)
    // If we set natural=5.0, the transform should update log to log(5.0)
    double log_val = 0.0;
    double natural_val = 5.0;
    transform->Transform(log_val, natural_val);
    EXPECT_NEAR(log_val, 1.60944, 0.0001);  // log(5.0) ≈ 1.60944
  }

  // Test CreateLogTransformation factory function
  // When log_initial is set (or both are default), should return LogTransformation
  TEST(CreateLogTransformation, ReturnsLogWhenLogSet)
  {
    // Log value is set, natural value is default (-999)
    auto transform = fims_transformations::CreateLogTransformation<double>(1.5, -999.0);
    
    EXPECT_TRUE(transform != nullptr);
    
    // Test that it performs log transformation (log -> natural)
    // If we set log=1.5, the transform should update natural to exp(1.5)
    double log_val = 1.5;
    double natural_val = 0.0;
    transform->Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 4.48169, 0.0001);  // exp(1.5) ≈ 4.48169
  }

  // Test CreateLogTransformation factory function
  // When both values are default, should return LogTransformation
  TEST(CreateLogTransformation, ReturnsLogWhenBothDefault)
  {
    // Both values are default (-999)
    auto transform = fims_transformations::CreateLogTransformation<double>(-999.0, -999.0);
    
    EXPECT_TRUE(transform != nullptr);
    
    // Test that it performs log transformation (log -> natural)
    double log_val = 2.0;
    double natural_val = 0.0;
    transform->Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 7.38906, 0.0001);  // exp(2.0) ≈ 7.38906
  }

  // Test CreateLogTransformation factory function
  // When both values are set (non-default), should return LogTransformation (default behavior)
  TEST(CreateLogTransformation, ReturnsLogWhenBothSet)
  {
    // Both values are set (not default)
    auto transform = fims_transformations::CreateLogTransformation<double>(1.0, 2.71828);
    
    EXPECT_TRUE(transform != nullptr);
    
    // Should default to log transformation (log -> natural)
    double log_val = 1.0;
    double natural_val = 0.0;
    transform->Transform(log_val, natural_val);
    EXPECT_NEAR(natural_val, 2.71828, 0.0001);  // exp(1.0) ≈ 2.71828
  }

  // Test round-trip transformation consistency
  // Transform from log to natural and back should recover the original value
  TEST(LogTransformation, RoundTripConsistency)
  {
    fims_transformations::LogTransformation<double> log_transform;
    fims_transformations::InverseLogTransformation<double> inverse_log_transform;
    
    // Start with log value
    double original_log = 1.5;
    double log_val = original_log;
    double natural_val = 0.0;
    
    // Transform log -> natural
    log_transform.Transform(log_val, natural_val);
    
    // Transform natural -> log
    inverse_log_transform.Transform(log_val, natural_val);
    
    // Should recover the original log value
    EXPECT_NEAR(log_val, original_log, 0.0001);
  }

}
