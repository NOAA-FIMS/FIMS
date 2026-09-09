#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "laplace_fixed_gradient.hpp"

namespace quadra {

struct LaplaceOptimizerOptions {
  LaplaceFixedGradientOptions gradient_m;

  int max_iterations_m = 100;
  double gradient_tolerance_m = 1e-6;
  double step_tolerance_m = 1e-10;

  double initial_step_scale_m = 1.0;
  double min_step_scale_m = 1e-10;
  double sufficient_decrease_m = 1e-4;

  bool use_backtracking_m = true;
  bool warm_start_random_m = true;
  bool verbose_m = false;
};

struct LaplaceOptimizerIteration {
  int iteration_m = 0;
  double objective_m = 0.0;
  double gradient_norm_m = 0.0;
  double step_scale_m = 0.0;
  double step_norm_m = 0.0;
};

struct LaplaceOptimizerResult {
  std::vector<double> theta_initial_m;
  std::vector<double> theta_hat_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_hat_m;
  std::vector<double> gradient_fixed_m;

  double laplace_objective_m = 0.0;
  double gradient_norm_m = 0.0;
  double step_norm_m = 0.0;

  int iterations_m = 0;
  bool converged_m = false;
  bool logdet_ok_m = false;

  std::string message_m;

  LaplaceFixedGradientResult gradient_result_m;
  std::vector<LaplaceOptimizerIteration> history_m;
};

inline std::vector<double>
gradient_descent_step(const std::vector<double> &theta,
                      const std::vector<double> &gradient, double step_scale) {
  if (theta.size() != gradient.size()) {
    throw std::invalid_argument(
        "gradient_descent_step: vector length mismatch");
  }

  std::vector<double> out = theta;

  for (size_t i = 0; i < theta.size(); ++i) {
    out[i] -= step_scale * gradient[i];
  }

  return out;
}

inline double squared_norm(const std::vector<double> &x) {
  double out = 0.0;

  for (double xi : x) {
    out += xi * xi;
  }

  return out;
}

inline bool all_finite(const std::vector<double> &x) {
  for (double xi : x) {
    if (!std::isfinite(xi)) {
      return false;
    }
  }

  return true;
}

// Optimize fixed effects theta over the Laplace marginal objective.
//
// This first implementation intentionally uses gradient descent with Armijo
// backtracking. It is simple, transparent, and validates the full outer-loop
// plumbing before replacing the step strategy with LBFGS.
template <class Model>
inline LaplaceOptimizerResult optimize_laplace_fixed_effects(
    Model &model, const std::vector<double> &theta_initial,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const LaplaceOptimizerOptions &options = LaplaceOptimizerOptions()) {
  if (theta_initial.empty()) {
    throw std::invalid_argument(
        "optimize_laplace_fixed_effects: theta_initial cannot be empty");
  }

  if (theta_initial.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument(
        "optimize_laplace_fixed_effects: theta_initial has incorrect length");
  }

  if (random_initial.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument(
        "optimize_laplace_fixed_effects: random_initial has incorrect length");
  }

  LaplaceOptimizerResult result;
  result.theta_initial_m = theta_initial;

  std::vector<double> theta = theta_initial;
  std::vector<double> random_start = random_initial;

  LaplaceFixedGradientResult current = evaluate_laplace_fixed_gradient(
      model, theta, random_start, partition, options.gradient_m);

  if (!current.converged_m || !current.logdet_ok_m ||
      !all_finite(current.gradient_fixed_m) ||
      !std::isfinite(current.laplace_objective_m)) {
    result.converged_m = false;
    result.message_m =
        "Failed: initial Laplace objective/gradient evaluation failed.";
    result.gradient_result_m = current;
    return result;
  }

  if (options.warm_start_random_m) {
    random_start = current.u_hat_m;
  }

  for (int iter = 0; iter < options.max_iterations_m; ++iter) {
    LaplaceOptimizerIteration hist;
    hist.iteration_m = iter;
    hist.objective_m = current.laplace_objective_m;
    hist.gradient_norm_m = current.gradient_norm_m;
    result.history_m.push_back(hist);

    result.iterations_m = iter;
    result.step_norm_m = 0.0;

    if (current.gradient_norm_m <= options.gradient_tolerance_m) {
      result.converged_m = true;
      result.message_m =
          "Converged: fixed-effect gradient norm below tolerance.";
      break;
    }

    const double grad_sq = squared_norm(current.gradient_fixed_m);

    if (!(grad_sq > 0.0) || !std::isfinite(grad_sq)) {
      result.converged_m = false;
      result.message_m = "Failed: non-finite or zero gradient norm.";
      break;
    }

    double alpha = options.initial_step_scale_m;
    bool accepted = false;

    std::vector<double> candidate_theta;
    LaplaceFixedGradientResult candidate;

    while (alpha >= options.min_step_scale_m) {
      candidate_theta =
          gradient_descent_step(theta, current.gradient_fixed_m, alpha);

      const std::vector<double> candidate_random_start =
          options.warm_start_random_m ? current.u_hat_m : random_initial;

      candidate = evaluate_laplace_fixed_gradient(
          model, candidate_theta, candidate_random_start, partition,
          options.gradient_m);

      if (!candidate.converged_m || !candidate.logdet_ok_m ||
          !std::isfinite(candidate.laplace_objective_m) ||
          !all_finite(candidate.gradient_fixed_m)) {
        alpha *= 0.5;
        continue;
      }

      const double armijo_rhs = current.laplace_objective_m -
                                options.sufficient_decrease_m * alpha * grad_sq;

      if (!options.use_backtracking_m ||
          candidate.laplace_objective_m <= armijo_rhs) {
        accepted = true;
        break;
      }

      alpha *= 0.5;
    }

    if (!accepted) {
      result.converged_m = false;
      result.message_m =
          "Failed: fixed-effect backtracking line search failed.";
      break;
    }

    result.step_norm_m = 0.0;
    for (size_t i = 0; i < theta.size(); ++i) {
      const double dx = candidate_theta[i] - theta[i];
      result.step_norm_m += dx * dx;
    }
    result.step_norm_m = std::sqrt(result.step_norm_m);

    if (!result.history_m.empty()) {
      result.history_m.back().step_scale_m = alpha;
      result.history_m.back().step_norm_m = result.step_norm_m;
    }

    theta = candidate_theta;
    current = candidate;

    if (options.warm_start_random_m) {
      random_start = current.u_hat_m;
    }

    result.iterations_m = iter + 1;

    if (result.step_norm_m <= options.step_tolerance_m) {
      result.converged_m = true;
      result.message_m = "Converged: fixed-effect step norm below tolerance.";
      break;
    }

    if (current.gradient_norm_m <= options.gradient_tolerance_m) {
      result.converged_m = true;
      result.message_m =
          "Converged: fixed-effect gradient norm below tolerance.";
      break;
    }
  }

  if (!result.converged_m && result.message_m.empty()) {
    result.message_m = "Stopped: maximum fixed-effect iterations reached.";
  }

  result.theta_hat_m = theta;
  result.u_hat_m = current.u_hat_m;
  result.full_hat_m = current.full_m;
  result.gradient_fixed_m = current.gradient_fixed_m;
  result.laplace_objective_m = current.laplace_objective_m;
  result.gradient_norm_m = current.gradient_norm_m;
  result.logdet_ok_m = current.logdet_ok_m;
  result.gradient_result_m = current;

  return result;
}

template <class Model>
inline LaplaceOptimizerResult optimize_laplace_fixed_effects(
    Model &model, const std::vector<double> &theta_initial,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const LaplaceOptimizerOptions &options = LaplaceOptimizerOptions()) {
  return optimize_laplace_fixed_effects(model, theta_initial, random_initial,
                                        partition_parameters(parameters),
                                        options);
}

} // namespace quadra
