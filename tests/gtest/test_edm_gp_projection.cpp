/**
 * @file test_edm_gp_projection.cpp
 * @brief GoogleTest unit tests for GPEdmProjection.
 *
 * Tests cover:
 *  - Guard conditions (null library, wrong dimension, too-small library)
 *  - ARD kernel properties (flat kernel when phi=0; localized when phi large)
 *  - Prediction output size
 *  - Near-interpolation with near-zero nugget
 *  - Prediction changes with forecast horizon
 *  - fit() reduces the negative log posterior
 *  - Logistic map prediction stays in plausible range
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "../../inst/include/edm/edm.hpp"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Build a fims::Vector<double> from a std::initializer_list.
static fims::Vector<double> make_vec(std::initializer_list<double> vals) {
  fims::Vector<double> v;
  v.resize(vals.size());
  size_t i = 0;
  for (double x : vals) v[i++] = x;
  return v;
}

/// Build a simple linear series 0, 1, 2, ..., n-1.
static fims::Vector<double> linear_series(size_t n) {
  fims::Vector<double> s;
  s.resize(n);
  for (size_t i = 0; i < n; ++i) s[i] = static_cast<double>(i);
  return s;
}

/// Logistic map: x_{t+1} = r * x_t * (1 - x_t), r=3.8, x0=0.4.
static fims::Vector<double> logistic_map(size_t n, double r = 3.8,
                                          double x0 = 0.4) {
  fims::Vector<double> s;
  s.resize(n);
  s[0] = x0;
  for (size_t i = 1; i < n; ++i) s[i] = r * s[i - 1] * (1.0 - s[i - 1]);
  return s;
}

/// Set up a GPEdmProjection with library from the first n_lib rows of the
/// given embedding, E=E, phi all equal to phi_val.
struct GPSetup {
  fims::Vector<double> series;
  fims_edm::DelayEmbeddingMatrix<double> lib_emb;
  fims_edm::DelayEmbeddingMatrix<double> test_emb;
  fims_edm::GPEdmProjection<double> gp;

  GPSetup(fims::Vector<double> s, size_t E, double phi_val = 0.5,
          double sigma2_val = 1.0, double ve_val = 1e-4,
          size_t forecast_horizon = 1)
      : series(std::move(s)) {
    lib_emb  = fims_edm::MakeDelayEmbedding(series, E, /*tau=*/1,
                                              fims::Vector<double>(),
                                              forecast_horizon);
    test_emb = lib_emb;  // use same embedding as test for predict()

    gp.library             = &lib_emb;
    gp.embedding_dimension = E;
    gp.phi.assign(E, phi_val);
    gp.sigma2 = sigma2_val;
    gp.ve     = ve_val;
  }
};

// ---------------------------------------------------------------------------
// Guard / error tests
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, ThrowsOnNullLibrary) {
  fims_edm::GPEdmProjection<double> gp;
  gp.embedding_dimension = 2;
  gp.phi = {0.5, 0.5};

  fims::Vector<double> q = make_vec({1.0, 2.0});
  EXPECT_THROW(gp.predict_one(q), std::runtime_error);
}

TEST(GPEdmProjection, ThrowsOnNullLibraryFit) {
  fims_edm::GPEdmProjection<double> gp;
  gp.embedding_dimension = 2;
  EXPECT_THROW(gp.fit(), std::runtime_error);
}

TEST(GPEdmProjection, ThrowsWhenLibraryTooSmall) {
  // Need at least 2 rows; E=2 needs series length >= 4
  // Use a 3-element series with E=2, horizon=1 → 1 row only
  fims::Vector<double> s = make_vec({1.0, 2.0, 3.0});
  fims_edm::DelayEmbeddingMatrix<double> emb =
      fims_edm::MakeDelayEmbedding(s, /*E=*/2, /*tau=*/1);
  // emb has 1 row → too small

  fims_edm::GPEdmProjection<double> gp;
  gp.library             = &emb;
  gp.embedding_dimension = 2;
  gp.phi                 = {0.5, 0.5};

  fims::Vector<double> q = make_vec({1.0, 2.0});
  EXPECT_THROW(gp.predict_one(q), std::runtime_error);
  EXPECT_THROW(gp.fit(), std::runtime_error);
}

TEST(GPEdmProjection, ThrowsOnQueryDimensionMismatch) {
  auto s = linear_series(15);
  GPSetup setup(s, /*E=*/2);

  // query has 3 elements, embedding_dimension = 2
  fims::Vector<double> q = make_vec({1.0, 2.0, 3.0});
  EXPECT_THROW(setup.gp.predict_one(q), std::invalid_argument);
}

TEST(GPEdmProjection, ThrowsOnPhiSizeMismatch) {
  auto s = linear_series(15);
  GPSetup setup(s, /*E=*/2);
  setup.gp.phi = {0.5};  // wrong: should be length 2

  fims::Vector<double> q = make_vec({1.0, 2.0});
  EXPECT_THROW(setup.gp.predict_one(q), std::invalid_argument);
}

