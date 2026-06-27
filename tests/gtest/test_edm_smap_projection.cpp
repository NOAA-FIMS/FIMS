// Instructions ----
// This file follows the format used for EDM gtest files in FIMS.
// Tests cover: IO correctness, edge handling, and error handling.
//
// ### Data-layout note (post mentor feedback, 2024-06)
// MakeDelayEmbedding now takes a forecast_horizon parameter (default = 1).
// With the default:
//
//   embedded_values[row][col] = &series[target_index - col * tau]
//   target_values[row]        = &series[target_index + 1]   (x_{t+1})
//
// This matches the standard EDM one-step-ahead formulation (Sugihara 1994;
// Esguerra & Munch 2024, Section 2.1): predict x_{t+1} from x_t.
//
// Consequence: the design matrix and target are now DIFFERENT.  For
// arithmetic series {a, a+k, a+2k, ...} the OLS still fits a perfect line,
// but the slope now captures the actual +k step, not a degenerate self-fit.
// Arithmetic series still produce a singular design matrix for E>=2 because
// col-1 = col-0 - k (linear combo of intercept and col-0).  All tests with
// E>=2 therefore use quadratic or logistic-map series.
//
// ### Kernel note (post mentor feedback, 2024-06)
// SMapProjection now exposes a `kernel` field:
//   SMapKernel::kExponential (default): w = exp(-theta * d / d_mean)  [rEDM]
//   SMapKernel::kGaussian:              w = exp(-theta^2 * (d/D)^2)   [E&M 2024]

#include "edm/edm.hpp"
#include "gtest/gtest.h"

using namespace fims_edm;

namespace {

// ---------------------------------------------------------------------------
// Helper: build a DelayEmbeddingMatrix<double> from a flat time series.
// Uses forecast_horizon = 1 (the new default) throughout.
// ---------------------------------------------------------------------------
DelayEmbeddingMatrix<double> MakeLib(const fims::Vector<double>& series,
                                     size_t E, size_t tau = 1,
                                     size_t h = 1) {
  return MakeDelayEmbedding(series, E, tau, fims::Vector<double>(), h);
}

// ---------------------------------------------------------------------------
// Helper: generate the logistic map  x_{t+1} = r * x_t * (1 - x_t)
// ---------------------------------------------------------------------------
fims::Vector<double> LogisticMap(double x0, double r, size_t n) {
  fims::Vector<double> s(n);
  s[0] = x0;
  for (size_t i = 1; i < n; ++i) {
    s[i] = r * s[i - 1] * (1.0 - s[i - 1]);
  }
  return s;
}

// ===========================================================================
// Test 1: theta=0, E=1, arithmetic series → target = x_{t+1} = x_t + 1.
//
//   Series: {1, 2, 3, ..., 10}.
//   With h=1 and E=1: embedding = [x_t], target = x_{t+1} = x_t + 1.
//   OLS fit: y = 1 + 1*x_t.  Query q={7} → prediction ≈ 8.
// ===========================================================================
TEST(SMapProjection, ThetaZeroE1LinearSeriesOneStepAhead) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0,
                                  6.0, 7.0, 8.0, 9.0, 10.0};
  auto lib = MakeLib(series, 1);  // h=1 default

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 1;
  smap.theta = 0.0;

  // Global OLS: y = 1 + x_t → prediction for x_t=7 is 8
  fims::Vector<double> q = {7.0};
  double pred = smap.predict_one(q);
  EXPECT_NEAR(pred, 8.0, 0.5);
}

// ===========================================================================
// Test 2: theta=0, quadratic series with E=2.
//
//   Series: {1, 4, 9, 16, 25, 36, 49, 64, 81}.
//   With h=1, E=2: embedding = [x_t, x_{t-1}], target = x_{t+1}.
//   The step pattern is non-constant so the design matrix is full-rank.
//   For theta=0 (global OLS) the prediction should be in a plausible range.
// ===========================================================================
TEST(SMapProjection, ThetaZeroQuadraticE2FullRankDesignMatrix) {
  fims::Vector<double> series = {1.0, 4.0, 9.0, 16.0, 25.0,
                                  36.0, 49.0, 64.0, 81.0, 100.0};
  auto lib = MakeLib(series, 2);

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 2;
  smap.theta = 0.0;

  // Query [64, 49]: the global OLS should predict a value close to 81
  fims::Vector<double> q = {64.0, 49.0};
  double pred = smap.predict_one(q);
  EXPECT_NEAR(pred, 81.0, 5.0);
}

// ===========================================================================
// Test 3: theta=0 vs theta>0 give different predictions on nonlinear data.
//
//   Use a logistic-map library and query with a point BETWEEN library rows
//   (not an exact member) so the distance weights actually matter.
// ===========================================================================
TEST(SMapProjection, ThetaZeroAndPositiveDifferOnNonlinearData) {
  auto series = LogisticMap(0.4, 3.9, 50);
  auto lib = MakeLib(series, 3);

  SMapProjection<double> smap_global;
  smap_global.library = &lib;
  smap_global.embedding_dimension = 3;
  smap_global.theta = 0.0;

  SMapProjection<double> smap_local;
  smap_local.library = &lib;
  smap_local.embedding_dimension = 3;
  smap_local.theta = 8.0;  // strong localization

  // Midpoint between library rows 10 and 11 → not an exact library member
  fims::Vector<double> q(3);
  for (size_t j = 0; j < 3; ++j) {
    q[j] = 0.5 * (lib.at(10, j) + lib.at(11, j));
  }

  double pred_global = smap_global.predict_one(q);
  double pred_local = smap_local.predict_one(q);

  // Global (theta=0) and strongly local (theta=8) should differ on chaotic data
  EXPECT_NE(pred_global, pred_local);
}

// ===========================================================================
// Test 4: Gaussian kernel vs exponential kernel give different predictions.
//   Validates that the kernel field is wired through and changes the result.
// ===========================================================================
TEST(SMapProjection, GaussianAndExponentialKernelsDiffer) {
  auto series = LogisticMap(0.4, 3.9, 50);
  auto lib = MakeLib(series, 3);

  SMapProjection<double> smap_exp;
  smap_exp.library = &lib;
  smap_exp.embedding_dimension = 3;
  smap_exp.theta = 3.0;
  smap_exp.kernel = SMapKernel::kExponential;

  SMapProjection<double> smap_gauss;
  smap_gauss.library = &lib;
  smap_gauss.embedding_dimension = 3;
  smap_gauss.theta = 3.0;
  smap_gauss.kernel = SMapKernel::kGaussian;

  // Midpoint query — not a library row
  fims::Vector<double> q(3);
  for (size_t j = 0; j < 3; ++j) {
    q[j] = 0.5 * (lib.at(5, j) + lib.at(6, j));
  }

  double pred_exp   = smap_exp.predict_one(q);
  double pred_gauss = smap_gauss.predict_one(q);

  // The two kernels apply different decay functions → results should differ
  EXPECT_NE(pred_exp, pred_gauss);
}

// ===========================================================================
// Test 5: predict() fills the predictions vector with the correct size.
// ===========================================================================
TEST(SMapProjection, PredictFillsOutputVectorCorrectSize) {
  auto series = LogisticMap(0.3, 3.7, 30);
  auto lib = MakeLib(series, 2);
  auto test = MakeLib(series, 2);

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 2;
  smap.theta = 1.0;

  smap.predict(test);
  EXPECT_EQ(smap.predictions.size(), test.n_rows);
}

// ===========================================================================
// Test 6: Throws std::runtime_error when library is null.
// ===========================================================================
TEST(SMapProjection, ThrowsOnNullLibrary) {
  SMapProjection<double> smap;
  smap.embedding_dimension = 2;
  smap.theta = 1.0;

  fims::Vector<double> q = {1.0, 2.0};
  EXPECT_THROW(smap.predict_one(q), std::runtime_error);
}

