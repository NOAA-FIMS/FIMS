#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "random_effect_newton.hpp"

namespace quadra {

struct JointOnlyExactGradientOptions {
  RandomEffectNewtonOptions newton_m;
};

struct JointOnlyExactGradientResult {
  std::vector<double> fixed_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;

  std::vector<double> gradient_fixed_m;
  std::vector<double> gradient_full_m;

  double joint_objective_m = 0.0;
  double gradient_norm_m = 0.0;

  bool converged_m = false;
  std::string message_m;

  RandomEffectNewtonResult newton_result_m;
};

inline double joint_only_gradient_norm(const std::vector<double> &x) {
  double out = 0.0;

  for (double xi : x) {
    out += xi * xi;
  }

  return std::sqrt(out);
}

// Joint-only exact envelope gradient.
//
// This intentionally skips Laplace logdet/Hessian reporting after the random
// effect Newton solve. It is not a full Laplace objective when logdet matters,
// but it is useful for benchmarking the upper bound of exact-envelope speed.
//
// It is exact for models where the logdet term is constant wrt fixed effects
// and can also be used as a fast profiling mode.
template <class Model>
inline JointOnlyExactGradientResult evaluate_joint_only_exact_gradient(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const JointOnlyExactGradientOptions &options =
        JointOnlyExactGradientOptions()) {
  JointOnlyExactGradientResult result;
  result.fixed_m = fixed;

  RandomEffectNewtonResult newton = optimize_random_effects_newton(
      model, fixed, random_initial, partition, options.newton_m);

  result.newton_result_m = newton;
  result.u_hat_m = newton.u_hat_m;
  result.full_m = newton.full_m;
  result.joint_objective_m = newton.objective_value_m;
  result.converged_m = newton.converged_m;
  result.message_m = newton.message_m;

  if (!newton.converged_m) {
    result.gradient_fixed_m.assign(fixed.size(), std::nan(""));
    result.gradient_full_m.assign(newton.full_m.size(), std::nan(""));
    result.gradient_norm_m = std::nan("");
    return result;
  }

  TapeContext tape;
  ADScope scope(tape.graph);

  ModelReportContext ctx;
  model.initialize(ctx);

  std::vector<AD> full_ad = to_ad(newton.full_m);
  AD joint = model.template evaluate<AD>(full_ad, ctx);

  scope.backward(joint);
  Eigen::VectorXd g_full = extract_gradient(full_ad);

  result.gradient_full_m.resize(static_cast<size_t>(g_full.size()));

  for (int i = 0; i < g_full.size(); ++i) {
    result.gradient_full_m[static_cast<size_t>(i)] = g_full[i];
  }

  result.gradient_fixed_m =
      extract_by_indices(result.gradient_full_m, partition.fixed_indices_m);

  result.gradient_norm_m = joint_only_gradient_norm(result.gradient_fixed_m);

  return result;
}

template <class Model>
inline JointOnlyExactGradientResult evaluate_joint_only_exact_gradient(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const JointOnlyExactGradientOptions &options =
        JointOnlyExactGradientOptions()) {
  return evaluate_joint_only_exact_gradient(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra
