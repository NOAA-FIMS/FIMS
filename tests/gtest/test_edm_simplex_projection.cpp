#include "edm/edm.hpp"
#include "gtest/gtest.h"

namespace {

// ---------------------------------------------------------------------------
// Test 1: Default n_neighbors is 0 (predicts using E+1 internally)
// ---------------------------------------------------------------------------
TEST(SimplexProjection, DefaultNeighborCountIsZero) {
  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 3;
  sp.time_lag = 1;

  // n_neighbors == 0 is the sentinel meaning "use E+1 internally".
  EXPECT_EQ(sp.n_neighbors, 0u);

  // A library with 5 rows (≥ E+1 = 4) must succeed with n_neighbors = 0.
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  sp.library = &lib;
  fims::Vector<double> q = {5.0, 4.0, 3.0};
  EXPECT_NO_THROW(sp.predict_one(q));
}

// ---------------------------------------------------------------------------
// Test 2: Exact-match point dominates exponential weighting
//
// For the linear series x_t = t (E=1, tau=1, k=2, h=1):
//  - Library rows: embedded={t}, target=x_{t+1}=t+1
//    series={1,2,3,4,5,6} → rows: [1]→2, [2]→3, [3]→4, [4]→5
//  - Query = {4.0} → exact match to embedded=4, target=5
//  - d_min = 0 → exponent for exact match = 0 → w=1
//  - exponent for second neighbor = large → w≈0
//  - Prediction ≈ target of exact match = 5.0
// ---------------------------------------------------------------------------
TEST(SimplexProjection, ExactMatchDominatesWeighting) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 1, 1);  // h=1 default

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 1;
  sp.time_lag = 1;
  sp.n_neighbors = 2;
  sp.library = &lib;

  // Query matches embedded=4 exactly. Its target is 5 (one step ahead).
  fims::Vector<double> q = {4.0};
  double pred = sp.predict_one(q);

  // The exact-match row dominates → prediction ≈ 5.0
  EXPECT_NEAR(pred, 5.0, 1e-6);
}

// ---------------------------------------------------------------------------
// Test 3: Uniform weights when all neighbors are equidistant
//
// Manually construct a library with 4 rows all at squared distance 1.0
// from the query point (1.0, 0.0):
//   Row 0: embedded=(2.0, 0.0), target=10.0  → d²=1
//   Row 1: embedded=(0.0, 0.0), target=20.0  → d²=1
//   Row 2: embedded=(1.0, 1.0), target=30.0  → d²=1
//   Row 3: embedded=(1.0,-1.0), target=40.0  → d²=1
// All exponents = 1/(1+eps) ≈ 1 → equal weights → mean = 25.0.
// ---------------------------------------------------------------------------
TEST(SimplexProjection, AllEqualDistancesGiveUniformWeights) {
  fims_edm::DelayEmbeddingMatrix<double> lib;
  lib.n_rows = 4;
  lib.n_cols = 2;

  // Static storage so pointers remain valid for the test lifetime.
  static double vals[8] = {2.0, 0.0,  // row 0
                            0.0, 0.0,  // row 1
                            1.0, 1.0,  // row 2
                            1.0, -1.0}; // row 3
  static double tgts[4] = {10.0, 20.0, 30.0, 40.0};

  lib.embedded_values.resize(8);
  lib.target_values.resize(4);
  for (size_t i = 0; i < 8; ++i) lib.embedded_values[i] = &vals[i];
  for (size_t i = 0; i < 4; ++i) lib.target_values[i] = &tgts[i];

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 2;
  sp.time_lag = 1;
  sp.n_neighbors = 4;
  sp.library = &lib;

  fims::Vector<double> q = {1.0, 0.0};
  double pred = sp.predict_one(q);

  // All weights identical → prediction = arithmetic mean of targets.
  EXPECT_NEAR(pred, 25.0, 1e-6);
}

