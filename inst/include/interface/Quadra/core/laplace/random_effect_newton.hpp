#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <Eigen/SparseCholesky>

#include "random_effect_hessian.hpp"

namespace quadra {

struct RandomEffectNewtonOptions {
  int max_iterations_m = 25;
  double gradient_tolerance_m = 1e-8;
  double step_tolerance_m = 1e-10;
  double initial_step_scale_m = 1.0;
  double min_step_scale_m = 1e-8;
  double sufficient_decrease_m = 1e-4;
  double hessian_drop_tol_m = 0.0;
  bool use_backtracking_m = true;
};

struct RandomEffectNewtonResult {
  std::vector<double> fixed_m;
  std::vector<double> u_initial_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;
  std::vector<double> gradient_random_m;

  Eigen::SparseMatrix<double> hessian_random_m;

  double objective_value_m = 0.0;
  double gradient_norm_m = 0.0;
  double step_norm_m = 0.0;

  bool damping_was_used_m = false;
  int damping_retry_count_m = 0;
  double max_damping_lambda_m = 0.0;

  int iterations_m = 0;
  bool converged_m = false;
  std::string message_m;

  std::vector<ReportValue> reports_m;
};

inline double vector_norm(const std::vector<double> &x) {
  double out = 0.0;

  for (double xi : x) {
    out += xi * xi;
  }

  return std::sqrt(out);
}

inline std::vector<double> eigen_to_std_vector(const Eigen::VectorXd &x) {
  std::vector<double> out(static_cast<size_t>(x.size()));

  for (int i = 0; i < x.size(); ++i) {
    out[static_cast<size_t>(i)] = x[i];
  }

  return out;
}

inline Eigen::VectorXd std_vector_to_eigen(const std::vector<double> &x) {
  Eigen::VectorXd out(static_cast<int>(x.size()));

  for (size_t i = 0; i < x.size(); ++i) {
    out[static_cast<int>(i)] = x[i];
  }

  return out;
}

inline std::vector<double> add_scaled_step(const std::vector<double> &x,
                                           const Eigen::VectorXd &step,
                                           double scale) {
  if (x.size() != static_cast<size_t>(step.size())) {
    throw std::invalid_argument("add_scaled_step: vector length mismatch");
  }

  std::vector<double> out = x;

  for (size_t i = 0; i < x.size(); ++i) {
    out[i] += scale * step[static_cast<int>(i)];
  }

  return out;
}

// Optimize random effects u for fixed effects theta.
//
// This is the first Newton layer over the new partition/scalar-generic model
// API. It repeatedly evaluates gradient/Hessian wrt u, solves:
//
//   H step = -g
//
// and updates:
//
//   u <- u + alpha * step
//
// with optional conservative backtracking.
template <class Model>
inline RandomEffectNewtonResult optimize_random_effects_newton(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const RandomEffectNewtonOptions &options = RandomEffectNewtonOptions()) {
  if (partition.random_indices_m.empty()) {
    throw std::invalid_argument(
        "optimize_random_effects_newton: partition has no random effects");
  }

  if (random_initial.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument(
        "optimize_random_effects_newton: random_initial has incorrect length");
  }

  if (fixed.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument(
        "optimize_random_effects_newton: fixed has incorrect length");
  }

  std::vector<double> u = random_initial;

  RandomEffectHessianResult eval = evaluate_random_effect_hessian(
      model, fixed, u, partition, options.hessian_drop_tol_m);

  RandomEffectNewtonResult result;
  result.fixed_m = fixed;
  result.u_initial_m = random_initial;

  for (int iter = 0; iter < options.max_iterations_m; ++iter) {
    result.iterations_m = iter;

    if (eval.gradient_norm_m <= options.gradient_tolerance_m) {
      result.converged_m = true;
      result.message_m = "Converged: gradient norm below tolerance.";
      break;
    }

    Eigen::VectorXd g = std_vector_to_eigen(eval.gradient_random_m);

    Eigen::VectorXd step;
    bool solved_newton_system = false;

    double lambda = 0.0;
    const double lambda_initial = 1.0e-10;
    const double lambda_growth = 10.0;
    const int max_damping_attempts = 10;

    for (int damping_attempt = 0; damping_attempt <= max_damping_attempts;
         ++damping_attempt) {

      Eigen::SparseMatrix<double> H_damped = eval.hessian_random_m;

      if (lambda > 0.0) {
        H_damped.makeCompressed();
        for (int k = 0; k < H_damped.rows(); ++k) {
          H_damped.coeffRef(k, k) += lambda;
        }
        H_damped.makeCompressed();
      }

      Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
      solver.compute(H_damped);

      if (solver.info() == Eigen::Success) {
        step = solver.solve(-g);

        if (solver.info() == Eigen::Success && step.allFinite()) {
          solved_newton_system = true;

          if (lambda > 0.0) {
            std::cout << "Newton: adaptive damping accepted lambda = " << lambda
                      << std::endl;
          }

          break;
        }
      }

      lambda = (lambda == 0.0) ? lambda_initial : lambda * lambda_growth;

      result.damping_retry_count_m++;

      if (lambda > 0.0) {
        result.damping_was_used_m = true;
        result.max_damping_lambda_m =
            std::max(result.max_damping_lambda_m, lambda);
      }
    }

    if (!solved_newton_system) {
      result.converged_m = false;
      result.message_m =
          "Failed: sparse Newton solve failed after adaptive damping.";
      break;
    }

    result.step_norm_m = step.norm();

    if (!std::isfinite(result.step_norm_m)) {
      result.converged_m = false;
      result.message_m = "Failed: non-finite Newton step.";
      break;
    }

    if (result.step_norm_m <= options.step_tolerance_m) {
      result.converged_m = true;
      result.message_m = "Converged: step norm below tolerance.";
      break;
    }

    double alpha = options.initial_step_scale_m;
    std::vector<double> candidate_u;
    RandomEffectHessianResult candidate_eval;

    bool accepted = false;

    while (alpha >= options.min_step_scale_m) {
      candidate_u = add_scaled_step(u, step, alpha);

      candidate_eval = evaluate_random_effect_hessian(
          model, fixed, candidate_u, partition, options.hessian_drop_tol_m);

      const double armijo_rhs =
          eval.objective_value_m +
          options.sufficient_decrease_m * alpha * g.dot(step);

      if (!options.use_backtracking_m ||
          candidate_eval.objective_value_m <= armijo_rhs) {
        accepted = true;
        break;
      }

      alpha *= 0.5;
    }

    if (!accepted) {
      result.converged_m = false;
      result.message_m = "Failed: backtracking line search failed.";
      break;
    }

    u = candidate_u;
    eval = candidate_eval;
    result.iterations_m = iter + 1;

    if (eval.gradient_norm_m <= options.gradient_tolerance_m) {
      result.converged_m = true;
      result.message_m = "Converged: gradient norm below tolerance.";
      break;
    }
  }

  if (!result.converged_m && result.message_m.empty()) {
    result.message_m = "Stopped: maximum iterations reached.";
  }

  result.u_hat_m = u;
  result.full_m = eval.full_m;
  result.gradient_random_m = eval.gradient_random_m;
  result.hessian_random_m = eval.hessian_random_m;
  result.objective_value_m = eval.objective_value_m;
  result.gradient_norm_m = eval.gradient_norm_m;
  result.reports_m = eval.reports_m;

  if (result.damping_was_used_m) {
    std::cout << "Newton damping summary: retries="
              << result.damping_retry_count_m
              << ", max_lambda=" << result.max_damping_lambda_m << std::endl;
  }

  return result;
}

template <class Model>
inline RandomEffectNewtonResult optimize_random_effects_newton(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const RandomEffectNewtonOptions &options = RandomEffectNewtonOptions()) {
  return optimize_random_effects_newton(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra
