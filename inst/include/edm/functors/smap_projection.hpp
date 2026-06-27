/**
 * @file smap_projection.hpp
 * @brief Implements the S-Map (Sequential Locally Weighted Global Linear Maps)
 *        EDM prediction algorithm.
 *
 * @details S-Map (Sugihara 1994) predicts the next value of a time series by
 * fitting a **locally weighted linear regression** using ALL library rows.
 * Unlike Simplex Projection (which uses only E+1 nearest neighbors),
 * S-Map uses every library point but down-weights distant ones via a
 * kernel function controlled by the nonlinearity parameter theta (θ).
 *
 * Two kernel functions are supported (see SMapKernel):
 *  - SMapKernel::kExponential (default): w = exp(-θ · d / d̄)  [rEDM style]
 *  - SMapKernel::kGaussian:              w = exp(-θ² · (d/D)²) [Esguerra & Munch 2024]
 * Per Munch (pers. comm.) the performance difference is rarely large;
 * use kGaussian to avoid a sqrt() inside the AD tape.
 *
 * ### Algorithm
 * Given a query embedding coordinate q of dimension E:
 *
 *  1. Compute squared Euclidean distance d_i from q to every library row i.
 *  2. Compute S-Map weights using SMapWeights() from edm_distance_weights.hpp:
 *     \f[
 *       w_i = \exp\!\Bigl(-\theta \cdot \frac{d_i}{\bar{d}}\Bigr)
 *     \f]
 *     where \f$\bar{d}\f$ is the mean squared distance across all library rows
 *     and \f$\theta \geq 0\f$ controls locality:
 *       - \f$\theta = 0\f$: all weights equal → global linear model.
 *       - Large \f$\theta\f$: only nearby rows contribute → highly local.
 *  3. Assemble the weighted normal equations for coefficient vector
 *     \f$\boldsymbol{\beta}\f$ of size E+1 (intercept + E slopes):
 *     \f[
 *       \mathbf{A} = \mathbf{X}^{\top} \mathbf{W} \mathbf{X},\quad
 *       \mathbf{b} = \mathbf{X}^{\top} \mathbf{W} \mathbf{y}
 *     \f]
 *     where row i of \f$\mathbf{X}\f$ is \f$[1,\, x_{i1},\ldots, x_{iE}]\f$,
 *     \f$\mathbf{W}\f$ is diagonal with entries \f$w_i\f$, and
 *     \f$y_i\f$ is the library target value.
 *  4. Solve \f$\mathbf{A}\boldsymbol{\beta} = \mathbf{b}\f$ via Gaussian
 *     elimination with partial pivoting (in-place, CppAD-compatible).
 *  5. Return \f$\hat{x}_{t+1} = [1, q_1, \ldots, q_E]\cdot\boldsymbol{\beta}\f$.
 *
 * ### Relationship to Simplex Projection
 * | Feature | Simplex | S-Map |
 * |---|---|---|
 * | Library rows used | E+1 nearest | ALL |
 * | Prediction model | weighted average | locally weighted linear regression |
 * | Key parameter | n_neighbors k | nonlinearity θ |
 * | Weighting (default) | exp(-d/d_min) | exp(-θ·d/d̄) |
 * | Weighting (Gaussian) | — | exp(-θ²·(d/D)²) |
 *
 * @see edm_distance_weights.hpp for SMapWeights() and NormalizeWeights().
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_SMAP_PROJECTION_HPP
#define FIMS_EDM_SMAP_PROJECTION_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../utilities/edm_distance_weights.hpp"
#include "edm_predictor_base.hpp"

namespace fims_edm {

// ---------------------------------------------------------------------------
// Internal helper: Gaussian elimination with partial pivoting
// ---------------------------------------------------------------------------

/**
 * @brief Solve the linear system A x = b in-place via Gaussian elimination
 *        with partial pivoting.
 *
 * @details Both @p A and @p b are modified during elimination. On exit,
 * @p b contains the solution vector x. The matrix @p A is stored in
 * row-major order as a flat std::vector of size n*n.
 *
 * This implementation avoids std::sort and std::swap on AD scalars so that
 * the loop structure remains traceable by TMB's CppAD tracer. Index
 * operations (pivot selection, row swaps) use double-typed indices.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param A    Flat row-major coefficient matrix (n × n), modified in-place.
 * @param b    Right-hand-side vector (length n), overwritten with solution.
 * @param n    System dimension.
 * @throws std::runtime_error if the matrix is (near-)singular.
 */
