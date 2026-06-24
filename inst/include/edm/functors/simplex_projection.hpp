/**
 * @file simplex_projection.hpp
 * @brief Implements the Simplex Projection EDM prediction algorithm.
 *
 * @details Simplex Projection (Sugihara & May, 1990) predicts the next value
 * of a time series by finding the E+1 nearest neighbors of the query point in
 * the library manifold and returning an exponentially-weighted average of
 * their one-step-ahead targets.
 *
 * ### Algorithm
 * Given a query embedding coordinate `q` of dimension E:
 *
 *  1. Compute the squared Euclidean distance d_i from q to every library row i.
 *  2. Select the k = E+1 nearest neighbors (hand-rolled O(n*k) partial
 *     selection to remain compatible with TMB's CppAD tracer, which cannot
 *     trace std::partial_sort or std::nth_element).
 *  3. Compute exponential weights:
 *     \f[
 *       w_i = \exp\!\Bigl(-\frac{d_i}{d_{\min} + \varepsilon}\Bigr)
 *     \f]
 *     where d_min is the smallest neighbor distance and eps guards against
 *     division by zero when the query is an exact match.
 *  4. Return the normalized weighted average of the library target values
 *     for those k neighbors:
 *     \f[
 *       \hat{x}_{t+1} = \frac{\sum_i w_i \cdot x^*_i}{\sum_i w_i}
 *     \f]
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_SIMPLEX_PROJECTION_HPP
#define FIMS_EDM_SIMPLEX_PROJECTION_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../../common/fims_math.hpp"
#include "../utilities/edm_distance_weights.hpp"
#include "edm_predictor_base.hpp"

namespace fims_edm {

/**
 * @brief Simplex Projection EDM predictor.
 *
 * @details Inherits from EDMPredictorBase<Type>. Set `library` and
 * `embedding_dimension` before calling predict_one() or predict().
 *
 * `n_neighbors` defaults to embedding_dimension + 1 when left at 0 (the
 * theoretically motivated choice from Sugihara & May 1990). The caller may
 * override it for smoother (larger k) or sharper (smaller k) predictions.
 *
 * @tparam Type Numeric scalar type (double or a TMB AD scalar).
 */
template <typename Type>
struct SimplexProjection : public EDMPredictorBase<Type> {
  /**
   * @brief Number of nearest neighbors.
   * When 0 (default), predict_one() uses embedding_dimension + 1.
   * Override to a positive value to fix k regardless of E.
   */
  size_t n_neighbors = 0;

  /**
   * @brief Small constant added to d_min when computing exponential weights.
   * Prevents NaN when d_min == 0 (exact query match in the library).
   */
  double weight_epsilon = 1e-12;

  SimplexProjection() = default;
  virtual ~SimplexProjection() = default;

  /**
   * @brief Predict the next value for a single query embedding point.
   *
   * @param query_point Lagged coordinate vector of length embedding_dimension
   *   representing the state at time t:
   *   [x_t, x_{t-tau}, ..., x_{t-(E-1)*tau}].
   * @return Predicted value \f$\hat{x}_{t+1}\f$.
   * @throws std::runtime_error if library is null.
   * @throws std::invalid_argument if query_point.size() != embedding_dimension.
   * @throws std::runtime_error if the library has fewer rows than n_neighbors.
   */
  virtual Type predict_one(const fims::Vector<Type>& query_point) override {
    this->ValidateLibrary();
    this->ValidateQueryPoint(query_point);

    const size_t k =
        (n_neighbors == 0) ? (this->embedding_dimension + 1) : n_neighbors;
    const size_t n_lib = this->library->n_rows;
    const size_t E = this->embedding_dimension;

    if (n_lib < k) {
      throw std::runtime_error(
          "SimplexProjection: library has fewer rows than n_neighbors (k). "
          "Increase the library size or decrease n_neighbors.");
    }

    // -----------------------------------------------------------------------
    // Step 1 & 2: Compute squared distances and track the k nearest neighbors
    // with a hand-rolled O(n*k) partial selection. Using raw pointer indexing
    // into embedded_values so the loop is CppAD-traceable.
    //
    // nn_indices[m] = library row index of the m-th nearest-neighbor slot.
    // nn_sq_dist[m] = its squared distance (initialized to +inf).
    // -----------------------------------------------------------------------
    const double kInf = std::numeric_limits<double>::max();
    std::vector<size_t> nn_indices(k, 0);
    std::vector<Type> nn_sq_dist(k, Type(kInf));

    for (size_t i = 0; i < n_lib; ++i) {
      // Squared distance from query_point to library row i.
      Type d = Type(0);
      for (size_t j = 0; j < E; ++j) {
        Type diff = query_point[j] - *(this->library->embedded_values[i * E + j]);
        d += diff * diff;
      }

      // Find the slot with the current worst (largest) distance.
      size_t worst = 0;
      for (size_t m = 1; m < k; ++m) {
        if (nn_sq_dist[m] > nn_sq_dist[worst]) {
          worst = m;
        }
      }

      // Replace the worst slot if this neighbor is closer.
      if (d < nn_sq_dist[worst]) {
        nn_sq_dist[worst] = d;
        nn_indices[worst] = i;
      }
    }

    // -----------------------------------------------------------------------
    // Step 3: Compute Simplex exponential weights using the shared utility.
    // w_i = exp(-d_i / (d_min + eps)), then normalize so they sum to 1.
    // -----------------------------------------------------------------------
    std::vector<Type> weights;
    SimplexWeights(nn_sq_dist, weights, weight_epsilon);
    NormalizeWeights(weights);

    // -----------------------------------------------------------------------
    // Step 4: Normalized weighted average of target values.
    // -----------------------------------------------------------------------
    Type prediction = Type(0);
    for (size_t m = 0; m < k; ++m) {
      prediction += weights[m] * (*this->library->target_values[nn_indices[m]]);
    }
    return prediction;
  }

  /**
   * @brief Predict for all rows of a test embedding matrix.
   *
   * Iterates over every row of @p test_embedding, builds the query coordinate
   * vector, calls predict_one(), and stores the results in `predictions`.
   * After this call: predictions.size() == test_embedding.n_rows.
   *
   * @param test_embedding The query manifold. n_cols must equal
   *   embedding_dimension.
   * @throws std::runtime_error / std::invalid_argument forwarded from
   *   predict_one() and ValidateLibrary().
   */
  virtual void predict(
      const DelayEmbeddingMatrix<Type>& test_embedding) override {
    this->ValidateLibrary();

    const size_t E = this->embedding_dimension;
    if (test_embedding.n_cols != E) {
      throw std::invalid_argument(
          "SimplexProjection::predict: test embedding n_cols does not match "
          "embedding_dimension.");
    }

    const size_t n_test = test_embedding.n_rows;
    this->predictions.resize(n_test);

    fims::Vector<Type> query_point(E);
    for (size_t row = 0; row < n_test; ++row) {
      for (size_t col = 0; col < E; ++col) {
        query_point[col] = test_embedding.at(row, col);
      }
      this->predictions[row] = predict_one(query_point);
    }
  }
};

}  // namespace fims_edm

#endif  // FIMS_EDM_SIMPLEX_PROJECTION_HPP
