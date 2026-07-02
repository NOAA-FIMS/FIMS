/**
 * @file gp_edm_projection.hpp
 * @brief Implements the GP-EDM prediction algorithm with ARD priors.
 *
 * @details GP-EDM (Gaussian Process Empirical Dynamic Modelling) predicts the
 * next value of a time series using a Gaussian Process regression fitted to
 * the delay embedding library.  The kernel uses per-dimension inverse
 * length-scale parameters (Automatic Relevance Determination, ARD) that are
 * estimated by maximising the log posterior via the Rprop gradient optimizer.
 *
 * ### Mathematical formulation
 *
 * Given N library rows \f$X \in \mathbb{R}^{N \times E}\f$ and target vector
 * \f$y \in \mathbb{R}^N\f$, the covariance matrix is:
 * \f[
 *   \Sigma_{ij} = \sigma^2 \exp\!\Bigl(-\sum_{d=1}^{E}
 *                 \phi_d (x_{id}-x_{jd})^2\Bigr) + v_e\,\delta_{ij}
 * \f]
 *
 * Hyperparameters optimized by Rprop:
 *  - \f$\boldsymbol{\phi} = [\phi_1,\ldots,\phi_E]\f$ — ARD inverse
 *    length-scales; one per embedding dimension (prior: half-Normal).
 *  - \f$\sigma^2\f$ — signal variance (prior: Beta-shaped on (0, 5)).
 *  - \f$v_e\f$ — process noise / nugget variance (prior: Beta-shaped on (0, 5)).
 *
 * The posterior mean prediction for a query point \f$q\f$ is:
 * \f[
 *   \hat{y}(q) = k^*(q)^\top \Sigma^{-1} y
 * \f]
 * where \f$k^*(q)_i = \sigma^2 \exp\!\bigl(-\sum_d \phi_d(q_d-x_{id})^2\bigr)\f$.
 *
 * ### Usage
 * ```cpp
 * GPEdmProjection<double> gp;
 * gp.library            = &train_embedding;
 * gp.embedding_dimension = E;
 * gp.phi                = std::vector<double>(E, 0.5); // initial ARD params
 * gp.fit();                  // optimize phi, sigma2, ve from library
 * gp.predict(test_embedding); // fill gp.predictions
 * ```
 *
 * @references
 *  Munch, S. B., Poynor, V., and Arriaza, J. L. (2017). Circumventing
 *  structural uncertainty: a Bayesian perspective on nonlinear forecasting
 *  for ecology. Ecological Complexity, 32: 134.
 *
 *  Rogers, T. (2023). GPEDM: Gaussian process empirical dynamic modeling.
 *  https://github.com/tanyarogers/GPEDM  (R reference implementation)
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_GP_EDM_PROJECTION_HPP
#define FIMS_EDM_GP_EDM_PROJECTION_HPP

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../../common/fims_math.hpp"
#include "../utilities/edm_kernels.hpp"
#include "../utilities/edm_linear_algebra.hpp"
#include "edm_predictor_base.hpp"

namespace fims_edm {

/**
 * @brief GP-EDM prediction functor with per-dimension ARD length-scale priors.
 *
 * Inherits from EDMPredictorBase<Type>. The caller must set `library` and
 * `embedding_dimension` before calling fit() or predict_one() / predict().
 *
 * Hyperparameters (`phi`, `sigma2`, `ve`) may be set manually or estimated
 * from the library by calling fit().
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 */
template <typename Type>
struct GPEdmProjection : public EDMPredictorBase<Type> {
  // -----------------------------------------------------------------------
  // Hyperparameters (can be set manually or estimated via fit())
  // -----------------------------------------------------------------------

  /**
   * @brief Per-dimension ARD inverse length-scales (length = embedding_dimension).
   *
   * Each entry \f$\phi_d \geq 0\f$ controls how quickly the kernel decays
   * along embedding dimension d:
   *  - \f$\phi_d \approx 0\f$: dimension d is nearly irrelevant (dropped by ARD).
   *  - Large \f$\phi_d\f$: strong locality / high nonlinearity along d.
   *
   * If empty when fit() is called, initialised to 0.1 per dimension.
   */
  std::vector<double> phi;

