#include "edm/functors/delay_embedding.hpp"
#include "gtest/gtest.h"

// ### Layout note (h=1 default)
// With forecast_horizon=1 (default):
//   n_rows          = series.size() - lag_span - 1
//   embedded[row]   = [series[t], series[t-tau], ...] where t = row + lag_span
//   target[row]     = series[t + 1]  (one step ahead)

namespace {

TEST(DelayEmbedding, BuildsEmbeddingWithUnitLag) {
  // series={1,2,3,4,5}, E=3, tau=1, h=1
  // lag_span = 2, n_rows = 5 - 2 - 1 = 2
  // row 0: t=2 → embedded=[3,2,1], target=series[3]=4
  // row 1: t=3 → embedded=[4,3,2], target=series[4]=5
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  EXPECT_EQ(embedding.n_rows, 2);
  EXPECT_EQ(embedding.n_cols, 3);

  // target_values[row] = x_{t+1}
  EXPECT_DOUBLE_EQ(*embedding.target_values[0], 4.0);  // row 0 target = x_3
  EXPECT_DOUBLE_EQ(*embedding.target_values[1], 5.0);  // row 1 target = x_4

  // embedded_values: [x_t, x_{t-1}, x_{t-2}]
  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 3.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 2), 1.0);
  EXPECT_DOUBLE_EQ(embedding.at(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(embedding.at(1, 1), 3.0);
  EXPECT_DOUBLE_EQ(embedding.at(1, 2), 2.0);
}

TEST(DelayEmbedding, BuildsEmbeddingWithLargerLag) {
  // series={10,20,30,40,50,60}, E=2, tau=2, h=1
  // lag_span = 2, n_rows = 6 - 2 - 1 = 3
  // row 0: t=2 → embedded=[30,10], target=series[3]=40
  // row 2: t=4 → embedded=[50,30], target=series[5]=60
  fims::Vector<double> series = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 2, 2);

  EXPECT_EQ(embedding.n_rows, 3);
  EXPECT_EQ(embedding.n_cols, 2);

  // Row 0: t=2, target = x_3 = 40
  EXPECT_DOUBLE_EQ(*embedding.target_values[0], 40.0);

  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 30.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 1), 10.0);
  EXPECT_DOUBLE_EQ(embedding.at(2, 0), 50.0);
  EXPECT_DOUBLE_EQ(embedding.at(2, 1), 30.0);
}

TEST(DelayEmbedding, SupportsEmbeddingDimensionOne) {
  // series={4,5,6}, E=1, tau=1, h=1
  // lag_span=0, n_rows = 3 - 0 - 1 = 2
  // row 0: t=0, embedded=[4], target=5
  // row 1: t=1, embedded=[5], target=6
  fims::Vector<double> series = {4.0, 5.0, 6.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 1, 1);

  EXPECT_EQ(embedding.n_rows, 2);
  EXPECT_EQ(embedding.n_cols, 1);
  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 4.0);
  EXPECT_DOUBLE_EQ(embedding.at(1, 0), 5.0);
  EXPECT_DOUBLE_EQ(*embedding.target_values[0], 5.0);
  EXPECT_DOUBLE_EQ(*embedding.target_values[1], 6.0);
}

TEST(DelayEmbedding, RejectsInvalidInputs) {
  fims::Vector<double> series = {1.0, 2.0, 3.0};

  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 0, 1),
               std::invalid_argument);
  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 2, 0),
               std::invalid_argument);
  // E=4, tau=1: lag_span=3, need size >= 3+1+1=5, only have 3
  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 4, 1),
               std::invalid_argument);
  // E=2, tau=3: lag_span=3, need size >= 3+1+1=5, only have 3
  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 2, 3),
               std::invalid_argument);
}

TEST(DelayEmbedding, DropsRowsWithMissingValues) {
  // series={1,-999,3,4,5}, E=2, tau=1, h=1
  // Full embedding (before drop): n_rows = 5-1-1 = 3
  // row 0: t=1, embedded=[-999,1], target=3  → has missing → drop
  // row 1: t=2, embedded=[3,-999], target=4  → has missing → drop
  // row 2: t=3, embedded=[4,3],    target=5  → valid
  // After drop: 1 valid row
  const double missing_value = -999.0;
  fims::Vector<double> series = {1.0, missing_value, 3.0, 4.0, 5.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbeddingDropMissing(series, 2, 1, missing_value);

  EXPECT_EQ(embedding.n_rows, 1);
  EXPECT_EQ(embedding.n_cols, 2);

  // Retained row: embedded=[4,3], target=5
  EXPECT_DOUBLE_EQ(*embedding.target_values[0], 5.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 4.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 1), 3.0);
}

