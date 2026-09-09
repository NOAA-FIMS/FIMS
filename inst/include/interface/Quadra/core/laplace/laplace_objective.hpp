#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <Eigen/SparseCholesky>

#include "laplace_backend_factory.hpp"
#include "persistent_structured_runtime.hpp"
#include "random_effect_newton.hpp"

namespace quadra {

struct LaplaceObjectiveOptions {
  RandomEffectNewtonOptions newton_m;
  bool include_constant_m = true;
  bool validate_reusable_tape_m = true;
  bool collect_reports_m = true;
  bool compute_mixed_derivatives_m = true;
  double logdet_jitter_m = 0.0;
};

struct LaplaceObjectiveResult {
  std::vector<double> fixed_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;

  double joint_objective_m = 0.0;
  double log_det_hessian_m = 0.0;
  double laplace_objective_m = 0.0;
  double gradient_norm_random_m = 0.0;
  double random_step_norm_m = 0.0;

  int n_random_m = 0;
  int newton_iterations_m = 0;

  bool converged_m = false;
  bool logdet_ok_m = false;

  std::string message_m;

  Eigen::SparseMatrix<double> hessian_random_m;
  std::vector<double> gradient_random_m;
  std::vector<double> gradient_fixed_joint_m;
  Eigen::MatrixXd mixed_hessian_m;
  std::vector<ReportValue> reports_m;
  laplace::BackendRecommendation backend_m;
  bool structure_detected_m = false;
  bool tape_rebuilt_m = false;
};

inline Eigen::SparseMatrix<double>
laplace_objective_add_diagonal_jitter(const Eigen::SparseMatrix<double> &H,
                                      double jitter) {
  if (jitter == 0.0) {
    return H;
  }

  Eigen::SparseMatrix<double> out = H;
  out.makeCompressed();

  const int n = static_cast<int>(out.rows());

  for (int i = 0; i < n; ++i) {
    out.coeffRef(i, i) += jitter;
  }

  out.makeCompressed();
  return out;
}

inline double sparse_ldlt_logdet(const Eigen::SparseMatrix<double> &H,
                                 bool &ok) {
  ok = false;

  if (H.rows() != H.cols()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  try {
    auto backend = laplace::CreateLaplaceBackendForHessian(H);
    backend->factorize(H);
    ok = backend->is_spd() && std::isfinite(backend->logdet());
    return ok ? backend->logdet() : std::numeric_limits<double>::quiet_NaN();
  } catch (...) {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Evaluate the Laplace approximation:
//
//   f(theta, u_hat) + 0.5 logdet(H_uu) - 0.5 n_u log(2*pi)
//
// where u_hat minimizes f(theta, u) for fixed theta.
//
// This function assumes f is a negative log joint density up to constants.
// If constants are omitted from f, set include_constant_m = false when you
// want to keep the same constant-free convention.
template <class Model>
inline LaplaceObjectiveResult evaluate_laplace_objective(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const LaplaceObjectiveOptions &options = LaplaceObjectiveOptions()) {
  if (partition.random_indices_m.empty()) {
    throw std::invalid_argument(
        "evaluate_laplace_objective: partition has no random effects");
  }

  RandomEffectNewtonResult newton = optimize_random_effects_newton(
      model, fixed, random_initial, partition, options.newton_m);

  LaplaceObjectiveResult result;
  result.fixed_m = fixed;
  result.u_hat_m = newton.u_hat_m;
  result.full_m = newton.full_m;
  result.joint_objective_m = newton.objective_value_m;
  result.gradient_norm_random_m = newton.gradient_norm_m;
  result.random_step_norm_m = newton.step_norm_m;
  result.newton_iterations_m = newton.iterations_m;
  result.converged_m = newton.converged_m;
  result.message_m = newton.message_m;
  result.hessian_random_m = newton.hessian_random_m;
  result.gradient_random_m = newton.gradient_random_m;
  result.gradient_fixed_joint_m = newton.gradient_fixed_m;
  result.mixed_hessian_m = newton.mixed_hessian_m;
  result.reports_m = newton.reports_m;
  result.n_random_m = static_cast<int>(partition.random_indices_m.size());
  result.backend_m = newton.backend_m;

  Eigen::SparseMatrix<double> H_for_logdet =
      laplace_objective_add_diagonal_jitter(result.hessian_random_m,
                                            options.logdet_jitter_m);

  bool logdet_ok = false;
  double logdet = std::numeric_limits<double>::quiet_NaN();
  try {
    laplace::PersistentStructuredLaplaceRuntime structured_runtime;
    const auto structured = structured_runtime.evaluate(H_for_logdet);
    logdet = structured.logdet;
    result.backend_m = structured.recommendation;
    result.structure_detected_m = structured.detected_structure;
    logdet_ok = std::isfinite(logdet);
  } catch (...) {
    logdet = sparse_ldlt_logdet(H_for_logdet, logdet_ok);
  }

  result.logdet_ok_m = logdet_ok;
  result.log_det_hessian_m = logdet;

  if (!logdet_ok) {
    result.laplace_objective_m = std::numeric_limits<double>::quiet_NaN();

    if (result.message_m.empty()) {
      result.message_m = "Failed: log determinant computation failed.";
    } else {
      result.message_m += " Log determinant computation failed.";
    }

    return result;
  }

  result.laplace_objective_m =
      result.joint_objective_m + 0.5 * result.log_det_hessian_m;

  if (options.include_constant_m) {
    result.laplace_objective_m -=
        0.5 * static_cast<double>(result.n_random_m) * std::log(2.0 * M_PI);
  }

  return result;
}

template <class Model>
inline LaplaceObjectiveResult evaluate_laplace_objective(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const LaplaceObjectiveOptions &options = LaplaceObjectiveOptions()) {
  return evaluate_laplace_objective(model, fixed, random_initial,
                                    partition_parameters(parameters), options);
}

} // namespace quadra
