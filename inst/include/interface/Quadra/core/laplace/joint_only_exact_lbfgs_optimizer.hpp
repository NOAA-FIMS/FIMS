#pragma once

#include <cmath>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

#include "joint_only_exact_gradient.hpp"
#include "laplace_lbfgs_optimizer.hpp"

namespace quadra {

struct JointOnlyExactLBFGSOptions {
  JointOnlyExactGradientOptions gradient_m;

  int max_iterations_m = 100;
  int memory_m = 7;

  double gradient_tolerance_m = 1e-6;
  double step_tolerance_m = 1e-10;

  double initial_step_scale_m = 1.0;
  double min_step_scale_m = 1e-10;
  double sufficient_decrease_m = 1e-4;

  bool use_backtracking_m = true;
  bool warm_start_random_m = true;
};

struct JointOnlyExactLBFGSResult {
  std::vector<double> theta_initial_m;
  std::vector<double> theta_hat_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_hat_m;
  std::vector<double> gradient_fixed_m;

  double joint_objective_m = 0.0;
  double gradient_norm_m = 0.0;
  double step_norm_m = 0.0;

  int iterations_m = 0;
  bool converged_m = false;

  std::string message_m;

  JointOnlyExactGradientResult gradient_result_m;
  std::vector<LaplaceLBFGSIteration> history_m;
};

template <class Model>
inline JointOnlyExactLBFGSResult optimize_joint_only_exact_lbfgs(
    Model &model, const std::vector<double> &theta_initial,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const JointOnlyExactLBFGSOptions &options = JointOnlyExactLBFGSOptions()) {
  JointOnlyExactLBFGSResult result;
  result.theta_initial_m = theta_initial;

  std::vector<double> theta = theta_initial;
  std::vector<double> random_start = random_initial;

  JointOnlyExactGradientResult current = evaluate_joint_only_exact_gradient(
      model, theta, random_start, partition, options.gradient_m);

  if (!current.converged_m || !lbfgs_all_finite(current.gradient_fixed_m) ||
      !std::isfinite(current.joint_objective_m)) {
    result.message_m =
        "Failed: initial joint-only exact gradient evaluation failed.";
    result.gradient_result_m = current;
    return result;
  }

  if (options.warm_start_random_m) {
    random_start = current.u_hat_m;
  }

  std::deque<std::vector<double>> s_history;
  std::deque<std::vector<double>> y_history;

  for (int iter = 0; iter < options.max_iterations_m; ++iter) {
    LaplaceLBFGSIteration hist;
    hist.iteration_m = iter;
    hist.objective_m = current.joint_objective_m;
    hist.gradient_norm_m = current.gradient_norm_m;
    result.history_m.push_back(hist);

    result.iterations_m = iter;

    if (current.gradient_norm_m <= options.gradient_tolerance_m) {
      result.converged_m = true;
      result.message_m =
          "Converged: joint-only exact gradient norm below tolerance.";
      break;
    }

    std::vector<double> direction = lbfgs_two_loop_direction(
        current.gradient_fixed_m, s_history, y_history);

    const double descent = lbfgs_dot(current.gradient_fixed_m, direction);
    if (!(descent < 0.0) || !std::isfinite(descent)) {
      s_history.clear();
      y_history.clear();
      direction = current.gradient_fixed_m;

      for (double &v : direction) {
        v = -v;
      }
    }

    double alpha = options.initial_step_scale_m;
    bool accepted = false;

    std::vector<double> candidate_theta;
    JointOnlyExactGradientResult candidate;

    while (alpha >= options.min_step_scale_m) {
      candidate_theta = lbfgs_add_scaled(theta, direction, alpha);

      const std::vector<double> candidate_random_start =
          options.warm_start_random_m ? current.u_hat_m : random_initial;

      candidate = evaluate_joint_only_exact_gradient(
          model, candidate_theta, candidate_random_start, partition,
          options.gradient_m);

      if (!candidate.converged_m ||
          !std::isfinite(candidate.joint_objective_m) ||
          !lbfgs_all_finite(candidate.gradient_fixed_m)) {
        alpha *= 0.5;
        continue;
      }

      const double armijo_rhs =
          current.joint_objective_m +
          options.sufficient_decrease_m * alpha *
              lbfgs_dot(current.gradient_fixed_m, direction);

      if (!options.use_backtracking_m ||
          candidate.joint_objective_m <= armijo_rhs) {
        accepted = true;
        break;
      }

      alpha *= 0.5;
    }

    if (!accepted) {
      result.converged_m = false;
      result.message_m = "Failed: joint-only exact LBFGS backtracking failed.";
      break;
    }

    std::vector<double> s_vec = lbfgs_subtract(candidate_theta, theta);
    std::vector<double> y_vec =
        lbfgs_subtract(candidate.gradient_fixed_m, current.gradient_fixed_m);

    const double sy = lbfgs_dot(s_vec, y_vec);

    if (sy > 1e-14 && std::isfinite(sy)) {
      s_history.push_back(s_vec);
      y_history.push_back(y_vec);

      while (static_cast<int>(s_history.size()) > options.memory_m) {
        s_history.pop_front();
        y_history.pop_front();
      }
    }

    result.step_norm_m = lbfgs_norm(s_vec);

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
      result.message_m =
          "Converged: joint-only exact step norm below tolerance.";
      break;
    }

    if (current.gradient_norm_m <= options.gradient_tolerance_m) {
      result.converged_m = true;
      result.message_m =
          "Converged: joint-only exact gradient norm below tolerance.";
      break;
    }
  }

  if (!result.converged_m && result.message_m.empty()) {
    result.message_m =
        "Stopped: maximum joint-only exact LBFGS iterations reached.";
  }

  result.theta_hat_m = theta;
  result.u_hat_m = current.u_hat_m;
  result.full_hat_m = current.full_m;
  result.gradient_fixed_m = current.gradient_fixed_m;
  result.joint_objective_m = current.joint_objective_m;
  result.gradient_norm_m = current.gradient_norm_m;
  result.gradient_result_m = current;

  return result;
}

template <class Model>
inline JointOnlyExactLBFGSResult optimize_joint_only_exact_lbfgs(
    Model &model, const std::vector<double> &theta_initial,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const JointOnlyExactLBFGSOptions &options = JointOnlyExactLBFGSOptions()) {
  return optimize_joint_only_exact_lbfgs(model, theta_initial, random_initial,
                                         partition_parameters(parameters),
                                         options);
}

} // namespace quadra