template <typename Type>
void GaussianElimination(std::vector<Type>& A, std::vector<Type>& b,
                         size_t n) {
  for (size_t col = 0; col < n; ++col) {
    // --- Partial pivot: find the row with the largest absolute value in
    //     column `col`, at or below the current diagonal. ---
    size_t pivot_row = col;
    double max_val = 0.0;
    for (size_t row = col; row < n; ++row) {
      // Cast to double for pivot comparison only (not in AD trace).
      double val = static_cast<double>(A[row * n + col]);
      if (val < 0.0) val = -val;
      if (val > max_val) {
        max_val = val;
        pivot_row = row;
      }
    }
    if (max_val < 1e-14) {
      throw std::runtime_error(
          "SMapProjection::GaussianElimination: matrix is singular or "
          "near-singular. The library may be too small, collinear, or "
          "theta may need adjustment.");
    }

    // --- Swap current row with pivot row (both A and b). ---
    if (pivot_row != col) {
      for (size_t j = 0; j < n; ++j) {
        Type tmp = A[col * n + j];
        A[col * n + j] = A[pivot_row * n + j];
        A[pivot_row * n + j] = tmp;
      }
      Type tmp_b = b[col];
      b[col] = b[pivot_row];
      b[pivot_row] = tmp_b;
    }

    // --- Eliminate entries below the pivot. ---
    Type pivot_val = A[col * n + col];
    for (size_t row = col + 1; row < n; ++row) {
      Type factor = A[row * n + col] / pivot_val;
      for (size_t j = col; j < n; ++j) {
        A[row * n + j] -= factor * A[col * n + j];
      }
      b[row] -= factor * b[col];
    }
  }

  // --- Back-substitution. ---
  for (size_t i = n; i-- > 0;) {
    Type sum = Type(0);
    for (size_t j = i + 1; j < n; ++j) {
      sum += A[i * n + j] * b[j];
    }
    b[i] = (b[i] - sum) / A[i * n + i];
  }
}

// ---------------------------------------------------------------------------
// SMapProjection
// ---------------------------------------------------------------------------

/**
 * @brief S-Map EDM predictor (Sugihara 1994).
 *
 * @details Inherits from EDMPredictorBase<Type>. Set `library` and
 * `embedding_dimension` before calling predict_one() or predict().
 *
 * @tparam Type Numeric scalar type (double or a TMB AD scalar).
 */
template <typename Type>
struct SMapProjection : public EDMPredictorBase<Type> {
  /**
   * @brief Nonlinearity parameter θ ≥ 0.
   *
   * Controls how strongly the model localizes around the query point.
   *  - θ = 0 → global linear regression (all weights equal 1).
   *  - θ > 0 → down-weight distant library rows via the chosen kernel.
   * Typical values: 0, 0.5, 1, 2, 4, 8 (scan via cross-validation).
   */
  double theta = 1.0;

  /**
   * @brief Weighting kernel used to compute S-Map locality weights.
   *
   * Choices:
   *  - SMapKernel::kExponential (default): w = exp(-θ·d/d̄)  [classic rEDM]
   *  - SMapKernel::kGaussian:              w = exp(-θ²·(d/D)²) [Esguerra & Munch 2024]
   *
   * Both kernels reduce to uniform weights when θ = 0 (global OLS).
   * Per Munch (pers. comm.) the performance difference is rarely large;
   * kGaussian avoids a sqrt() call inside the CppAD tape.
   */
  SMapKernel kernel = SMapKernel::kExponential;

  /**
   * @brief Small constant added to the mean distance to prevent division
   * by zero when all library rows are identical to the query.
   */
  double epsilon = 1e-12;

  SMapProjection() = default;
  virtual ~SMapProjection() = default;

