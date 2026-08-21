#pragma once

#include "../autodiff/had_quadra_workspace.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "laplace_exact_gradient.hpp"
#include "laplace_exact_lbfgs_optimizer.hpp"
#include "random_effect_hessian.hpp"
#include "random_effect_newton.hpp"

namespace quadra {

struct LaplaceProfileTimings {
  double total_ms_m = 0.0;

  double random_newton_ms_m = 0.0;
  double random_hessian_ms_m = 0.0;
  double laplace_objective_ms_m = 0.0;
  double exact_gradient_ms_m = 0.0;
  double exact_lbfgs_ms_m = 0.0;

  int random_newton_iterations_m = 0;
  int exact_lbfgs_iterations_m = 0;

  bool random_newton_converged_m = false;
  bool exact_gradient_converged_m = false;
  bool exact_lbfgs_converged_m = false;

  double theta_hat_m = 0.0;
  double u_hat_m = 0.0;
  double gradient_norm_m = 0.0;
  double laplace_objective_m = 0.0;
};

template <typename F> inline double profile_elapsed_ms(F &&f) {
  const auto start = std::chrono::high_resolution_clock::now();
  f();
  const auto end = std::chrono::high_resolution_clock::now();

  return std::chrono::duration<double, std::milli>(end - start).count();
}

// Profile one fixed theta/u state, measuring the main Laplace components.
// This intentionally runs components separately, so total component time is
// diagnostic rather than a perfectly additive execution trace.
template <class Model>
inline LaplaceProfileTimings
profile_laplace_components(Model &model, const std::vector<double> &theta,
                           const std::vector<double> &u0,
                           const ParameterSet &parameters) {
  LaplaceProfileTimings profile;

  const auto partition = partition_parameters(parameters);

  RandomEffectNewtonResult newton;
  RandomEffectHessianResult hessian;
  LaplaceObjectiveResult laplace;
  LaplaceExactGradientResult exact_gradient;

  profile.total_ms_m = profile_elapsed_ms([&]() {
    profile.random_newton_ms_m = profile_elapsed_ms([&]() {
      newton = optimize_random_effects_newton(model, theta, u0, partition);
    });

    profile.random_hessian_ms_m = profile_elapsed_ms([&]() {
      hessian = evaluate_random_effect_hessian(model, theta, newton.u_hat_m,
                                               partition);
    });

    profile.laplace_objective_ms_m = profile_elapsed_ms([&]() {
      if (workspace != nullptr) {
        workspace->reset();
      }

      laplace = evaluate_laplace_objective(model, theta, u0, partition);
    });

    profile.exact_gradient_ms_m = profile_elapsed_ms([&]() {
      exact_gradient =
          evaluate_laplace_exact_gradient(model, theta, u0, partition);
    });
  });

  profile.random_newton_iterations_m = newton.iterations_m;
  profile.random_newton_converged_m = newton.converged_m;
  profile.exact_gradient_converged_m = exact_gradient.converged_m;

  if (!newton.u_hat_m.empty()) {
    profile.u_hat_m = newton.u_hat_m[0];
  }

  if (!theta.empty()) {
    profile.theta_hat_m = theta[0];
  }

  profile.gradient_norm_m = exact_gradient.gradient_norm_m;
  profile.laplace_objective_m = exact_gradient.laplace_objective_m;

  return profile;
}

// Profile the exact LBFGS optimizer end-to-end.
template <class Model>
inline LaplaceProfileTimings profile_exact_lbfgs(
    Model &model, const std::vector<double> &theta0,
    const std::vector<double> &u0, const ParameterSet &parameters,
    const LaplaceExactLBFGSOptions &options = LaplaceExactLBFGSOptions()) {
  LaplaceProfileTimings profile;

  LaplaceExactLBFGSResult result;

  profile.exact_lbfgs_ms_m = profile_elapsed_ms([&]() {
    result = optimize_laplace_fixed_effects_exact_lbfgs(model, theta0, u0,
                                                        parameters, options);
  });

  profile.total_ms_m = profile.exact_lbfgs_ms_m;
  profile.exact_lbfgs_iterations_m = result.iterations_m;
  profile.exact_lbfgs_converged_m = result.converged_m;
  profile.gradient_norm_m = result.gradient_norm_m;
  profile.laplace_objective_m = result.laplace_objective_m;

  if (!result.theta_hat_m.empty()) {
    profile.theta_hat_m = result.theta_hat_m[0];
  }

  if (!result.u_hat_m.empty()) {
    profile.u_hat_m = result.u_hat_m[0];
  }

  return profile;
}

} // namespace quadra
