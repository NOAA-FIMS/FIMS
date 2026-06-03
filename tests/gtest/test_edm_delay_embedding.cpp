#include "edm/functors/delay_embedding.hpp"
#include "gtest/gtest.h"

namespace {

TEST(DelayEmbedding, BuildsEmbeddingWithUnitLag) {
  fims::Vector<double> series = {1.0, 2.0, 3.0, 4.0, 5.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 3, 1);

  EXPECT_EQ(embedding.n_rows, 3);
  EXPECT_EQ(embedding.n_cols, 3);
  EXPECT_EQ(embedding.target_indices[0], 2);
  EXPECT_EQ(embedding.target_indices[2], 4);

  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 3.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 2), 1.0);
  EXPECT_DOUBLE_EQ(embedding.at(2, 0), 5.0);
  EXPECT_DOUBLE_EQ(embedding.at(2, 1), 4.0);
  EXPECT_DOUBLE_EQ(embedding.at(2, 2), 3.0);
}

TEST(DelayEmbedding, BuildsEmbeddingWithLargerLag) {
  fims::Vector<double> series = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 2, 2);

  EXPECT_EQ(embedding.n_rows, 4);
  EXPECT_EQ(embedding.n_cols, 2);
  EXPECT_EQ(embedding.target_indices[0], 2);

  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 30.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 1), 10.0);
  EXPECT_DOUBLE_EQ(embedding.at(3, 0), 60.0);
  EXPECT_DOUBLE_EQ(embedding.at(3, 1), 40.0);
}

TEST(DelayEmbedding, SupportsEmbeddingDimensionOne) {
  fims::Vector<double> series = {4.0, 5.0, 6.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 1, 1);

  EXPECT_EQ(embedding.n_rows, 3);
  EXPECT_EQ(embedding.n_cols, 1);
  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 4.0);
  EXPECT_DOUBLE_EQ(embedding.at(2, 0), 6.0);
}

TEST(DelayEmbedding, RejectsInvalidInputs) {
  fims::Vector<double> series = {1.0, 2.0, 3.0};

  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 0, 1),
               std::invalid_argument);
  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 2, 0),
               std::invalid_argument);
  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 4, 1),
               std::invalid_argument);
  EXPECT_THROW(fims_edm::MakeDelayEmbedding(series, 2, 3),
               std::invalid_argument);
}

TEST(DelayEmbedding, DropsRowsWithMissingValues) {
  const double missing_value = -999.0;
  fims::Vector<double> series = {1.0, 2.0, missing_value, 4.0, 5.0};

  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbeddingDropMissing(series, 2, 1, missing_value);

  EXPECT_EQ(embedding.n_rows, 2);
  EXPECT_EQ(embedding.n_cols, 2);
  EXPECT_EQ(embedding.target_indices[0], 1);
  EXPECT_EQ(embedding.target_indices[1], 4);

  EXPECT_DOUBLE_EQ(embedding.at(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(embedding.at(0, 1), 1.0);
  EXPECT_DOUBLE_EQ(embedding.at(1, 0), 5.0);
  EXPECT_DOUBLE_EQ(embedding.at(1, 1), 4.0);
}

TEST(DelayEmbedding, ThrowsOnOutOfBoundsMatrixAccess) {
  fims::Vector<double> series = {1.0, 2.0, 3.0};
  fims_edm::DelayEmbeddingMatrix<double> embedding =
      fims_edm::MakeDelayEmbedding(series, 2, 1);

  EXPECT_THROW(embedding.at(embedding.n_rows, 0), std::invalid_argument);
  EXPECT_THROW(embedding.at(0, embedding.n_cols), std::invalid_argument);
}

}  // namespace