TEST(DelayEmbedding, ThrowsOnOutOfBoundsMatrixAccess) {
  fims::Vector<double> series = {1.0, 2.0, 3.0};
  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 2, 1);

  EXPECT_THROW(embedding.at(embedding.n_rows, 0), std::invalid_argument);
  EXPECT_THROW(embedding.at(0, embedding.n_cols), std::invalid_argument);
}

TEST(DelayEmbedding, DynamicallyReflectsOriginalSeriesModifications) {
  // series={1,2,3,4,5}, E=3, tau=1, h=1 → n_rows=2
  // row 0: t=2, embedded=[3,2,1], target=4
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 3.0);
  EXPECT_DOUBLE_EQ(*embedding.target_values[0], 4.0);

  // Modify the original series → pointers reflect change immediately
  series[3] = 99.0;  // target of row 0

  EXPECT_DOUBLE_EQ(*embedding.target_values[0], 99.0);
}

TEST(DelayEmbedding, PropagatesUncertaintyVectors) {
  // series={1,2,3,4,5}, E=3, tau=1, h=1 → n_rows=2
  // uncertainty={0.1,0.2,0.3,0.4,0.5}
  // row 0: t=2, embedded uncertainty=[0.3,0.2,0.1], target uncertainty=0.4
  // row 1: t=3, embedded uncertainty=[0.4,0.3,0.2], target uncertainty=0.5
  fims::Vector<double> series      = {1.0, 2.0, 3.0, 4.0, 5.0};
  fims::Vector<double> uncertainty = {0.1, 0.2, 0.3, 0.4, 0.5};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 3, 1, uncertainty);

  EXPECT_EQ(embedding.n_rows, 2);
  EXPECT_EQ(embedding.n_cols, 3);

  // target_uncertainty[row] = sigma_{t+1}
  EXPECT_DOUBLE_EQ(*embedding.target_uncertainty[0], 0.4);  // sigma_3
  EXPECT_DOUBLE_EQ(*embedding.target_uncertainty[1], 0.5);  // sigma_4

  // embedded_uncertainty layout mirrors embedded_values
  EXPECT_DOUBLE_EQ(*embedding.embedded_uncertainty[0 * 3 + 0], 0.3);
  EXPECT_DOUBLE_EQ(*embedding.embedded_uncertainty[0 * 3 + 1], 0.2);
  EXPECT_DOUBLE_EQ(*embedding.embedded_uncertainty[0 * 3 + 2], 0.1);
  EXPECT_DOUBLE_EQ(*embedding.embedded_uncertainty[1 * 3 + 0], 0.4);
}

TEST(DelayEmbedding, UncertaintyFieldsEmptyWhenNotProvided) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  EXPECT_TRUE(embedding.embedded_uncertainty.empty());
  EXPECT_TRUE(embedding.target_uncertainty.empty());
}

TEST(DelayEmbedding, UncertaintyDynamicallyReflectsSeriesModifications) {
  // row 0: t=2, target uncertainty = sigma_{3} = 0.4
  fims::Vector<double> series      = {1.0, 2.0, 3.0, 4.0, 5.0};
  fims::Vector<double> uncertainty = {0.1, 0.2, 0.3, 0.4, 0.5};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 3, 1, uncertainty);

  EXPECT_DOUBLE_EQ(*embedding.target_uncertainty[0], 0.4);

  // Modify the original uncertainty series element
  uncertainty[3] = 9.9;

  EXPECT_DOUBLE_EQ(*embedding.target_uncertainty[0], 9.9);
}

TEST(DelayEmbedding, DropMissingPropagatesUncertainty) {
  // series={1,-999,3,4,5}, E=2, tau=1, h=1
  // Only row 2 (t=3, embedded=[4,3], target=5) is valid after drop
  const double missing_value = -999.0;
  fims::Vector<double> series      = {1.0, missing_value, 3.0, 4.0, 5.0};
  fims::Vector<double> uncertainty = {0.1, 0.2,           0.3, 0.4, 0.5};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbeddingDropMissing(series, 2, 1, missing_value,
                                              uncertainty);

  EXPECT_EQ(embedding.n_rows, 1);

  // target_uncertainty = sigma_{t+1} = sigma_4 = 0.5
  EXPECT_DOUBLE_EQ(*embedding.target_uncertainty[0], 0.5);

  // embedded_uncertainty at row 0: [sigma_3, sigma_2] = [0.4, 0.3]
  EXPECT_DOUBLE_EQ(*embedding.embedded_uncertainty[0 * 2 + 0], 0.4);
  EXPECT_DOUBLE_EQ(*embedding.embedded_uncertainty[0 * 2 + 1], 0.3);
}

}  // namespace