  /**
   * @brief Signal variance \f$\sigma^2 > 0\f$.
   * Controls the overall amplitude of the covariance function.
   */
  double sigma2 = 1.0;

  /**
   * @brief Process noise variance \f$v_e > 0\f$ (nugget).
   * Added to the diagonal of the covariance matrix for numerical stability
   * and to represent observation / process uncertainty.
   */
  double ve = 0.1;

  /**
   * @brief Maximum Rprop iterations for hyperparameter optimization.
   * Matches the default in the GPEDM R package (Rogers 2023).
   */
  size_t max_fit_iterations = 200;

  // -----------------------------------------------------------------------
  // Public interface
  // -----------------------------------------------------------------------

  /**
   * @brief Estimate hyperparameters from the library via Rprop MAP optimization.
   *
   * @details Maximises the log posterior:
   * \f[
   *   \ln p(\boldsymbol{\phi}, \sigma^2, v_e \mid y) =
   *   -\tfrac{1}{2} y^\top \Sigma^{-1} y - \tfrac{1}{2} \ln|\Sigma|
   *   + \ln \pi(\boldsymbol{\phi}) + \ln \pi(\sigma^2) + \ln \pi(v_e)
   * \f]
   * using the Rprop (Resilient Backpropagation) optimizer with adaptive
   * per-parameter step sizes, matching the GPEDM R package (Rogers 2023).
   *
   * Parameters are optimised in transformed (unconstrained) space:
   *  - \f$\phi_d\f$: \f$\ln \phi_d\f$ (log transform)
   *  - \f$\sigma^2\f$: logit on (0.0001, 5)
   *  - \f$v_e\f$: logit on (0.0001, 5)
   *
   * After calling fit(), `phi`, `sigma2`, and `ve` contain the MAP estimates.
   *
   * @throws std::runtime_error if library is null or N < 2.
   * @throws std::invalid_argument if embedding_dimension mismatches library.
   */
  void fit() {
    this->ValidateLibrary();
    const size_t N = this->library->n_rows;
    const size_t E = this->embedding_dimension;

    if (N < 2) {
      throw std::runtime_error(
          "GPEdmProjection::fit: library must have at least 2 rows.");
    }

    // --- Initialize phi if empty ---
    if (phi.empty()) {
      phi.assign(E, 0.1);
    }
    if (phi.size() != E) {
      throw std::invalid_argument(
          "GPEdmProjection::fit: phi.size() must equal embedding_dimension.");
    }

    // --- Bounds for sigma2 and ve (same as GPEDM R package) ---
    const double kVeMin = 0.0001, kVeMax = 4.9999;
    const double kS2Min = 0.0001, kS2Max = 4.9999;

    // --- Build initial parameter vector in transformed space ---
    // parst = [log(phi[0]), ..., log(phi[E-1]), logit(ve), logit(sigma2)]
    const size_t np = E + 2;
    std::vector<double> parst(np);
    for (size_t d = 0; d < E; ++d) {
      parst[d] = std::log(phi[d] > 1e-12 ? phi[d] : 1e-12);
    }
    parst[E]     = logit_transform(ve,     kVeMin, kVeMax);
    parst[E + 1] = logit_transform(sigma2, kS2Min, kS2Max);

    // --- Extract library targets into a plain double vector ---
    std::vector<double> y(N);
    for (size_t i = 0; i < N; ++i) {
      y[i] = static_cast<double>(*(this->library->target_values[i]));
    }

    // --- Pre-compute per-dimension squared-distance matrices (N x N) ---
    // D[d][i*N + j] = (x_id - x_jd)^2
    std::vector<std::vector<double>> D(E, std::vector<double>(N * N, 0.0));
    for (size_t d = 0; d < E; ++d) {
      for (size_t i = 0; i < N; ++i) {
        double xi =
            static_cast<double>(*(this->library->embedded_values[i * E + d]));
        for (size_t j = i; j < N; ++j) {
          double xj =
              static_cast<double>(*(this->library->embedded_values[j * E + d]));
          double diff = xi - xj;
          double sq   = diff * diff;
          D[d][i * N + j] = sq;
          D[d][j * N + i] = sq;
        }
      }
    }

    // --- Rprop optimizer ---
    // Adaptive per-parameter step sizes, sign-based descent.
    const double kEtaPlus  = 0.2;   // step growth  (factor 1+eta)
    const double kEtaMinus = -0.5;  // step shrink  (factor 1+eta)
    const double kDeltaMin = 1e-6;
    const double kDeltaMax = 50.0;

    std::vector<double> delta(np, 0.1);
    std::vector<double> grad_prev(np, 0.0);
    double nllpost_prev = std::numeric_limits<double>::max();

    // Compute initial gradient
    LogPosteriorResult res = compute_log_posterior_grad(
        parst, y, D, N, E, kVeMin, kVeMax, kS2Min, kS2Max);

    for (size_t iter = 0; iter < max_fit_iterations; ++iter) {
      // Convergence check (gradient norm and relative change in objective)
      double gnorm = 0.0;
      for (size_t k = 0; k < np; ++k) gnorm += res.grad[k] * res.grad[k];
      gnorm = std::sqrt(gnorm);
      double df = std::abs(res.nllpost / (nllpost_prev + 1e-15) - 1.0);
      if (gnorm < 1e-4 && df < 1e-7) break;
      nllpost_prev = res.nllpost;

      // --- Rprop step ---
      std::vector<double> parst_new(np);
      for (size_t k = 0; k < np; ++k) {
        parst_new[k] = parst[k] - sign_d(res.grad[k]) * delta[k];
      }

      LogPosteriorResult res_new = compute_log_posterior_grad(
          parst_new, y, D, N, E, kVeMin, kVeMax, kS2Min, kS2Max);

      // Update adaptive step sizes
      for (size_t k = 0; k < np; ++k) {
        double gc = res.grad[k] * res_new.grad[k];
        double tdelta =
            delta[k] * (1.0 + (gc > 0.0 ? kEtaPlus : (gc < 0.0 ? kEtaMinus : 0.0)));
        delta[k] = tdelta < kDeltaMin ? kDeltaMin
                   : (tdelta > kDeltaMax ? kDeltaMax : tdelta);
      }

      parst     = parst_new;
      res       = res_new;
    }

    // --- Untransform optimized parameters ---
    for (size_t d = 0; d < E; ++d) {
      phi[d] = std::exp(parst[d]);
    }
    ve     = logit_inverse(parst[E],     kVeMin, kVeMax);
    sigma2 = logit_inverse(parst[E + 1], kS2Min, kS2Max);
  }

