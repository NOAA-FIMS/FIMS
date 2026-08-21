#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "laplace_fixed_gradient.hpp"
#include "laplace_objective.hpp"

namespace quadra {

struct LaplaceExactGradientOptions {
  LaplaceObjectiveOptions objective_m;

  // This first exact-gradient layer computes the exact envelope-theorem
  // gradient of f(theta, u_hat) wrt theta.
  //
  // If include_logdet_derivative_m is false, the derivative of
  // 0.5 * logdet(H_uu) is omitted. This is exact for models where H_uu
  // is constant wrt theta and is a validated stepping stone toward the
  // full trace/Hessian-derivative term.
  bool include_logdet_derivative_m = false;
};

struct LaplaceExactGradientResult {
  std::vector<double> fixed_m;
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

  LaplaceObjectiveResult objective_result_m;
};

inline std::vector<double>
extract_fixed_gradient_from_full(const std::vector<double> &gradient_full,
                                 const ParameterPartition &partition) {
  return extract_by_indices(gradient_full, partition.fixed_indices_m);
}

// Compute the exact envelope gradient of the optimized joint objective:
//
//   d/dtheta f(theta, u_hat(theta)) = partial f / partial theta
//
// because partial f / partial u = 0 at u_hat.
//
// This is the mathematically exact joint-objective part of the Laplace
// gradient. The full Laplace gradient additionally requires:
//
//   0.5 * d/dtheta logdet(H_uu)
//
// which needs third-order information. That term is intentionally left as
// a separate future extension unless include_logdet_derivative_m is enabled
// by a later implementation.
template <class Model>
inline LaplaceExactGradientResult
evaluate_laplace_exact_gradient(Model &model, const std::vector<double> &fixed,
                                const std::vector<double> &random_initial,
                                const ParameterPartition &partition,
                                const LaplaceExactGradientOptions &options =
                                    LaplaceExactGradientOptions()) {
  if (fixed.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument(
        "evaluate_laplace_exact_gradient: fixed vector has incorrect length");
  }

  if (random_initial.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument(
        "evaluate_laplace_exact_gradient: random_initial has incorrect length");
  }

  if (options.include_logdet_derivative_m) {
    throw std::invalid_argument("evaluate_laplace_exact_gradient: logdet "
                                "derivative is not implemented yet");
  }

  LaplaceObjectiveResult objective = evaluate_laplace_objective(
      model, fixed, random_initial, partition, options.objective_m);

  LaplaceExactGradientResult result;
  result.fixed_m = fixed;
  result.u_hat_m = objective.u_hat_m;
  result.full_m = objective.full_m;
  result.joint_objective_m = objective.joint_objective_m;
  result.log_det_hessian_m = objective.log_det_hessian_m;
  result.laplace_objective_m = objective.laplace_objective_m;
  result.converged_m = objective.converged_m;
  result.logdet_ok_m = objective.logdet_ok_m;
  result.message_m = objective.message_m;
  result.objective_result_m = objective;
  result.includes_logdet_derivative_m = false;

  if (!objective.converged_m || !objective.logdet_ok_m) {
    result.gradient_fixed_m.assign(fixed.size(), std::nan(""));
    result.gradient_full_m.assign(objective.full_m.size(), std::nan(""));
    result.gradient_norm_m = std::nan("");
    return result;
  }

  TapeContext tape;
  ADScope scope(tape.graph);

  ModelReportContext ctx;
  model.initialize(ctx);

  std::vector<AD> full_ad = to_ad(objective.full_m);
  AD joint = model.template evaluate<AD>(full_ad, ctx);

  scope.backward(joint);
  Eigen::VectorXd g_full = extract_gradient(full_ad);

  result.gradient_full_m.resize(static_cast<size_t>(g_full.size()));
  for (int i = 0; i < g_full.size(); ++i) {
    result.gradient_full_m[static_cast<size_t>(i)] = g_full[i];
  }

  result.gradient_fixed_m =
      extract_fixed_gradient_from_full(result.gradient_full_m, partition);

  result.gradient_norm_m = norm2(result.gradient_fixed_m);

  return result;
}

template <class Model>
inline LaplaceExactGradientResult
evaluate_laplace_exact_gradient(Model &model, const std::vector<double> &fixed,
                                const std::vector<double> &random_initial,
                                const ParameterSet &parameters,
                                const LaplaceExactGradientOptions &options =
                                    LaplaceExactGradientOptions()) {
  return evaluate_laplace_exact_gradient(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra
