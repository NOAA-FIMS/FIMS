/**
 * @file mvnorm_lpdf.hpp
 * @brief Implements the MVNormLPDF distribution functor used by FIMS to
 * evaluate observation-level and total log-likelihood contributions under a
 * multivariate normal error model for data, priors, and random effects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef MVNORM_LPDF
#define MVNORM_LPDF

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <Eigen/Dense>

#include "../../common/def.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "density_components_base.hpp"

namespace fims_distributions {

/**
 * @copybrief mvnorm_lpdf.hpp
 *
 * @details This implementation evaluates the multivariate normal log probability
 * density function for a random vector \f$x \in \mathbb{R}^k\f$ with mean vector
 * \f$\mu\f$ and covariance matrix \f$\Sigma\f$:
 * \f[
 * \ln f(x \mid \mu, \Sigma) = -\frac{k}{2} \ln(2\pi) - \frac{1}{2} \ln|\Sigma|
 * -\frac{1}{2} (x - \mu)^\top \Sigma^{-1} (x - \mu)
 * \f]
 * Under `TMB_MODEL`, it delegates to TMB's `density::MVNORM_t<Type>` when available.
 * In standalone execution (or general C++ AD), it solves the linear system
 * \f$\Sigma \alpha = (x - \mu)\f$ and computes the log determinant using `Eigen::LDLT`.
 */
template <typename Type>
struct MVNormLPDF : public DensityComponentBase<Type> {
  /** @brief Covariance matrix stored in flat row-major order (k x k). */
  fims::Vector<Type> sigma_mat;

  /** @brief Dimension of the multivariate normal vector (k). */
  size_t k_dim = 0;

  /** @brief Constructor. */
  MVNormLPDF() : DensityComponentBase<Type>() {}

  /** @brief Destructor. */
  virtual ~MVNormLPDF() {}

  /**
   * @brief Evaluates the multivariate normal log probability density function.
   * @return Summed log probability density value.
   */
  virtual const Type evaluate() {
    size_t n_x = (this->observed_values.size() > 0) ? this->observed_values.size()
                                                   : this->get_n_x();

    if (k_dim == 0) {
      k_dim = n_x;
    }

    if (sigma_mat.size() != k_dim * k_dim) {
      throw std::invalid_argument(
          "MVNormLPDF: Covariance matrix size (" +
          std::to_string(sigma_mat.size()) + ") does not match k_dim^2 (" +
          std::to_string(k_dim * k_dim) + ").");
    }

    this->lpdf_vec.resize(1);
    this->lpdf_vec[0] = static_cast<Type>(0);
    this->lpdf = static_cast<Type>(0);

#ifdef TMB_MODEL
    // Build Eigen covariance matrix for TMB MVNORM_t
    Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> Sigma(k_dim, k_dim);
    for (size_t i = 0; i < k_dim; ++i) {
      for (size_t j = 0; j < k_dim; ++j) {
        Sigma(i, j) = sigma_mat[i * k_dim + j];
      }
    }
    density::MVNORM_t<Type> mvnorm_density(Sigma);
    Eigen::Matrix<Type, Eigen::Dynamic, 1> residual(k_dim);
    for (size_t i = 0; i < k_dim; ++i) {
      Type obs = (this->input_type == "prior" && this->priors.size() == 1)
                     ? (*(this->priors[0]))[i]
                     : this->get_observed(i);
      Type exp_val = (this->expected_values.size() > 0)
                         ? this->get_expected(i)
                         : static_cast<Type>(0);
      residual(i) = obs - exp_val;
    }
    // MVNORM_t returns negative log likelihood
    this->lpdf_vec[0] = -mvnorm_density(residual);
    this->lpdf = this->lpdf_vec[0];
#else
    // Standalone C++ calculation via Eigen::LDLT
    Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> Sigma(k_dim, k_dim);
    for (size_t i = 0; i < k_dim; ++i) {
      for (size_t j = 0; j < k_dim; ++j) {
        Sigma(i, j) = sigma_mat[i * k_dim + j];
      }
    }

    Eigen::Matrix<Type, Eigen::Dynamic, 1> res(k_dim);
    for (size_t i = 0; i < k_dim; ++i) {
      Type obs = (this->input_type == "prior" && this->priors.size() == 1)
                     ? (*(this->priors[0]))[i]
                     : this->get_observed(i);
      Type exp_val = (this->expected_values.size() > 0)
                         ? this->get_expected(i)
                         : static_cast<Type>(0);
      res(i) = obs - exp_val;
    }

    Eigen::LDLT<Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic>> ldlt(Sigma);
    Eigen::Matrix<Type, Eigen::Dynamic, 1> alpha = ldlt.solve(res).eval();

    Type quad_form = static_cast<Type>(0);
    for (size_t i = 0; i < k_dim; ++i) {
      quad_form += res(i) * alpha(i);
    }

    // Log-determinant from D vector of LDLT
    Type logdet = ldlt.vectorD().array().abs().log().sum();

    const double log_2pi = std::log(2.0 * M_PI);
    this->lpdf_vec[0] = static_cast<Type>(-0.5) *
                        (quad_form + logdet + static_cast<Type>(k_dim * log_2pi));
    this->lpdf = this->lpdf_vec[0];
#endif

    return this->lpdf;
  }
};

}  // namespace fims_distributions

#endif  // MVNORM_LPDF
