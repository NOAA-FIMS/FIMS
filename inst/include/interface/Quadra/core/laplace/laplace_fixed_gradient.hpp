#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "laplace_objective.hpp"

namespace quadra {

struct LaplaceFixedGradientOptions {
  LaplaceObjectiveOptions objective_m;
  double relative_step_m = 1e-5;
  double absolute_step_m = 1e-6;
  bool use_central_difference_m = true;
  bool warm_start_random_m = true;
};

struct LaplaceFixedGradientResult {
  std::vector<double> fixed_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;
  std::vector<double> gradient_fixed_m;

  double laplace_objective_m = 0.0;
  double gradient_norm_m = 0.0;

  bool converged_m = false;
  bool logdet_ok_m = false;

  std::string message_m;

  LaplaceObjectiveResult objective_result_m;
};

inline double finite_difference_step(double x, double relative_step,
                                     double absolute_step) {
  return absolute_step + relative_step * std::max(1.0, std::abs(x));
}

inline double norm2(const std::vector<double> &x) {
  double out = 0.0;

  for (double xi : x) {
    out += xi * xi;
  }

  return std::sqrt(out);
}

// Finite-difference gradient of the Laplace marginal objective wrt fixed
// effects theta.
//
// This is intentionally not the final exact implicit/logdet derivative.
// It provides an optimizer-ready API and validates the fixed-effect marginal
// objective pathway before adding exact derivatives.
template <class Model>
inline LaplaceFixedGradientResult
evaluate_laplace_fixed_gradient(Model &model, const std::vector<double> &fixed,
                                const std::vector<double> &random_initial,
                                const ParameterPartition &partition,
                                const LaplaceFixedGradientOptions &options =
                                    LaplaceFixedGradientOptions()) {
  if (fixed.empty()) {
    throw std::invalid_argument(
        "evaluate_laplace_fixed_gradient: fixed vector cannot be empty");
  }

  LaplaceFixedGradientResult result;
  result.fixed_m = fixed;

  LaplaceObjectiveResult center = evaluate_laplace_objective(
      model, fixed, random_initial, partition, options.objective_m);

  result.objective_result_m = center;
  result.u_hat_m = center.u_hat_m;
  result.full_m = center.full_m;
  result.laplace_objective_m = center.laplace_objective_m;
  result.converged_m = center.converged_m;
  result.logdet_ok_m = center.logdet_ok_m;
  result.message_m = center.message_m;

  if (!center.converged_m || !center.logdet_ok_m) {
    result.gradient_fixed_m.assign(fixed.size(), std::nan(""));
    result.gradient_norm_m = std::nan("");
    return result;
  }

  result.gradient_fixed_m.assign(fixed.size(), 0.0);

  for (size_t i = 0; i < fixed.size(); ++i) {
    const double h = finite_difference_step(fixed[i], options.relative_step_m,
                                            options.absolute_step_m);

    std::vector<double> fixed_plus = fixed;
    fixed_plus[i] += h;

    std::vector<double> u_start_plus =
        options.warm_start_random_m ? center.u_hat_m : random_initial;

    LaplaceObjectiveResult plus = evaluate_laplace_objective(
        model, fixed_plus, u_start_plus, partition, options.objective_m);

    if (!plus.converged_m || !plus.logdet_ok_m) {
      result.gradient_fixed_m[i] = std::nan("");
      continue;
    }

    if (options.use_central_difference_m) {
      std::vector<double> fixed_minus = fixed;
      fixed_minus[i] -= h;

      std::vector<double> u_start_minus =
          options.warm_start_random_m ? center.u_hat_m : random_initial;

      LaplaceObjectiveResult minus = evaluate_laplace_objective(
          model, fixed_minus, u_start_minus, partition, options.objective_m);

      if (!minus.converged_m || !minus.logdet_ok_m) {
        result.gradient_fixed_m[i] = std::nan("");
        continue;
      }

      result.gradient_fixed_m[i] =
          (plus.laplace_objective_m - minus.laplace_objective_m) / (2.0 * h);
    } else {
      result.gradient_fixed_m[i] =
          (plus.laplace_objective_m - center.laplace_objective_m) / h;
    }
  }

  result.gradient_norm_m = norm2(result.gradient_fixed_m);

  return result;
}

template <class Model>
inline LaplaceFixedGradientResult
evaluate_laplace_fixed_gradient(Model &model, const std::vector<double> &fixed,
                                const std::vector<double> &random_initial,
                                const ParameterSet &parameters,
                                const LaplaceFixedGradientOptions &options =
                                    LaplaceFixedGradientOptions()) {
  return evaluate_laplace_fixed_gradient(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra
