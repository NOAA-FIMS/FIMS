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
 *
 * Uncertainty propagation follows the same pointer-based pattern:
 *  - embedded_uncertainty: pointers into the input_uncertainty series,
 *    laid out identically to embedded_values
 *  - target_uncertainty: one pointer per row, pointing to the uncertainty
 *    at the target time-point (mirrors target_values)
 *
 * If no uncertainty series is provided to the factory functions,
 * embedded_uncertainty and target_uncertainty remain empty.
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
   * @brief Flattened row-major pointers into the input_uncertainty series.
   * Laid out identically to embedded_values:
   * element [row * n_cols + col] points to sigma_{t - col * tau}.
   * Empty when no uncertainty series is provided.
   */
  std::vector<const Type*> embedded_uncertainty;
  /**
   * @brief Pointers to the uncertainty at the target time-point for each row.
   * target_uncertainty[row] points to sigma_t for that row.
   * Empty when no uncertainty series is provided.
   */
  std::vector<const Type*> target_uncertainty;

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
 *
 * If @p uncertainty is non-empty (and must be the same length as @p series),
 * embedded_uncertainty and target_uncertainty are populated with pointers into
 * @p uncertainty following the same layout. If @p uncertainty is empty, those
 * fields are left empty.
 */
template <typename Type>
DelayEmbeddingMatrix<Type> MakeDelayEmbedding(
    const fims::Vector<Type>& series, size_t embedding_dimension,
    size_t time_lag,
    const fims::Vector<Type>& uncertainty = fims::Vector<Type>()) {
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

  const bool has_uncertainty = (uncertainty.size() == 0) ? false : true;
  if (has_uncertainty && uncertainty.size() != series.size()) {
    throw std::invalid_argument(
        "uncertainty must be the same length as series.");
  }

  DelayEmbeddingMatrix<Type> embedding;
  embedding.n_rows = series.size() - lag_span;
  embedding.n_cols = embedding_dimension;
  embedding.embedded_values.resize(embedding.n_rows * embedding.n_cols);
  embedding.target_values.resize(embedding.n_rows);

  if (has_uncertainty) {
    embedding.embedded_uncertainty.resize(embedding.n_rows * embedding.n_cols);
    embedding.target_uncertainty.resize(embedding.n_rows);
  }

  for (size_t row = 0; row < embedding.n_rows; row++) {
    const size_t target_index = row + lag_span;
    // target_values[row] points to the x_t value for this row
    embedding.target_values[row] = &series[target_index];
    for (size_t col = 0; col < embedding.n_cols; col++) {
      embedding.embedded_values[row * embedding.n_cols + col] =
          &series[target_index - col * time_lag];
    }

    if (has_uncertainty) {
      embedding.target_uncertainty[row] = &uncertainty[target_index];
      for (size_t col = 0; col < embedding.n_cols; col++) {
        embedding.embedded_uncertainty[row * embedding.n_cols + col] =
            &uncertainty[target_index - col * time_lag];
      }
    }
  }

  return embedding;
}

/**
 * @brief Build a delay embedding matrix and omit rows containing missing
 * values.
 *
 * If @p uncertainty is non-empty (same length as @p series), the uncertainty
 * fields are propagated for the retained rows using the same pointer-based
 * approach as MakeDelayEmbedding.
 */
template <typename Type>
DelayEmbeddingMatrix<Type> MakeDelayEmbeddingDropMissing(
    const fims::Vector<Type>& series, size_t embedding_dimension,
    size_t time_lag, const Type& missing_value,
    const fims::Vector<Type>& uncertainty = fims::Vector<Type>()) {
  DelayEmbeddingMatrix<Type> full_embedding =
      MakeDelayEmbedding(series, embedding_dimension, time_lag, uncertainty);

  DelayEmbeddingMatrix<Type> embedding;
  embedding.n_cols = full_embedding.n_cols;

  const bool has_uncertainty = (full_embedding.embedded_uncertainty.size() > 0);

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

      if (has_uncertainty) {
        embedding.target_uncertainty.push_back(
            full_embedding.target_uncertainty[row]);
        for (size_t col = 0; col < full_embedding.n_cols; col++) {
          embedding.embedded_uncertainty.push_back(
              full_embedding
                  .embedded_uncertainty[row * full_embedding.n_cols + col]);
        }
      }
    }
  }

  embedding.n_rows = embedding.target_values.size();
  return embedding;
}

}  // namespace fims_edm

#endif