  /**
   * @brief Predict the next value for a single query embedding point.
   *
   * @details Fits a locally weighted linear regression using all library rows,
   * with weights w_i = exp(-θ * d_i / d̄). The regression includes an
   * intercept term, so the coefficient vector β has length E+1.
   *
   * @param query_point Lagged coordinate vector of length embedding_dimension
   *   representing the state at time t:
   *   [x_t, x_{t-tau}, ..., x_{t-(E-1)*tau}].
   * @return Predicted value \f$\hat{x}_{t+1}\f$.
   * @throws std::runtime_error if library is null.
   * @throws std::invalid_argument if query_point.size() != embedding_dimension.
   * @throws std::runtime_error if the weighted design matrix is singular.
   */
  virtual Type predict_one(const fims::Vector<Type>& query_point) override {
    this->ValidateLibrary();
    this->ValidateQueryPoint(query_point);

    const size_t n_lib = this->library->n_rows;
    const size_t E = this->embedding_dimension;

    if (n_lib < E + 1) {
      throw std::runtime_error(
          "SMapProjection: library must have at least E+1 rows to fit a "
          "linear model with E predictors and an intercept.");
    }

    // -----------------------------------------------------------------------
    // Step 1: Compute squared distance from query to ALL library rows.
    // Raw pointer indexing keeps the loop CppAD-traceable.
    // -----------------------------------------------------------------------
    std::vector<Type> sq_dists(n_lib, Type(0));
    for (size_t i = 0; i < n_lib; ++i) {
      for (size_t j = 0; j < E; ++j) {
        Type diff = query_point[j] -
                    *(this->library->embedded_values[i * E + j]);
        sq_dists[i] += diff * diff;
      }
    }

    // -----------------------------------------------------------------------
    // Step 2: Compute S-Map weights using the user-selected kernel.
    // kExponential: w_i = exp(-theta * d_i / d_mean)
    // kGaussian:    w_i = exp(-theta^2 * (d_i / D)^2)
    // -----------------------------------------------------------------------
    std::vector<Type> weights;
    SMapWeights(sq_dists, weights, theta, kernel, epsilon);

    // -----------------------------------------------------------------------
    // Step 3: Assemble normal equations A β = b.
    //
    // Design matrix row i:  x̃_i = [1, lib[i][0], ..., lib[i][E-1]]  (length E+1)
    // A[k][j] += w_i * x̃_i[k] * x̃_i[j]
    // b[k]    += w_i * x̃_i[k] * y_i
    //
    // A is stored flat, row-major, size (E+1)*(E+1).
    // -----------------------------------------------------------------------
    const size_t p = E + 1;  // number of parameters (intercept + E slopes)
    std::vector<Type> A(p * p, Type(0));
    std::vector<Type> b_vec(p, Type(0));

    for (size_t i = 0; i < n_lib; ++i) {
      const Type w = weights[i];
      const Type yi = *(this->library->target_values[i]);

      // Build design-row x̃_i = [1, lib[i][0], ..., lib[i][E-1]]
      // We compute A and b without materializing x̃_i as a vector.
      // x̃_i[0] = 1 (intercept), x̃_i[k] = lib[i][k-1] for k >= 1.
      for (size_t k = 0; k < p; ++k) {
        Type xk = (k == 0) ? Type(1) : *(this->library->embedded_values[i * E + k - 1]);
        b_vec[k] += w * xk * yi;
        for (size_t jj = 0; jj < p; ++jj) {
          Type xj = (jj == 0) ? Type(1) : *(this->library->embedded_values[i * E + jj - 1]);
          A[k * p + jj] += w * xk * xj;
        }
      }
    }

    // -----------------------------------------------------------------------
    // Step 4: Solve A β = b via Gaussian elimination (in-place).
    // On exit b_vec holds the solution β.
    // -----------------------------------------------------------------------
    GaussianElimination(A, b_vec, p);

    // -----------------------------------------------------------------------
    // Step 5: Predict ŷ = [1, q[0], ..., q[E-1]] · β
    // -----------------------------------------------------------------------
    Type prediction = b_vec[0];  // intercept
    for (size_t j = 0; j < E; ++j) {
      prediction += b_vec[j + 1] * query_point[j];
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
          "SMapProjection::predict: test embedding n_cols does not match "
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

#endif  // FIMS_EDM_SMAP_PROJECTION_HPP