// ===========================================================================
// Test 7: Throws std::invalid_argument when query dimension mismatches.
// ===========================================================================
TEST(SMapProjection, ThrowsOnQueryDimensionMismatch) {
  auto series = LogisticMap(0.4, 3.9, 20);
  auto lib = MakeLib(series, 2);

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 2;
  smap.theta = 1.0;

  fims::Vector<double> q = {1.0};  // dimension 1, needs 2
  EXPECT_THROW(smap.predict_one(q), std::invalid_argument);
}

// ===========================================================================
// Test 8: Throws when library has fewer than E+1 rows.
// ===========================================================================
TEST(SMapProjection, ThrowsWhenLibraryTooSmall) {
  // E=3, h=1 → need series.size() >= lag_span + h + 1 = 2 + 1 + 1 = 4 rows.
  // Series of length 5 gives n_rows = 5 - 2 - 1 = 2 < E+1 = 4.
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0};
  auto lib = MakeLib(series, 3);

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 3;
  smap.theta = 1.0;

  fims::Vector<double> q = {5.0, 4.0, 3.0};
  EXPECT_THROW(smap.predict_one(q), std::runtime_error);
}

// ===========================================================================
// Test 9: predict() throws when test embedding dimension mismatches.
// ===========================================================================
TEST(SMapProjection, PredictThrowsOnTestDimensionMismatch) {
  auto series = LogisticMap(0.4, 3.9, 20);
  auto lib = MakeLib(series, 2);
  auto test = MakeLib(series, 3);  // wrong E

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 2;
  smap.theta = 1.0;

  EXPECT_THROW(smap.predict(test), std::invalid_argument);
}

// ===========================================================================
// Test 10: Negative theta throws std::invalid_argument (from SMapWeights).
// ===========================================================================
TEST(SMapProjection, ThrowsOnNegativeTheta) {
  auto series = LogisticMap(0.4, 3.9, 20);
  auto lib = MakeLib(series, 2);

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 2;
  smap.theta = -1.0;

  fims::Vector<double> q = {0.5, 0.4};
  EXPECT_THROW(smap.predict_one(q), std::invalid_argument);
}

// ===========================================================================
// Test 11: Logistic map — predictions stay within a plausible range.
//          Logistic map lives in (0,1); predictions should be close.
// ===========================================================================
TEST(SMapProjection, LogisticMapPredictionInRange) {
  auto series = LogisticMap(0.2, 3.9, 35);
  auto lib = MakeLib(series, 3);

  SMapProjection<double> smap;
  smap.library = &lib;
  smap.embedding_dimension = 3;
  smap.theta = 2.0;

  size_t last = lib.n_rows - 1;
  fims::Vector<double> q(3);
  for (size_t j = 0; j < 3; ++j) q[j] = lib.at(last, j);

  double pred = smap.predict_one(q);
  EXPECT_GT(pred, -0.5);
  EXPECT_LT(pred, 1.5);
}

// ===========================================================================
// Test 12: forecast_horizon=2 gives a different target than h=1.
//   Validates that the forecast_horizon parameter propagates correctly.
// ===========================================================================
TEST(SMapProjection, ForecastHorizonTwoGivesDifferentTargetThanOne) {
  auto series = LogisticMap(0.4, 3.9, 30);
  auto lib_h1 = MakeLib(series, 2, 1, 1);  // h=1 (default)
  auto lib_h2 = MakeLib(series, 2, 1, 2);  // h=2

  // Sanity: targets for the same embedded row differ between h=1 and h=2
  // target_h1[row] = series[target_index + 1]
  // target_h2[row] = series[target_index + 2]
  EXPECT_NE(*lib_h1.target_values[0], *lib_h2.target_values[0]);
}

}  // namespace