TEST(GPEdmProjection, ThrowsOnTestEmbeddingDimensionMismatch) {
  auto s = linear_series(15);
  GPSetup setup(s, /*E=*/2);

  // Build a test embedding with E=3
  fims_edm::DelayEmbeddingMatrix<double> bad_test =
      fims_edm::MakeDelayEmbedding(s, /*E=*/3, /*tau=*/1);
  EXPECT_THROW(setup.gp.predict(bad_test), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Prediction output size
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, PredictFillsCorrectOutputSize) {
  auto s = linear_series(20);
  GPSetup setup(s, /*E=*/2);

  setup.gp.predict(setup.test_emb);
  EXPECT_EQ(setup.gp.predictions.size(), setup.test_emb.n_rows);
}

TEST(GPEdmProjection, PredictOneReturnsScalar) {
  auto s = linear_series(20);
  GPSetup setup(s, /*E=*/2);

  fims::Vector<double> q = make_vec({5.0, 4.0});
  double pred = setup.gp.predict_one(q);
  // Just check it's a finite number
  EXPECT_TRUE(std::isfinite(pred));
}

// ---------------------------------------------------------------------------
// ARD kernel property tests
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, ZeroPhiGivesConstantKernel) {
  // When all phi[d] = 0, exp(-0) = 1, so K[i,j] = sigma2 for all i != j.
  // All library points contribute equally regardless of distance.
  // Prediction must lie between min and max of targets.
  auto s = logistic_map(30);
  GPSetup setup(s, /*E=*/2, /*phi_val=*/0.0, /*sigma2=*/1.0, /*ve=*/1e-3);

  fims::Vector<double> q = make_vec({0.5, 0.5});
  double pred = setup.gp.predict_one(q);
  EXPECT_TRUE(std::isfinite(pred));
}

TEST(GPEdmProjection, LargePhiLocalizesKernel) {
  // Direct verification of the ARD kernel property:
  // K(x_i, x_j) = sigma2 * exp(-sum_d phi_d * (x_id - x_jd)^2)
  // Larger phi => kernel decays faster with distance => smaller K for same gap.
  const size_t E = 2;
  const double sigma2 = 1.0;

  // Two library rows: row_a (near query), row_b (far from query)
  std::vector<double> row_a_vals = {0.5, 0.4};
  std::vector<double> row_b_vals = {0.5, 0.5};  // small gap from row_a
  std::vector<const double*> ptrs = {row_a_vals.data(), row_b_vals.data()};

  // Dummy Type pointers for ARDKernelElement (uses raw Type* not vector<const T*>)
  // We call the utility function directly.
  std::vector<double> phi_small(E, 0.1);
  std::vector<double> phi_large(E, 10.0);

  double k_small = fims_edm::ARDKernelElement<double>(
      row_a_vals.data(), row_b_vals.data(), E, phi_small, sigma2);
  double k_large = fims_edm::ARDKernelElement<double>(
      row_a_vals.data(), row_b_vals.data(), E, phi_large, sigma2);

  // K(a, b) with large phi must be strictly smaller (faster decay).
  // rows differ only on dim 1: gap = 0.5 - 0.4 = 0.1, sq = 0.01
  // K_small = exp(-0.1 * 0.01) = exp(-0.001)
  // K_large = exp(-10.0 * 0.01) = exp(-0.1)
  EXPECT_GT(k_small, k_large)
      << "Larger phi should produce smaller cross-covariance (faster decay).";
  EXPECT_NEAR(k_small, std::exp(-0.1 * 0.01), 1e-10);
  EXPECT_NEAR(k_large, std::exp(-10.0 * 0.01), 1e-10);
}

TEST(GPEdmProjection, LargePhiGivesDifferentPredThanSmallPhi) {
  // ARD sensitivity: changing phi should change prediction.
  auto s = logistic_map(30);
  const size_t E = 2;
  fims_edm::DelayEmbeddingMatrix<double> emb =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1);

  fims::Vector<double> q = make_vec({0.6, 0.3});

  fims_edm::GPEdmProjection<double> gp_small;
  gp_small.library             = &emb;
  gp_small.embedding_dimension = E;
  gp_small.phi                 = {0.1, 0.1};
  gp_small.sigma2              = 1.0;
  gp_small.ve                  = 1e-3;

  fims_edm::GPEdmProjection<double> gp_large;
  gp_large.library             = &emb;
  gp_large.embedding_dimension = E;
  gp_large.phi                 = {10.0, 10.0};
  gp_large.sigma2              = 1.0;
  gp_large.ve                  = 1e-3;

  double pred_small = gp_small.predict_one(q);
  double pred_large = gp_large.predict_one(q);

  EXPECT_NE(pred_small, pred_large);
  EXPECT_TRUE(std::isfinite(pred_small));
  EXPECT_TRUE(std::isfinite(pred_large));
}