  /**
   * @brief Predict the next value for a single query embedding point.
   *
   * @details Computes the GP posterior mean:
   * \f[
   *   \hat{y}(q) = k^*(q)^\top \Sigma^{-1} y
   * \f]
   * using the current values of `phi`, `sigma2`, and `ve`.  If fit() has
   * not been called, the default hyperparameters are used directly.
   *
   * @param query_point  Length-E vector of lagged coordinates for the query.
   * @return Predicted value at the next time step.
   * @throws std::runtime_error if library is null or has fewer than 2 rows.
   * @throws std::invalid_argument if query_point.size() != embedding_dimension.
   */
  Type predict_one(const fims::Vector<Type>& query_point) override {
    this->ValidateLibrary();
    this->ValidateQueryPoint(query_point);

    const size_t N = this->library->n_rows;
    const size_t E = this->embedding_dimension;

    if (N < 2) {
      throw std::runtime_error(
          "GPEdmProjection::predict_one: library must have at least 2 rows.");
    }
    if (phi.size() != E) {
      throw std::invalid_argument(
          "GPEdmProjection::predict_one: phi.size() must equal "
          "embedding_dimension. Call fit() first or set phi manually.");
    }

    // --- Build N×N covariance matrix Sigma = K + ve*I ---
    std::vector<Type> Sigma;
    BuildCovarianceMatrix<Type>(this->library->embedded_values, N, E,
                                phi, sigma2, ve, Sigma);

    // --- Build target vector y ---
    std::vector<Type> y(N);
    for (size_t i = 0; i < N; ++i) {
      y[i] = *(this->library->target_values[i]);
    }

    // --- Solve Sigma * alpha = y (alpha overwrites y) ---
    GaussianElimination<Type>(Sigma, y, N);
    // y now holds alpha = Sigma^{-1} y_targets

    // --- Build k_star: covariance between query and every library row ---
    std::vector<Type> k_star;
    BuildKStarVector<Type>(query_point.data(), this->library->embedded_values,
                           N, E, phi, sigma2, k_star);

    // --- Posterior mean: y_hat = k_star^T alpha ---
    Type y_hat = Type(0);
    for (size_t i = 0; i < N; ++i) {
      y_hat += k_star[i] * y[i];
    }
    return y_hat;
  }

  /**
   * @brief Predict for every row of a test embedding matrix.
   *
   * @details Iterates over @p test_embedding rows, extracts each query vector,
   * calls predict_one(), and stores results in `predictions`.
   *
   * @param test_embedding  Query manifold (n_cols must equal embedding_dimension).
   * @throws std::runtime_error if library is null or has fewer than 2 rows.
   * @throws std::invalid_argument if test_embedding.n_cols != embedding_dimension.
   */
  void predict(const DelayEmbeddingMatrix<Type>& test_embedding) override {
    this->ValidateLibrary();
    if (test_embedding.n_cols != this->embedding_dimension) {
      throw std::invalid_argument(
          "GPEdmProjection::predict: test_embedding.n_cols does not match "
          "embedding_dimension.");
    }

    const size_t n_test = test_embedding.n_rows;
    this->predictions.resize(n_test);

    fims::Vector<Type> query(this->embedding_dimension);
    for (size_t row = 0; row < n_test; ++row) {
      for (size_t col = 0; col < this->embedding_dimension; ++col) {
        query[col] = test_embedding.at(row, col);
      }
      this->predictions[row] = predict_one(query);
    }
  }

 private:
  // -----------------------------------------------------------------------
  // Rprop helpers (all in double arithmetic, not in the AD trace)
  // -----------------------------------------------------------------------

  /** @brief Result of one log-posterior evaluation. */
  struct LogPosteriorResult {
    double nllpost;           ///< Negative log posterior value.
    std::vector<double> grad; ///< Gradient of nllpost w.r.t. parst.
  };

  /** @brief Logit transform: maps x in (lo, hi) to the real line. */
  static double logit_transform(double x, double lo, double hi) {
    double p = (x - lo) / (hi - lo);
    p = p < 1e-9 ? 1e-9 : (p > 1.0 - 1e-9 ? 1.0 - 1e-9 : p);
    return std::log(p / (1.0 - p));
  }

  /** @brief Inverse logit: maps real line back to (lo, hi). */
  static double logit_inverse(double t, double lo, double hi) {
    return (hi - lo) / (1.0 + std::exp(-t)) + lo;
  }

  /** @brief Sign function returning -1, 0, or +1 as a double. */
  static double sign_d(double x) {
    return (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0);
  }

