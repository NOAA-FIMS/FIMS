#include "gtest/gtest.h"
#include "common/fims_transformations.hpp"

namespace {


// ============================================================
// TransformPrior (vector overload) Tests
// ============================================================

TEST(TransformPriorVector, IdenticalTransformationsReturnInputUnchanged) {
    fims::Transformation trans;
    trans.label = fims::Transformation::Label::log;

    fims::Vector<double> input;
    input.resize(3);
    input[0] = 1.0;
    input[1] = 2.0;
    input[2] = 3.0;

    // Same input and prior transformation - should return input unchanged
    fims::Vector<double> result = fims_transformations::TransformPrior(input, trans, trans);
    EXPECT_NEAR(result[0], 1.0, 1e-8);
    EXPECT_NEAR(result[1], 2.0, 1e-8);
    EXPECT_NEAR(result[2], 3.0, 1e-8);
}

TEST(TransformPriorVector, LogInputToNaturalScale) {
    fims::Transformation log_trans;
    log_trans.label = fims::Transformation::Label::log;

    fims::Transformation identity_trans;
    identity_trans.label = fims::Transformation::Label::identity;

    // Input is log(sd), prior is on natural scale (sd)
    fims::Vector<double> input;
    input.resize(3);
    input[0] = std::log(1.0);
    input[1] = std::log(2.0);
    input[2] = std::log(3.0);

    fims::Vector<double> result = fims_transformations::TransformPrior(
        input, log_trans, identity_trans);

    EXPECT_NEAR(result[0], 1.0, 1e-8);
    EXPECT_NEAR(result[1], 2.0, 1e-8);
    EXPECT_NEAR(result[2], 3.0, 1e-8);
}

TEST(TransformPriorVector, LogInputToVarianceScale) {
    fims::Transformation log_trans;
    log_trans.label = fims::Transformation::Label::log;

    fims::Transformation square_trans;
    square_trans.label = fims::Transformation::Label::square;

    // Input is log(sd), prior is on variance (sd^2)
    // log(sd) -> exp -> sd -> square -> sd^2
    double sd = 2.0;
    fims::Vector<double> input;
    input.resize(1);
    input[0] = std::log(sd);

    fims::Vector<double> result = fims_transformations::TransformPrior(
        input, log_trans, square_trans);

    EXPECT_NEAR(result[0], sd * sd, 1e-8);
}

TEST(TransformPriorVector, PreservesVectorSize) {
    fims::Transformation log_trans;
    log_trans.label = fims::Transformation::Label::log;

    fims::Transformation identity_trans;
    identity_trans.label = fims::Transformation::Label::identity;

    fims::Vector<double> input;
    input.resize(5);
    for (size_t i = 0; i < 5; i++) input[i] = std::log(i + 1.0);

    fims::Vector<double> result = fims_transformations::TransformPrior(
        input, log_trans, identity_trans);

    EXPECT_EQ(5, result.size());
}

// ============================================================
// TransformPrior (scalar overload) Tests
// ============================================================

TEST(TransformPriorScalar, LogInputToNaturalScale) {
    fims::Transformation log_trans;
    log_trans.label = fims::Transformation::Label::log;

    fims::Transformation identity_trans;
    identity_trans.label = fims::Transformation::Label::identity;

    double log_sd = std::log(3.0);
    double result = fims_transformations::TransformPrior(
        log_sd, log_trans, identity_trans);

    EXPECT_NEAR(result, 3.0, 1e-8);
}

TEST(TransformPriorScalar, LogInputToVarianceScale) {
    fims::Transformation log_trans;
    log_trans.label = fims::Transformation::Label::log;

    fims::Transformation square_trans;
    square_trans.label = fims::Transformation::Label::square;

    double sd = 2.5;
    double result = fims_transformations::TransformPrior(
        std::log(sd), log_trans, square_trans);

    EXPECT_NEAR(result, sd * sd, 1e-8);
}

TEST(TransformPriorScalar, IdentityInputToSquareScale) {
    fims::Transformation identity_trans;
    identity_trans.label = fims::Transformation::Label::identity;

    fims::Transformation square_trans;
    square_trans.label = fims::Transformation::Label::square;

    double x = 4.0;
    double result = fims_transformations::TransformPrior(
        x, identity_trans, square_trans);

    EXPECT_NEAR(result, 16.0, 1e-8);
}


}