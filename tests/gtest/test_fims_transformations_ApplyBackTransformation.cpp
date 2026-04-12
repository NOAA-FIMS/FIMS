#include "gtest/gtest.h"
#include "common/fims_transformations.hpp"

namespace {

TEST(FimsTransformations, TransformationLabelToString) {
  EXPECT_STREQ(
      fims_transformations::TransformationLabelToString(
          fims_transformations::Transformation::TransformationLabel::identity),
      "identity");
  EXPECT_STREQ(
      fims_transformations::TransformationLabelToString(
          fims_transformations::Transformation::TransformationLabel::log),
      "log");
  EXPECT_STREQ(
      fims_transformations::TransformationLabelToString(
          fims_transformations::Transformation::TransformationLabel::logit),
      "logit");
  EXPECT_STREQ(
      fims_transformations::TransformationLabelToString(
          fims_transformations::Transformation::TransformationLabel::square),
      "square");
}

TEST(FimsTransformations, AddLogJacobianLogAndIdentity) {
  EXPECT_DOUBLE_EQ(
      fims_transformations::AddLogJacobian<double>(
          1.75,
          fims_transformations::Transformation::TransformationLabel::log),
      1.75);

  EXPECT_DOUBLE_EQ(
      fims_transformations::AddLogJacobian<double>(
          1.75,
          fims_transformations::Transformation::TransformationLabel::identity),
      0.0);
}

TEST(FimsTransformations, ApplyBackTransformationIdentitySd) {
  fims_transformations::Transformation transformation;
  transformation.label =
      fims_transformations::Transformation::TransformationLabel::identity;
  transformation.uncertainty_label =
      fims_transformations::Transformation::UncertaintyLabel::sd;

  EXPECT_DOUBLE_EQ(
      fims_transformations::ApplyBackTransformation<double>(0.25,
                                                            transformation),
      0.25);
}

TEST(FimsTransformations, ApplyBackTransformationLogSd) {
  fims_transformations::Transformation transformation;
  transformation.label =
      fims_transformations::Transformation::TransformationLabel::log;
  transformation.uncertainty_label =
      fims_transformations::Transformation::UncertaintyLabel::sd;

  EXPECT_EQUAL(
      fims_transformations::ApplyBackTransformation<double>(log(2.0),
                                                            transformation),
      2.0);
}

TEST(FimsTransformations, ApplyBackTransformationLogitWithBounds) {
  fims_transformations::Transformation transformation;
  transformation.label =
      fims_transformations::Transformation::TransformationLabel::logit;
  transformation.uncertainty_label =
      fims_transformations::Transformation::UncertaintyLabel::sd;
  transformation.args.lower = 0.0;
  transformation.args.upper = 10.0;

  EXPECT_EQUAL(
      fims_transformations::ApplyBackTransformation<double>(0.0,
                                                            transformation),
      5.0);
}

TEST(FimsTransformations, ApplyBackTransformationVarReturnsSd) {
  fims_transformations::Transformation transformation;
  transformation.label =
      fims_transformations::Transformation::TransformationLabel::identity;
  transformation.uncertainty_label =
      fims_transformations::Transformation::UncertaintyLabel::var;

  EXPECT_DOUBLE_EQ(
      fims_transformations::ApplyBackTransformation<double>(9.0,
                                                            transformation),
      3.0);
}


TEST(FimsTransformations, ApplyBackTransformationLogVarReturnsSd) {
  fims_transformations::Transformation transformation;
  transformation.label =
      fims_transformations::Transformation::TransformationLabel::log;
  transformation.uncertainty_label =
      fims_transformations::Transformation::UncertaintyLabel::var;

  EXPECT_DOUBLE_EQ(
      fims_transformations::ApplyBackTransformation<double>(log(9.0),
                                                            transformation),
      3.0);
}

}  // namespace