  /**
   * @brief Evaluate the negative log posterior and its gradient.
   *
   * @details This runs entirely in double arithmetic (not on the AD tape).
   * Uses the analytic gradient derived from the GP marginal likelihood:
   * \f{align*}{
   *   \ln p &= -\tfrac{1}{2} y^\top \Sigma^{-1} y
   *            -\tfrac{1}{2} \ln|\Sigma| + \ln\pi(\boldsymbol{\phi},
   *            \sigma^2, v_e) \\
   *   \nabla_{\phi_d} \ln p &= \tfrac{1}{2} \operatorname{tr}
   *     \bigl((\alpha\alpha^\top - \Sigma^{-1}) \partial_{\phi_d}\Sigma\bigr)\\
   *   \partial_{\phi_d}\Sigma_{ij} &= -D_{d,ij}\, K_{ij}
   * \f}
   * where \f$\alpha = \Sigma^{-1} y\f$ and \f$D_{d,ij} = (x_{id}-x_{jd})^2\f$.
   *
   * @param parst   Transformed parameters (log phi; logit ve; logit sigma2).
   * @param y       Target vector (length N, plain double).
   * @param D       Per-dimension squared-distance matrices (E × N*N).
   * @param N, E    Library dimensions.
   * @param ve_min, ve_max, s2_min, s2_max  Parameter bounds.
   */
  LogPosteriorResult compute_log_posterior_grad(
      const std::vector<double>& parst, const std::vector<double>& y,
      const std::vector<std::vector<double>>& D, size_t N, size_t E,
      double ve_min, double ve_max, double s2_min, double s2_max) const {
    const size_t np = E + 2;

    // --- Untransform parameters ---
    std::vector<double> phi_cur(E);
    for (size_t d = 0; d < E; ++d) phi_cur[d] = std::exp(parst[d]);
    double ve_cur     = logit_inverse(parst[E],     ve_min, ve_max);
    double sigma2_cur = logit_inverse(parst[E + 1], s2_min, s2_max);

    // --- Jacobian factors (chain rule through transforms) ---
    // d(nllpost)/d(parst[d]) = d(nllpost)/d(phi[d]) * d(phi[d])/d(parst[d])
    // d(phi[d])/d(log phi[d]) = phi[d]
    // d(ve)/d(logit ve) = (ve - ve_min)(1 - (ve - ve_min)/(ve_max - ve_min))
    std::vector<double> dpars(np, 0.0);
    for (size_t d = 0; d < E; ++d) dpars[d] = phi_cur[d];
    dpars[E]     = (ve_cur     - ve_min) * (1.0 - (ve_cur     - ve_min) / (ve_max - ve_min));
    dpars[E + 1] = (sigma2_cur - s2_min) * (1.0 - (sigma2_cur - s2_min) / (s2_max - s2_min));

    // --- Build covariance matrix Sigma = K + ve*I (double version) ---
    std::vector<double> Sigma_d(N * N, 0.0);
    std::vector<double> K0(N * N, 0.0);  // K without nugget, for gradient
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = i; j < N; ++j) {
        double sq = 0.0;
        for (size_t d = 0; d < E; ++d) sq += phi_cur[d] * D[d][i * N + j];
        double k_ij = sigma2_cur * std::exp(-sq);
        K0[i * N + j] = k_ij;
        K0[j * N + i] = k_ij;
        Sigma_d[i * N + j] = k_ij;
        Sigma_d[j * N + i] = k_ij;
      }
      Sigma_d[i * N + i] += ve_cur;
    }

    // --- Solve Sigma * alpha = y via Gaussian elimination ---
    std::vector<double> alpha = y;
    std::vector<double> Sigma_copy = Sigma_d;
    GaussianElimination<double>(Sigma_copy, alpha, N);
    // alpha = Sigma^{-1} y

    // --- Log likelihood: -0.5 y'alpha - 0.5 log|Sigma| ---
    // Recompute Sigma to get its inverse (solve N identity columns)
    std::vector<double> iKVs(N * N, 0.0);
    for (size_t col = 0; col < N; ++col) {
      std::vector<double> e_col(N, 0.0);
      e_col[col] = 1.0;
      std::vector<double> Sigma_tmp = Sigma_d;
      GaussianElimination<double>(Sigma_tmp, e_col, N);
      for (size_t row = 0; row < N; ++row) iKVs[row * N + col] = e_col[row];
    }

    // Log determinant via triangular factor diagonal
    // (approximate: sum log|diag| of upper triangle from elimination)
    // We use: log|Sigma| = sum_i log(U[i,i]) from GE
    // Recompute to get diagonal of U
    double logdet = 0.0;
    {
      std::vector<double> S2 = Sigma_d;
      std::vector<double> b2(N, 0.0);
      b2[0] = 1.0;  // dummy rhs
      // forward elimination only to get U diagonal
      for (size_t col = 0; col < N; ++col) {
        // find max pivot
        size_t prow = col;
        double mval = 0.0;
        for (size_t r = col; r < N; ++r) {
          double v = std::abs(S2[r * N + col]);
          if (v > mval) { mval = v; prow = r; }
        }
        if (prow != col) {
          for (size_t j = 0; j < N; ++j) std::swap(S2[col * N + j], S2[prow * N + j]);
        }
        logdet += std::log(std::abs(S2[col * N + col]) + 1e-300);
        for (size_t r = col + 1; r < N; ++r) {
          double f = S2[r * N + col] / S2[col * N + col];
          for (size_t j = col; j < N; ++j) S2[r * N + j] -= f * S2[col * N + j];
        }
      }
    }

    double ytAlpha = 0.0;
    for (size_t i = 0; i < N; ++i) ytAlpha += y[i] * alpha[i];
    double like = -0.5 * ytAlpha - logdet;

    // --- Priors (matching GPEDM R package: Rogers 2023, getpriors()) ---
    // phi: half-Normal prior
    // lam_phi = (2^(modeprior-1))^2 * pi/2
    // Default modeprior=0: lam = (2^(-1))^2 * pi/2 = 0.25 * pi/2 = pi/8
    const double kLamPhi = 0.25 * M_PI / 2.0;  // = pi/8
    double lp_phi = 0.0;
    std::vector<double> dlp_phi(E, 0.0);
    for (size_t d = 0; d < E; ++d) {
      lp_phi     += -0.5 * phi_cur[d] * phi_cur[d] / kLamPhi;
      dlp_phi[d]  = -phi_cur[d] / kLamPhi;
    }
    // sigma2 and ve: Beta-shaped (a=2, b=2) on (0, max)
    // lp = (a-1)*log(x/xmax) + (b-1)*log(1 - x/xmax)  with a=b=2 → (a-1)=1
    double lp_ve = std::log(ve_cur / ve_max) + std::log(1.0 - ve_cur / ve_max);
    double lp_s2 = std::log(sigma2_cur / s2_max) + std::log(1.0 - sigma2_cur / s2_max);
    // dlp = (a-1)/x - (b-1)/(xmax-x)  with a=b=2 → (a-1)=1
    double dlp_ve = 1.0 / ve_cur - 1.0 / (ve_max - ve_cur);
    double dlp_s2 = 1.0 / sigma2_cur - 1.0 / (s2_max - sigma2_cur);

    double lp = lp_phi + lp_ve + lp_s2;
    double nllpost = -(like + lp);

    // --- Gradient of log posterior ---
    // vQ[i,j] = 0.5 * (alpha[i]*alpha[j] - iKVs[i,j])
    // dl[d]    = 0.5 * sum_{i,j} vQ[i,j] * dSigma[i,j]/dphi[d]
    //          = 0.5 * sum_{i,j} vQ[i,j] * (-D[d][i,j] * K0[i,j])
    std::vector<double> grad_unconstrained(np, 0.0);

    for (size_t d = 0; d < E; ++d) {
      double dl_d = 0.0;
      for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
          double vQ_ij = 0.5 * (alpha[i] * alpha[j] - iKVs[i * N + j]);
          dl_d += vQ_ij * (-D[d][i * N + j] * K0[i * N + j]);
        }
      }
      grad_unconstrained[d] = dl_d + dlp_phi[d];
    }

    // ve gradient: dSigma/dve = I
    {
      double dl_ve = 0.0;
      for (size_t i = 0; i < N; ++i) {
        dl_ve += 0.5 * (alpha[i] * alpha[i] - iKVs[i * N + i]);
      }
      grad_unconstrained[E] = dl_ve + dlp_ve;
    }

    // sigma2 gradient: dSigma/dsigma2 = K0 / sigma2
    {
      double dl_s2 = 0.0;
      for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
          double vQ_ij = 0.5 * (alpha[i] * alpha[j] - iKVs[i * N + j]);
          dl_s2 += vQ_ij * (K0[i * N + j] / (sigma2_cur + 1e-300));
        }
      }
      grad_unconstrained[E + 1] = dl_s2 + dlp_s2;
    }

    // Chain-rule through parameter transforms: grad_parst = -J * dpars
    LogPosteriorResult result;
    result.nllpost = nllpost;
    result.grad.resize(np);
    for (size_t k = 0; k < np; ++k) {
      result.grad[k] = -grad_unconstrained[k] * dpars[k];
    }
    return result;
  }
};

}  // namespace fims_edm

#endif  // FIMS_EDM_GP_EDM_PROJECTION_HPP