// ---------------------------------------------------------------------------
// Test 4: predict() fills predictions with n_test elements
// ---------------------------------------------------------------------------
TEST(SimplexProjection, PredictFillsOutputVector) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0,
                                  6.0, 7.0, 8.0, 9.0, 10.0};

  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 2, 1);  // h=1: n_rows = 10-1-1 = 8
  ASSERT_EQ(lib.n_rows, 8u);

  fims_edm::DelayEmbeddingMatrix<double> test_emb =
      fims_edm::MakeDelayEmbedding(series, 2, 1);

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 2;
  sp.time_lag = 1;
  sp.n_neighbors = 3;
  sp.library = &lib;

  sp.predict(test_emb);

  EXPECT_EQ(sp.predictions.size(), test_emb.n_rows);
}

// ---------------------------------------------------------------------------
// Test 5: Throws when library pointer is null
// ---------------------------------------------------------------------------
TEST(SimplexProjection, ThrowsOnNullLibrary) {
  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 2;
  sp.library = nullptr;

  fims::Vector<double> q = {1.0, 2.0};
  EXPECT_THROW(sp.predict_one(q), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Test 6: Throws when query dimension mismatches embedding_dimension
// ---------------------------------------------------------------------------
TEST(SimplexProjection, ThrowsOnQueryDimensionMismatch) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0};
  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 2, 1);

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 2;
  sp.library = &lib;

  // Query has 3 elements but embedding_dimension is 2.
  fims::Vector<double> q_wrong = {1.0, 2.0, 3.0};
  EXPECT_THROW(sp.predict_one(q_wrong), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Test 7: Throws when library has fewer rows than requested k
// ---------------------------------------------------------------------------
TEST(SimplexProjection, ThrowsWhenLibraryTooSmall) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0};
  // E=2, tau=1, h=1 → n_rows = 4 - 1 - 1 = 2.
  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 2, 1);
  ASSERT_EQ(lib.n_rows, 2u);

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 2;
  sp.library = &lib;
  sp.n_neighbors = 10;  // More neighbors than library rows.

  fims::Vector<double> q = {2.0, 1.0};
  EXPECT_THROW(sp.predict_one(q), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Test 8: Logistic-map prediction stays within [0, 1]
//
// The logistic map x_{t+1} = r * x_t * (1 - x_t) with r=3.8 is bounded
// in [0, 1]. A reasonable Simplex prediction on the embedded series should
// also fall within that range.
// ---------------------------------------------------------------------------
TEST(SimplexProjection, LogisticMapPredictionInRange) {
  const size_t N = 50;
  const double r = 3.8;
  fims::Vector<double> series;
  series.emplace_back(0.4);
  for (size_t i = 1; i < N; ++i) {
    double prev = series[i - 1];
    series.emplace_back(r * prev * (1.0 - prev));
  }

  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 3;
  sp.time_lag = 1;
  // n_neighbors = 0 → uses E+1 = 4 internally.
  sp.library = &lib;

  // Query: the last three values of the series.
  fims::Vector<double> q = {series[N - 1], series[N - 2], series[N - 3]};
  double pred = sp.predict_one(q);

  // Logistic map is bounded in (0, 1).
  EXPECT_GE(pred, 0.0);
  EXPECT_LE(pred, 1.0);
}

// ---------------------------------------------------------------------------
// Test 9: predict() throws if test embedding has wrong dimension
// ---------------------------------------------------------------------------
TEST(SimplexProjection, PredictThrowsOnTestDimensionMismatch) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  fims_edm::DelayEmbeddingMatrix<double> lib =
      fims_edm::MakeDelayEmbedding(series, 2, 1);

  // Test embedding uses E=3, but predictor expects E=2.
  fims_edm::DelayEmbeddingMatrix<double> test_emb =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  fims_edm::SimplexProjection<double> sp;
  sp.embedding_dimension = 2;
  sp.library = &lib;

  EXPECT_THROW(sp.predict(test_emb), std::invalid_argument);
}

}  // namespace
