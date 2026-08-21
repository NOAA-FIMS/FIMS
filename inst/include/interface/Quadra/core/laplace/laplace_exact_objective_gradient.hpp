#pragma once

#include <chrono>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "laplace_objective.hpp"

namespace quadra {

struct LaplaceExactObjectiveGradientOptions {
  LaplaceObjectiveOptions objective_m;

  // Current implementation includes the exact envelope gradient of
  // f(theta, u_hat). It does not include the derivative of
  // 0.5 * logdet(H_uu).
  bool include_logdet_derivative_m = false;
};

inline double exact_objective_gradient_norm2(const std::vector<double> &x) {
  double out = 0.0;

  for (double xi : x) {
    out += xi * xi;
  }

  return std::sqrt(out);
}

struct LaplaceExactObjectiveGradientResult {
  std::vector<double> fixed_m;

  double objective_ms_m = 0.0;
  double tape_setup_ms_m = 0.0;
  double reverse_pass_ms_m = 0.0;
  double gradient_extract_ms_m = 0.0;
  double total_gradient_ms_m = 0.0;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;

  std::vector<double> gradient_fixed_m;
  std::vector<double> gradient_full_m;

  double joint_objective_m = 0.0;
  double log_det_hessian_m = 0.0;
  double laplace_objective_m = 0.0;
  double gradient_norm_m = 0.0;

  bool converged_m = false;
  bool logdet_ok_m = false;
  bool includes_logdet_derivative_m = false;

  std::string message_m;

  Eigen::SparseMatrix<double> hessian_random_m;
  std::vector<double> gradient_random_m;
  std::vector<ReportValue> reports_m;
};

inline std::vector<double>
extract_fixed_gradient_from_full_og(const std::vector<double> &gradient_full,
                                    const ParameterPartition &partition) {
  return extract_by_indices(gradient_full, partition.fixed_indices_m);
}

// Combined Laplace objective + exact envelope-gradient evaluator.
//
// This performs the expensive Laplace objective path once, then evaluates
// the joint gradient at (theta, u_hat) to extract the envelope gradient wrt
// theta.
//
// It is intentionally a combined result object so outer optimizers can avoid
// juggling separate objective and exact-gradient result types.
template <class Model>
inline LaplaceExactObjectiveGradientResult
evaluate_laplace_exact_objective_gradient(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const LaplaceExactObjectiveGradientOptions &options =
        LaplaceExactObjectiveGradientOptions()) {
  if (fixed.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument("evaluate_laplace_exact_objective_gradient: "
                                "fixed vector has incorrect length");
  }

  if (random_initial.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument("evaluate_laplace_exact_objective_gradient: "
                                "random_initial has incorrect length");
  }

  if (options.include_logdet_derivative_m) {
    throw std::invalid_argument("evaluate_laplace_exact_objective_gradient: "
                                "logdet derivative is not implemented yet");
  }

  const auto objective_start = std::chrono::steady_clock::now();

  LaplaceObjectiveResult objective = evaluate_laplace_objective(
      model, fixed, random_initial, partition, options.objective_m);

  const auto objective_end = std::chrono::steady_clock::now();

  LaplaceExactObjectiveGradientResult result;
  result.fixed_m = fixed;
  result.u_hat_m = objective.u_hat_m;
  result.full_m = objective.full_m;
  result.joint_objective_m = objective.joint_objective_m;
  result.log_det_hessian_m = objective.log_det_hessian_m;
  result.laplace_objective_m = objective.laplace_objective_m;
  result.converged_m = objective.converged_m;
  result.logdet_ok_m = objective.logdet_ok_m;
  result.message_m = objective.message_m;
  result.hessian_random_m = objective.hessian_random_m;
  result.gradient_random_m = objective.gradient_random_m;
  result.reports_m = objective.reports_m;
  result.includes_logdet_derivative_m = false;

  result.objective_ms_m =
      std::chrono::duration<double, std::milli>(objective_end - objective_start)
          .count();

  if (!objective.converged_m || !objective.logdet_ok_m) {
    result.gradient_fixed_m.assign(fixed.size(), std::nan(""));
    result.gradient_full_m.assign(objective.full_m.size(), std::nan(""));
    result.gradient_norm_m = std::nan("");
    return result;
  }

  const auto tape_start = std::chrono::steady_clock::now();

  TapeContext tape;
  ADScope scope(tape.graph);

  ModelReportContext ctx;
  model.initialize(ctx);

  std::vector<AD> full_ad = to_ad(objective.full_m);
  AD joint = model.template evaluate<AD>(full_ad, ctx);

  const auto tape_end = std::chrono::steady_clock::now();

  const auto reverse_start = std::chrono::steady_clock::now();

  scope.backward(joint);

  const auto reverse_end = std::chrono::steady_clock::now();

  const auto extract_start = std::chrono::steady_clock::now();

  Eigen::VectorXd g_full = extract_gradient(full_ad);

  const auto extract_end = std::chrono::steady_clock::now();

  result.gradient_full_m.resize(static_cast<size_t>(g_full.size()));
  for (int i = 0; i < g_full.size(); ++i) {
    result.gradient_full_m[static_cast<size_t>(i)] = g_full[i];
  }

  result.gradient_fixed_m =
      extract_fixed_gradient_from_full_og(result.gradient_full_m, partition);

  result.gradient_norm_m =
      exact_objective_gradient_norm2(result.gradient_fixed_m);

  result.tape_setup_ms_m =
      std::chrono::duration<double, std::milli>(tape_end - tape_start).count();

  result.reverse_pass_ms_m =
      std::chrono::duration<double, std::milli>(reverse_end - reverse_start)
          .count();

  result.gradient_extract_ms_m =
      std::chrono::duration<double, std::milli>(extract_end - extract_start)
          .count();

  result.total_gradient_ms_m = result.tape_setup_ms_m +
                               result.reverse_pass_ms_m +
                               result.gradient_extract_ms_m;

  return result;
}

template <class Model>
inline LaplaceExactObjectiveGradientResult
evaluate_laplace_exact_objective_gradient(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const LaplaceExactObjectiveGradientOptions &options =
        LaplaceExactObjectiveGradientOptions()) {
  return evaluate_laplace_exact_objective_gradient(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra
