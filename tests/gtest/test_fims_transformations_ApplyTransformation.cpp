#include "gtest/gtest.h"
#include "common/fims_transformations.hpp"

namespace {


// ============================================================
// ApplyTransformation Tests
// ============================================================


TEST(ApplyTransformation, IdentityReturnsInputUnchanged) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::identity;

    EXPECT_EQ(fims_transformations::ApplyTransformation(0.0, trans), 0.0);
    EXPECT_EQ(fims_transformations::ApplyTransformation(1.0, trans), 1.0);
    EXPECT_EQ(fims_transformations::ApplyTransformation(-1.0, trans), -1.0);
    EXPECT_EQ(fims_transformations::ApplyTransformation(100.0, trans), 100.0);
}

TEST(ApplyTransformation, LogTransformMatchesStdLog) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::log;

    EXPECT_EQ(fims_transformations::ApplyTransformation(1.0, trans), std::log(1.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(10.0, trans), std::log(10.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(0.5, trans), std::log(0.5));
    EXPECT_EQ(fims_transformations::ApplyTransformation(100.0, trans), std::log(100.0));
}

TEST(ApplyTransformation, ExpTransformMatchesStdExp) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::exp;

    EXPECT_EQ(fims_transformations::ApplyTransformation(0.0, trans), std::exp(0.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(1.0, trans), std::exp(1.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(-1.0, trans), std::exp(-1.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(3.0, trans), std::exp(3.0));
}

TEST(ApplyTransformation, SquareTransformReturnsSquaredValue) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::square;

    EXPECT_EQ(fims_transformations::ApplyTransformation(2.0, trans), 4.0);
    EXPECT_EQ(fims_transformations::ApplyTransformation(3.0, trans), 9.0);
    EXPECT_EQ(fims_transformations::ApplyTransformation(0.5, trans), 0.25);
    EXPECT_EQ(fims_transformations::ApplyTransformation(0.0, trans), 0.0);
}

TEST(ApplyTransformation, SqrtTransformMatchesStdSqrt) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::sqrt;

    EXPECT_EQ(fims_transformations::ApplyTransformation(4.0, trans), std::sqrt(4.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(9.0, trans), std::sqrt(9.0));
    EXPECT_EQ(fims_transformations::ApplyTransformation(0.25, trans), std::sqrt(0.25));
    EXPECT_EQ(fims_transformations::ApplyTransformation(0.0, trans), 0.0);
}

TEST(ApplyTransformation, LogitTransformDefaultBounds) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::logit;
    // Default bounds: lower = 0.0, upper = 1.0

    // logit(0.5) = 0
    EXPECT_EQ(fims_transformations::ApplyTransformation(0.5, trans), 0.0);
    // logit(0.731) ~ 1.0
    EXPECT_NEAR(fims_transformations::ApplyTransformation(0.731059, trans), 1.0, 1e-5);
    // logit(0.269) ~ -1.0
    EXPECT_NEAR(fims_transformations::ApplyTransformation(0.268941, trans), -1.0, 1e-5);
}

TEST(ApplyTransformation, LogitTransformCustomBounds) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::logit;
    trans.args.lower = 0.2;
    trans.args.upper = 1.0;

    // At midpoint of [0.2, 1.0] = 0.6, logit should be ~ 0
    EXPECT_NEAR(fims_transformations::ApplyTransformation(0.6, trans), 0.0, 1e-5);
}

TEST(ApplyTransformation, UnsupportedLabelThrows) {
    fims::Transformation trans;
    // Force an invalid label value
    trans.label = static_cast<fims::Transformation::Label>(999);

    EXPECT_THROW(
        fims_transformations::ApplyTransformation(1.0, trans),
        std::invalid_argument
    );
}


// ============================================================
// ApplyBackTransformation Tests
// ============================================================



TEST(ApplyBackTransformation, IdentityReturnsInputUnchanged) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::identity;

    EXPECT_EQ(fims_transformations::ApplyBackTransformation(0.0, trans), 0.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(1.0, trans), 1.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(-1.0, trans), -1.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(100.0, trans), 100.0);
}

TEST(ApplyBackTransformation, LogBackTransformIsExp) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::log;

    // Back transform of log is exp
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(0.0, trans), std::exp(0.0));
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(1.0, trans), std::exp(1.0));
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(-1.0, trans), std::exp(-1.0));
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(3.0, trans), std::exp(3.0));
}

TEST(ApplyBackTransformation, ExpBackTransformIsLog) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::exp;

    // Back transform of exp is log
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(1.0, trans), std::log(1.0));
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(10.0, trans), std::log(10.0));
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(0.5, trans), std::log(0.5));
}

TEST(ApplyBackTransformation, SquareBackTransformIsSqrt) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::square;

    // Back transform of square is sqrt
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(4.0, trans), 2.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(9.0, trans), 3.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(0.25, trans), 0.5);
}

TEST(ApplyBackTransformation, SqrtBackTransformIsSquare) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::sqrt;

    // Back transform of sqrt is square
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(2.0, trans), 4.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(3.0, trans), 9.0);
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(0.5, trans), 0.25);
}

TEST(ApplyBackTransformation, LogitBackTransformIsInvLogit) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::logit;
    // Default bounds: lower = 0.0, upper = 1.0

    // inv_logit(0) = 0.5
    EXPECT_EQ(fims_transformations::ApplyBackTransformation(0.0, trans), 0.5);
    // inv_logit(1) ~ 0.731
    EXPECT_NEAR(fims_transformations::ApplyBackTransformation(1.0, trans), 0.731059, 1e-5);
    // inv_logit(-1) ~ 0.269
    EXPECT_NEAR(fims_transformations::ApplyBackTransformation(-1.0, trans), 0.268941, 1e-5);
}

TEST(ApplyBackTransformation, UnsupportedLabelThrows) {
    fims::Transformation trans;
    trans.label = static_cast<fims::Transformation::Label>(999);

    EXPECT_THROW(
        fims_transformations::ApplyBackTransformation(1.0, trans),
        std::invalid_argument
    );
}


// ============================================================
// Round-trip tests: ApplyTransformation(ApplyBackTransformation(x)) == x
// ============================================================

TEST(TransformationRoundTrip, LogAndBackLog) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::log;

    double x = 5.0;
    double forward = fims_transformations::ApplyTransformation(x, trans);
    double back = fims_transformations::ApplyBackTransformation(forward, trans);
    EXPECT_NEAR(back, x, 1e-8);
}

TEST(TransformationRoundTrip, ExpAndBackExp) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::exp;

    double x = 2.0;
    double forward = fims_transformations::ApplyTransformation(x, trans);
    double back = fims_transformations::ApplyBackTransformation(forward, trans);
    EXPECT_NEAR(back, x, 1e-8);
}

TEST(TransformationRoundTrip, SquareAndBackSquare) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::square;

    double x = 3.0;
    double forward = fims_transformations::ApplyTransformation(x, trans);
    double back = fims_transformations::ApplyBackTransformation(forward, trans);
    EXPECT_NEAR(back, x, 1e-8);
}

TEST(TransformationRoundTrip, SqrtAndBackSqrt) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::sqrt;

    double x = 7.0;
    double forward = fims_transformations::ApplyTransformation(x, trans);
    double back = fims_transformations::ApplyBackTransformation(forward, trans);
    EXPECT_NEAR(back, x, 1e-8);
}

TEST(TransformationRoundTrip, LogitAndBackLogitDefaultBounds) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::logit;

    double x = 0.7;
    double forward = fims_transformations::ApplyTransformation(x, trans);
    double back = fims_transformations::ApplyBackTransformation(forward, trans);
    EXPECT_NEAR(back, x, 1e-8);
}

TEST(TransformationRoundTrip, LogitAndBackLogitCustomBounds) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::logit;
    trans.args.lower = 0.2;
    trans.args.upper = 1.0;

    double x = 0.8;
    double forward = fims_transformations::ApplyTransformation(x, trans);
    double back = fims_transformations::ApplyBackTransformation(forward, trans);
    EXPECT_NEAR(back, x, 1e-8);
}

}