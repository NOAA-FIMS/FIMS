/**
 * @file delay_embedding.hpp
 * @brief Utilities for constructing EDM delay embeddings.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_DELAY_EMBEDDING_HPP
#define FIMS_EDM_DELAY_EMBEDDING_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../../common/fims_vector.hpp"

namespace fims_edm {

/**
 * @brief Flattened delay embedding matrix.
 *
 * @details Stores pointers into the original input_values series rather than
 * copying values. This means:
 *  - embedded_values: pointers to the E lagged coordinates per row
 *    [x_t, x_{t-tau}, ..., x_{t-(E-1)tau}]
 *  - target_values: pointers to the target time-point value x_t for each row
 *
 * Both point into the same underlying input_values vector, so any update to
 * that vector (e.g., during TMB optimization iterations) is automatically
 * reflected here without copying.
 */
template <typename Type>
struct DelayEmbeddingMatrix {
  /** @brief Number of rows in the embedding matrix. */
  size_t n_rows = 0;
  /** @brief Number of columns (equal to embedding dimension E). */
  size_t n_cols = 0;
  /**
   * @brief Flattened row-major pointers to the embedded lagged coordinates.
   * element [row * n_cols + col] points to x_{t - col * tau}.
   */
  std::vector<const Type*> embedded_values;
  /**
   * @brief Pointers to the target time-point value for each row.
   * target_values[row] == embedded_values[row * n_cols + 0] (i.e., x_t).
   */
  std::vector<const Type*> target_values;

  /**
   * @brief Access an element of the embedding matrix.
   * @param row Zero-based row index.
   * @param col Zero-based column index.
   * @return Const reference to the value at (row, col).
   */
  const Type& at(size_t row, size_t col) const {
    if (row >= n_rows || col >= n_cols) {
      throw std::invalid_argument("DelayEmbeddingMatrix index out of bounds.");
    }
    return *embedded_values[row * n_cols + col];
  }
};

/**
 * @brief Build a delay embedding matrix from a univariate time series.
 *
 * Rows are ordered by increasing target time. Columns are ordered as
 * x_t, x_{t - tau}, ..., x_{t - (E - 1) tau}.
 *
 * Both embedded_values and target_values store pointers into @p series so
 * that subsequent modifications to @p series are automatically reflected.
 */
template <typename Type>
DelayEmbeddingMatrix<Type> MakeDelayEmbedding(const fims::Vector<Type>& series,
                                              size_t embedding_dimension,
                                              size_t time_lag) {
  if (embedding_dimension == 0) {
    throw std::invalid_argument("embedding_dimension must be greater than 0.");
  }
  if (time_lag == 0) {
    throw std::invalid_argument("time_lag must be greater than 0.");
  }

  const size_t lag_span = (embedding_dimension - 1) * time_lag;
  if (series.size() <= lag_span) {
    throw std::invalid_argument(
        "series is too short for the requested delay embedding.");
  }

  DelayEmbeddingMatrix<Type> embedding;
  embedding.n_rows = series.size() - lag_span;
  embedding.n_cols = embedding_dimension;
  embedding.embedded_values.resize(embedding.n_rows * embedding.n_cols);
  embedding.target_values.resize(embedding.n_rows);

  for (size_t row = 0; row < embedding.n_rows; row++) {
    const size_t target_index = row + lag_span;
    // target_values[row] points to the x_t value for this row
    embedding.target_values[row] = &series[target_index];
    for (size_t col = 0; col < embedding.n_cols; col++) {
      embedding.embedded_values[row * embedding.n_cols + col] =
          &series[target_index - col * time_lag];
    }
  }

  return embedding;
}

/**
 * @brief Build a delay embedding matrix and omit rows containing missing
 * values.
 */
template <typename Type>
DelayEmbeddingMatrix<Type> MakeDelayEmbeddingDropMissing(
    const fims::Vector<Type>& series, size_t embedding_dimension,
    size_t time_lag, const Type& missing_value) {
  DelayEmbeddingMatrix<Type> full_embedding =
      MakeDelayEmbedding(series, embedding_dimension, time_lag);

  DelayEmbeddingMatrix<Type> embedding;
  embedding.n_cols = full_embedding.n_cols;

  for (size_t row = 0; row < full_embedding.n_rows; row++) {
    bool row_has_missing = false;
    for (size_t col = 0; col < full_embedding.n_cols; col++) {
      if (full_embedding.at(row, col) == missing_value) {
        row_has_missing = true;
      }
    }

    if (!row_has_missing) {
      embedding.target_values.push_back(full_embedding.target_values[row]);
      for (size_t col = 0; col < full_embedding.n_cols; col++) {
        embedding.embedded_values.push_back(
            full_embedding.embedded_values[row * full_embedding.n_cols + col]);
      }
    }
  }

  embedding.n_rows = embedding.target_values.size();
  return embedding;
}

}  // namespace fims_edm

#endif
