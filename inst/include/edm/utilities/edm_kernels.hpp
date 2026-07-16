/**
 * @file edm_kernels.hpp
 * @brief Covariance kernel utilities for GP-EDM.
 *
 * @details Provides the Squared Exponential kernel with Automatic Relevance
 * Determination (ARD) used by GPEdmProjection.  The kernel follows the
 * formulation of Munch et al. (2017) and the GPEDM R package by Tanya Rogers:
 *
 * \f[
 *   K(x_i, x_j) = \sigma^2 \exp\!\Bigl(
 *     -\sum_{d=1}^{E} \phi_d (x_{id} - x_{jd})^2
 *   \Bigr)
 * \f]
 *
 * where:
 *  - \f$\phi_d \geq 0\f$ is the **inverse length-scale** for embedding
 *    dimension \f$d\f$ (ARD parameter): large \f$\phi_d\f$ means dimension
 *    \f$d\f$ matters a lot; \f$\phi_d \approx 0\f$ means it is irrelevant.
 *  - \f$\sigma^2 > 0\f$ is the signal variance (prior amplitude).
 *
 * The full covariance matrix used for prediction is:
 * \f[
 *   \Sigma = K + v_e I
 * \f]
 * where \f$v_e > 0\f$ is the process noise variance (nugget) added to the
 * diagonal for numerical stability and to model observation uncertainty.
 *
 * All routines use raw pointer loops for CppAD traceability.
 *
 * @references
 *  Munch, S. B., Poynor, V., and Arriaza, J. L. (2017). Circumventing
 *  structural uncertainty: a Bayesian perspective on nonlinear forecasting
 *  for ecology. Ecological Complexity, 32: 134.
 *
 *  Rogers, T. (2023). GPEDM: Gaussian process empirical dynamic modeling.
 *  https://github.com/tanyarogers/GPEDM
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_KERNELS_HPP
#define FIMS_EDM_KERNELS_HPP

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../../common/fims_math.hpp"

namespace fims_edm {

/**
 * @brief Compute a single ARD squared-exponential kernel element.
 *
 * @details Evaluates
 * \f[
 *   k(x_i, x_j) = \sigma^2 \exp\!\Bigl(-\sum_{d=0}^{E-1} \phi_d
 *                  (x_{id} - x_{jd})^2 \Bigr)
 * \f]
 * where \f$x_i\f$ and \f$x_j\f$ are embedding rows of length E, accessed via
 * raw pointers.  The summation is done in Type arithmetic so the result is
 * differentiable under TMB/CppAD.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param row_i    Pointer to the first element of embedding row i (length E).
 * @param row_j    Pointer to the first element of embedding row j (length E).
 * @param E        Embedding dimension (number of elements in each row).
 * @param phi      Per-dimension inverse length-scales (length E).  Each entry
 *                 must be >= 0.
 * @param sigma2   Signal variance (must be > 0).
 * @return  \f$k(x_i, x_j)\f$
 */
template <typename Type>
Type ARDKernelElement(const Type* row_i, const Type* row_j, size_t E,
                      const std::vector<double>& phi, double sigma2) {
  Type sq_dist = Type(0);
  for (size_t d = 0; d < E; ++d) {
    Type diff = row_i[d] - row_j[d];
    sq_dist += Type(phi[d]) * diff * diff;
  }
  return Type(sigma2) * fims_math::exp(-sq_dist);
}

/**
 * @brief Build the N×N ARD kernel matrix for the library.
 *
 * @details Fills the flat row-major vector @p K (length N*N) with:
 * \f[
 *   K[i \cdot N + j] = \sigma^2 \exp\!\Bigl(-\sum_d \phi_d
 *                       (x_{id} - x_{jd})^2\Bigr)
 * \f]
 * and then adds the process noise @p ve to every diagonal element:
 * \f[
 *   K[i \cdot N + i] \mathrel{+}= v_e
 * \f]
 *
 * The matrix is symmetric; only the upper triangle is computed and mirrored.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param embedded_values  Flat row-major pointer array for the library
 *                         (length N * E); element [row * E + col] points to
 *                         the value at (row, col) of the embedding matrix.
 * @param N        Number of library rows.
 * @param E        Embedding dimension.
 * @param phi      Per-dimension inverse length-scales (length E).
 * @param sigma2   Signal variance (> 0).
 * @param ve       Process noise variance added to the diagonal (> 0).
 * @param[out] K   Output flat row-major covariance matrix (N × N).
 *                 Resized to N*N and overwritten.
 */
template <typename Type>
void BuildCovarianceMatrix(
    const std::vector<const Type*>& embedded_values, size_t N, size_t E,
    const std::vector<double>& phi, double sigma2, double ve,
    std::vector<Type>& K) {
  K.resize(N * N);

  for (size_t i = 0; i < N; ++i) {
    const Type* row_i = embedded_values[i * E];
    for (size_t j = i; j < N; ++j) {
      const Type* row_j = embedded_values[j * E];
      Type k_ij = ARDKernelElement<Type>(row_i, row_j, E, phi, sigma2);
      K[i * N + j] = k_ij;
      K[j * N + i] = k_ij;  // exploit symmetry
    }
    // Add nugget to diagonal
    K[i * N + i] += Type(ve);
  }
}

/**
 * @brief Compute the N-element covariance vector between a query point and
 *        every library row.
 *
 * @details Fills @p k_star so that:
 * \f[
 *   k^*[i] = \sigma^2 \exp\!\Bigl(-\sum_d \phi_d (q_d - x_{id})^2\Bigr)
 * \f]
 *
 * Note: the nugget \f$v_e\f$ is NOT added here — it is only on the
 * training covariance matrix diagonal.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param query            Pointer to the E-element query row.
 * @param embedded_values  Flat row-major library pointer array (N * E).
 * @param N                Number of library rows.
 * @param E                Embedding dimension.
 * @param phi              Per-dimension inverse length-scales (length E).
 * @param sigma2           Signal variance.
 * @param[out] k_star      Output vector of length N.  Resized and overwritten.
 */
template <typename Type>
void BuildKStarVector(const Type* query,
                      const std::vector<const Type*>& embedded_values,
                      size_t N, size_t E, const std::vector<double>& phi,
                      double sigma2, std::vector<Type>& k_star) {
  k_star.resize(N);
  for (size_t i = 0; i < N; ++i) {
    const Type* row_i = embedded_values[i * E];
    k_star[i] = ARDKernelElement<Type>(query, row_i, E, phi, sigma2);
  }
}

}  // namespace fims_edm

#endif  // FIMS_EDM_KERNELS_HPP
