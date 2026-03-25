/**
 * @file test_fims_math_ddiric_multinom.cpp
 * @brief Unit test for the Dirichlet-multinomial probability mass function.
 */

#include <gtest/gtest.h>
#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"

TEST(FimsMathDdiricMultinomTest, HandlesOneInput) {
    // Inputs matched to the reference R calculation.
    fims::Vector<double> x(3);
    x[0] = 10.0;
    x[1] = 20.0;
    x[2] = 5.0;

    fims::Vector<double> p(3);
    p[0] = 0.3;
    p[1] = 0.5;
    p[2] = 0.2;

    const double theta = 2.0;

    // Expected values from the reference calculation.
    const double expected_log_prob = -4.254199658272608;
    const double expected_prob = 0.014204454615224965;
    const double tolerance = 1e-6;

    const double result_log = fims_math::ddiric_multinom<double>(x, p, theta, 1);
    const double result_prob = fims_math::ddiric_multinom<double>(x, p, theta, 0);

    EXPECT_NEAR(result_log, expected_log_prob, tolerance);
    EXPECT_NEAR(result_prob, expected_prob, tolerance);
}