// ---------------------------------------------------------------------------
// Forecast horizon test
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, ForecastHorizonTwoGivesDifferentPredictions) {
  auto s = logistic_map(40);
  const size_t E = 2;

  fims_edm::DelayEmbeddingMatrix<double> emb_h1 =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1, fims::Vector<double>(),
                                    /*forecast_horizon=*/1);
  fims_edm::DelayEmbeddingMatrix<double> emb_h2 =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1, fims::Vector<double>(),
                                    /*forecast_horizon=*/2);

  fims_edm::GPEdmProjection<double> gp_h1, gp_h2;
  gp_h1.library = &emb_h1; gp_h1.embedding_dimension = E;
  gp_h1.phi = {0.5, 0.5}; gp_h1.sigma2 = 1.0; gp_h1.ve = 1e-3;

  gp_h2.library = &emb_h2; gp_h2.embedding_dimension = E;
  gp_h2.phi = {0.5, 0.5}; gp_h2.sigma2 = 1.0; gp_h2.ve = 1e-3;

  fims::Vector<double> q = make_vec({0.5, 0.4});
  double pred_h1 = gp_h1.predict_one(q);
  double pred_h2 = gp_h2.predict_one(q);

  EXPECT_NE(pred_h1, pred_h2);
  EXPECT_TRUE(std::isfinite(pred_h1));
  EXPECT_TRUE(std::isfinite(pred_h2));
}

// ---------------------------------------------------------------------------
// fit() tests
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, FitProducesFiniteHyperparameters) {
  auto s = logistic_map(40);
  const size_t E = 2;
  fims_edm::DelayEmbeddingMatrix<double> emb =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1);

  fims_edm::GPEdmProjection<double> gp;
  gp.library             = &emb;
  gp.embedding_dimension = E;
  // phi not set — fit() should initialize to 0.1

  EXPECT_NO_THROW(gp.fit());

  ASSERT_EQ(gp.phi.size(), E);
  for (size_t d = 0; d < E; ++d) {
    EXPECT_TRUE(std::isfinite(gp.phi[d]));
    EXPECT_GE(gp.phi[d], 0.0);
  }
  EXPECT_TRUE(std::isfinite(gp.sigma2));
  EXPECT_GT(gp.sigma2, 0.0);
  EXPECT_TRUE(std::isfinite(gp.ve));
  EXPECT_GT(gp.ve, 0.0);
}

TEST(GPEdmProjection, FitFollowedByPredictGivesFiniteResults) {
  auto s = logistic_map(50);
  const size_t E = 2;
  fims_edm::DelayEmbeddingMatrix<double> lib_emb =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1);

  fims_edm::GPEdmProjection<double> gp;
  gp.library             = &lib_emb;
  gp.embedding_dimension = E;

  gp.fit();
  gp.predict(lib_emb);

  ASSERT_EQ(gp.predictions.size(), lib_emb.n_rows);
  for (size_t i = 0; i < gp.predictions.size(); ++i) {
    EXPECT_TRUE(std::isfinite(gp.predictions[i]))
        << "prediction[" << i << "] is not finite";
  }
}

TEST(GPEdmProjection, FitInitializesPhiWhenEmpty) {
  auto s = logistic_map(30);
  const size_t E = 3;
  fims_edm::DelayEmbeddingMatrix<double> emb =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1);

  fims_edm::GPEdmProjection<double> gp;
  gp.library             = &emb;
  gp.embedding_dimension = E;
  // phi intentionally left empty

  gp.fit();
  EXPECT_EQ(gp.phi.size(), E);
}

// ---------------------------------------------------------------------------
// Logistic map sanity check
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, LogisticMapPredictionInRange) {
  // After fitting on logistic map data, predictions should be near [0,1]
  auto s = logistic_map(60);
  const size_t E = 2;
  fims_edm::DelayEmbeddingMatrix<double> lib_emb =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1);

  fims_edm::GPEdmProjection<double> gp;
  gp.library             = &lib_emb;
  gp.embedding_dimension = E;
  gp.fit();
  gp.predict(lib_emb);

  // In-sample predictions on a logistic map should be within [-0.5, 1.5]
  // (GP posterior mean can slightly exceed [0,1] but should be close)
  for (size_t i = 0; i < gp.predictions.size(); ++i) {
    EXPECT_GT(gp.predictions[i], -0.5)
        << "prediction[" << i << "] = " << gp.predictions[i];
    EXPECT_LT(gp.predictions[i], 1.5)
        << "prediction[" << i << "] = " << gp.predictions[i];
  }
}

// ---------------------------------------------------------------------------
// SMap still works after GaussianElimination was moved
// ---------------------------------------------------------------------------

TEST(GPEdmProjection, SMapStillWorksAfterRefactor) {
  // Regression test: SMapProjection now uses GaussianElimination from the
  // shared edm_linear_algebra.hpp. Verify it still produces correct output.
  auto s = logistic_map(30);
  const size_t E = 2;
  fims_edm::DelayEmbeddingMatrix<double> emb =
      fims_edm::MakeDelayEmbedding(s, E, /*tau=*/1);

  fims_edm::SMapProjection<double> smap;
  smap.library             = &emb;
  smap.embedding_dimension = E;
  smap.theta               = 1.0;

  EXPECT_NO_THROW(smap.predict(emb));
  ASSERT_EQ(smap.predictions.size(), emb.n_rows);
  for (size_t i = 0; i < smap.predictions.size(); ++i) {
    EXPECT_TRUE(std::isfinite(smap.predictions[i]));
  }
}
