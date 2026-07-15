#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP
#pragma once

#include <chrono>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "../external/LBFGSpp/include/LBFGS.h"
#include "../external/eigen/Eigen/Dense"

#include "autodiff.hpp"
#include "laplace.hpp"
#include "laplace/model_analysis_report.hpp"
#include "laplace/persistent_structured_runtime.hpp"

namespace quadra {

struct OptPatternInfo {
  bool available = false;

  std::string detected_structure = "unknown";
  std::string backend = "unknown";
  std::string solver = "unknown";
  std::string complexity = "unknown";

  int bandwidth = -1;
  std::size_t rows = 0;
  std::size_t cols = 0;
  std::size_t nonzeros = 0;
  std::size_t random_effect_count = 0;
};

struct OptResult {
  Eigen::VectorXd x; // fitted fixed-effect vector
  // Final fixed-effect gradient diagnostics for optimizer troubleshooting.
  // Entries correspond to fixed_index and fixed_gradient_names.
  std::vector<std::string> fixed_gradient_names;
  std::vector<double> fixed_gradient;

  // Backward-compatible fixed-effect estimate.
  std::vector<double> par;

  // Random-effect mode at the final fixed-effect estimate.
  std::vector<double> u_hat;

  // Parameter indices used to construct par and u_hat.
  std::vector<int> fixed_index;
  std::vector<int> random_index;

  // Objective and outer-gradient diagnostics.
  double value = std::numeric_limits<double>::quiet_NaN();
  double joint_objective = std::numeric_limits<double>::quiet_NaN();
  double laplace_logdet = std::numeric_limits<double>::quiet_NaN();
  double laplace_constant = std::numeric_limits<double>::quiet_NaN();
  int iterations = 0;
  double grad_norm = std::numeric_limits<double>::quiet_NaN();

  bool converged = false;
  std::string message;

  // Random-effect Hessian / backend diagnostic payload.
  //
  // v1 fills the random-effect count and leaves detailed structure as unknown.
  // The next patch should wire this to the structure detector / backend
  // factory.
  OptPatternInfo pattern;
};

inline Eigen::VectorXd to_eigen(const std::vector<double> &x) {
  Eigen::VectorXd out(static_cast<Eigen::Index>(x.size()));
  for (Eigen::Index i = 0; i < out.size(); ++i) {
    out[i] = x[static_cast<size_t>(i)];
  }
  return out;
}

inline bool all_finite_eigen(const Eigen::VectorXd &v) {
  for (Eigen::Index i = 0; i < v.size(); ++i) {
    if (!std::isfinite(v[i])) {
      return false;
    }
  }
  return true;
}

inline double safe_eigen_norm(const Eigen::VectorXd &v) {
  if (!all_finite_eigen(v)) {
    return std::numeric_limits<double>::infinity();
  }
  return v.norm();
}

inline OptPatternInfo
make_opt_pattern_info_from_report(const laplace::ModelAnalysisReport &report) {
  OptPatternInfo info;
  info.available = true;
  info.detected_structure = laplace::ToString(report.structure);
  info.backend = laplace::ToString(report.backend);
  info.solver = laplace::ToString(report.solver);
  info.complexity = report.complexity;
  info.bandwidth = report.bandwidth;
  info.rows = static_cast<std::size_t>(report.rows);
  info.cols = static_cast<std::size_t>(report.cols);
  info.nonzeros = static_cast<std::size_t>(report.nnz);
  info.random_effect_count = static_cast<std::size_t>(report.random_effects);
  return info;
}

template <typename Model>
OptPatternInfo analyze_final_random_effect_pattern(
    Model &model, ParameterVector &params, const Eigen::VectorXd &x,
    const std::vector<double> &u_hat, const std::vector<int> &fixed_idx,
    const std::vector<int> &random_idx,
    const LaplaceOptions & /*options*/ = default_laplace_options()) {
  OptPatternInfo info;
  info.random_effect_count = random_idx.size();

  if (random_idx.empty() || u_hat.empty()) {
    info.available = false;
    info.detected_structure = "none";
    info.backend = "none";
    info.solver = "none";
    info.complexity = "none";
    return info;
  }

  if (u_hat.size() != random_idx.size()) {
    info.available = false;
    info.detected_structure = "unavailable";
    info.backend = "unavailable";
    info.solver = "unavailable";
    info.complexity = "random-effect mode size mismatch";
    return info;
  }

  try {
    had::ADGraph graph;
    ADScope scope(graph);

    std::vector<AD> p_full;
    p_full.reserve(params.size());

    for (int i = 0; i < params.size(); ++i) {
      p_full.emplace_back(AD(0.0));
    }

    inject_fixed_params(x, p_full, fixed_idx);
    inject_random_params(u_hat, p_full, random_idx);

    AD nll = model(p_full);
    scope.backward(nll);

    const auto &pattern = get_pattern(scope, p_full, random_idx);

    Eigen::SparseMatrix<double> H =
        extract_sparse_hessian(scope, p_full, random_idx, pattern, 0.0);

    laplace::StructureDetectorOptions detector_options;
    detector_options.prefer_dense_for_small_matrices = false;
    detector_options.dense_size_cutoff = 0;

    const laplace::ModelAnalysisReport report =
        laplace::analyze_hessian_structure(H, detector_options);

    return make_opt_pattern_info_from_report(report);
  } catch (const std::exception &e) {
    info.available = false;
    info.detected_structure = "unavailable";
    info.backend = "unavailable";
    info.solver = "unavailable";
    info.complexity = e.what();
    return info;
  } catch (...) {
    info.available = false;
    info.detected_structure = "unavailable";
    info.backend = "unavailable";
    info.solver = "unavailable";
    info.complexity = "unknown pattern-analysis failure";
    return info;
  }
}

template <typename Model>
LaplaceResult<Model> laplace_eval_at_u_star_persistent_structured(
    Model &model, ParameterVector &params, const std::vector<int> &fixed_idx,
    const std::vector<int> &random_idx, const Eigen::VectorXd &x,
    const std::vector<double> &u_star, had::ADGraph &graph,
    laplace::PersistentStructuredRuntimeState &structured_runtime,
    Eigen::VectorXd *last_logdet_x = nullptr,
    Eigen::VectorXd *last_logdet_u = nullptr,
    Eigen::VectorXd *last_logdet_grad = nullptr,
    bool *last_logdet_available = nullptr, double *timing_joint_ad_ms = nullptr,
    double *timing_logdet_gradient_ms = nullptr,
    double *timing_hessian_extract_ms = nullptr,
    double *timing_structured_logdet_ms = nullptr,
    const LaplaceOptions &options = default_laplace_options()) {
  ADScope scope(graph);

  using Result = LaplaceResult<Model>;
  Result res;

  std::vector<AD> p_full;
  p_full.reserve(params.size());

  for (int i = 0; i < params.size(); ++i) {
    p_full.emplace_back(AD(0.0));
  }

  inject_fixed_params(x, p_full, fixed_idx);
  inject_random_params(u_star, p_full, random_idx);

  const auto timing_joint_start = std::chrono::steady_clock::now();

  AD nll = model(p_full);

  scope.backward(nll);

  const auto timing_joint_end = std::chrono::steady_clock::now();
  if (timing_joint_ad_ms != nullptr) {
    *timing_joint_ad_ms += std::chrono::duration<double, std::milli>(
                               timing_joint_end - timing_joint_start)
                               .count();
  }

  res.grad_x.resize(fixed_idx.size());
  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    res.grad_x[k] = scope.grad(p_full[fixed_idx[k]]);
  }

#ifdef QUADRA_DEBUG_LAPLACE_GRADIENT_PARTS
  Eigen::VectorXd joint_grad_debug = Eigen::Map<Eigen::VectorXd>(
      res.grad_x.data(), static_cast<Eigen::Index>(res.grad_x.size()));
#endif

  // Fast comparison mode: skip exact logdet-gradient contribution.
  // The objective still includes the Laplace logdet term, but the fixed-effect
  // gradient uses the joint objective contribution only. This is useful for
  // profiling optimizer overhead before the logdet-gradient path is cached.
#if !defined(QUADRA_SKIP_EXACT_LOGDET_GRADIENT)
  {
    Eigen::Map<const Eigen::VectorXd> u_star_eigen(
        u_star.data(), static_cast<Eigen::Index>(u_star.size()));

    Eigen::VectorXd g_logdet =
        laplace_logdet_gradient_exact(model, params, x, u_star_eigen, options);

#ifdef QUADRA_DEBUG_LAPLACE_GRADIENT_PARTS
    Eigen::VectorXd g_logdet_fd =
        laplace_logdet_gradient_fd(model, params, x, u_star_eigen);
    std::cout << "Quadra logdet gradient parts\n";
    std::cout << "  logdet_grad      = " << g_logdet.transpose() << "\n";
    std::cout << "  logdet_fd_grad   = " << g_logdet_fd.transpose() << "\n";
    std::cout << "  logdet_grad diff = " << (g_logdet - g_logdet_fd).transpose()
              << "\n";
#endif

    had::g_ADGraph = &scope.graph;

    for (size_t k = 0; k < fixed_idx.size(); ++k) {
      res.grad_x[k] += g_logdet[static_cast<Eigen::Index>(k)];
    }

#ifdef QUADRA_DEBUG_LAPLACE_GRADIENT_PARTS
    Eigen::VectorXd total_grad_debug = Eigen::Map<Eigen::VectorXd>(
        res.grad_x.data(), static_cast<Eigen::Index>(res.grad_x.size()));
    std::cout << "Quadra gradient parts\n";
    std::cout << "  joint_grad  = " << joint_grad_debug.transpose() << "\n";
    std::cout << "  logdet_grad = " << g_logdet.transpose() << "\n";
    std::cout << "  total_grad  = " << total_grad_debug.transpose() << "\n";
#endif
  }
#endif

  res.grad_u.resize(random_idx.size());
  for (size_t k = 0; k < random_idx.size(); ++k) {
    res.grad_u[k] = scope.grad(p_full[random_idx[k]]);
  }

  const auto timing_hessian_start = std::chrono::steady_clock::now();

  const auto &pattern = get_pattern(scope, p_full, random_idx);

  Eigen::SparseMatrix<double> H = extract_sparse_hessian(
      scope, p_full, random_idx, pattern, options.hessian_drop_tol);

  const auto timing_hessian_end = std::chrono::steady_clock::now();
  if (timing_hessian_extract_ms != nullptr) {
    *timing_hessian_extract_ms += std::chrono::duration<double, std::milli>(
                                      timing_hessian_end - timing_hessian_start)
                                      .count();
  }

  // Persistent structured bridge:
  //   First call: detect structure and choose backend.
  //   Later calls: update structured values only and reuse recommendation.
  laplace::StructureDetectorOptions detector_options;
  detector_options.prefer_dense_for_small_matrices = false;
  detector_options.dense_size_cutoff = 0;

  const auto timing_structured_start = std::chrono::steady_clock::now();

  if (!structured_runtime.initialized) {
    structured_runtime.update_from_hessian(H, detector_options);
  } else {
    structured_runtime.update_values_only(H);
  }

  const double logdet = structured_runtime.logdet();

  const auto timing_structured_end = std::chrono::steady_clock::now();
  if (timing_structured_logdet_ms != nullptr) {
    *timing_structured_logdet_ms +=
        std::chrono::duration<double, std::milli>(timing_structured_end -
                                                  timing_structured_start)
            .count();
  }

  const double laplace_constant =
      0.5 * static_cast<double>(random_idx.size()) * std::log(2.0 * M_PI);

  res.joint_objective = value_of(nll);
  res.laplace_logdet = logdet;
  res.laplace_constant = laplace_constant;
  res.value = res.joint_objective + 0.5 * logdet - laplace_constant;

  return res;
}

struct LBFGSConvergedByGradient : public std::runtime_error {
  LBFGSConvergedByGradient()
      : std::runtime_error(
            "Quadra LBFGS reached requested gradient tolerance") {}
};

template <typename Model> class LBFGSObjective {
  void print(int iter, double fx, double gnorm) {
    const bool converged = std::isfinite(gnorm) && gnorm <= epsilon;
    std::cout << "L-BFGS: " << "outer eval = " << std::setw(3) << iter
              << ", fx = " << std::setw(14) << std::fixed
              << std::setprecision(6) << fx << ", |grad| = ";

    if (converged) {
      std::cout << "\033[1;32m";
    } else {
      std::cout << "\033[1;31m";
    }

    std::cout << std::setw(12) << std::fixed << std::setprecision(6) << gnorm
              << "\033[0m" << std::endl;
  }

public:
  double epsilon = 1e-6;
  Model &model;
  ParameterVector &params;
  std::vector<int> fixed_idx;
  std::vector<int> random_idx;
  LaplaceOptions options;

  int iter = 0;
  int print_every = 10;

  double timing_total_ms = 0.0;
  double timing_mode_solve_ms = 0.0;
  double timing_laplace_eval_ms = 0.0;
  double timing_joint_ad_ms = 0.0;
  double timing_logdet_gradient_ms = 0.0;
  double timing_hessian_extract_ms = 0.0;
  double timing_structured_logdet_ms = 0.0;
  int timing_eval_count = 0;

  double last_fx = std::numeric_limits<double>::quiet_NaN();
  double last_joint_objective = std::numeric_limits<double>::quiet_NaN();
  double last_laplace_logdet = std::numeric_limits<double>::quiet_NaN();
  double last_laplace_constant = std::numeric_limits<double>::quiet_NaN();
  Eigen::VectorXd last_grad;
  Eigen::VectorXd last_x;
  std::vector<double> last_u_star;

  Eigen::VectorXd best_converged_x;
  Eigen::VectorXd best_converged_grad;
  std::vector<double> best_converged_u_star;
  double best_converged_fx = std::numeric_limits<double>::quiet_NaN();
  int best_converged_iter = 0;
  bool has_best_converged = false;

  // Best finite point seen by the optimizer, independent of the final
  // line-search bookkeeping state.
  double best_fx = std::numeric_limits<double>::infinity();
  double best_grad_norm = std::numeric_limits<double>::infinity();
  Eigen::VectorXd best_x;
  Eigen::VectorXd best_grad;
  std::vector<double> best_u_star;
  bool best_available = false;

  // Best point satisfying the configured fixed-effect gradient tolerance.
  double best_converged_grad_norm = std::numeric_limits<double>::infinity();
  laplace::PersistentStructuredRuntimeState structured_runtime;

  Eigen::VectorXd last_logdet_x;
  Eigen::VectorXd last_logdet_u;
  Eigen::VectorXd last_logdet_grad;
  bool last_logdet_grad_available = false;

  LBFGSObjective(Model &m, ParameterVector &p, std::vector<int> fixed,
                 std::vector<int> random,
                 const LaplaceOptions &opts = default_laplace_options())
      : model(m), params(p), fixed_idx(std::move(fixed)),
        random_idx(std::move(random)), options(opts) {
    laplace_pattern_cache().clear();
    last_u_star.reserve(random_idx.size());
    for (int index : random_idx) {
      last_u_star.push_back(params.params[static_cast<size_t>(index)].value);
    }
  }

  double operator()(const VectorXd &x, VectorXd &grad) {
    TapeContext tape;
    had::ADGraph &graph = tape.graph;

    ++iter;
    ++timing_eval_count;
    const auto timing_eval_start = std::chrono::steady_clock::now();

    std::vector<double> u_star;
    const bool verbose_inner = ((iter % print_every) == 0) || iter == 1;

    try {
      const auto timing_mode_start = std::chrono::steady_clock::now();

      const std::vector<double> *u_warm_start =
          (last_u_star.size() == random_idx.size()) ? &last_u_star : nullptr;

      u_star = solve_random_effects_laplace(model, params, x, fixed_idx,
                                            random_idx, graph, u_warm_start);
      last_u_star = u_star;

      const auto timing_mode_end = std::chrono::steady_clock::now();
      timing_mode_solve_ms += std::chrono::duration<double, std::milli>(
                                  timing_mode_end - timing_mode_start)
                                  .count();
    } catch (const std::exception &e) {
      std::cerr << "L-BFGS: random-effect mode solve failed; returning "
                   "penalty. reason="
                << e.what() << std::endl;
      const double penalty_gradient_scale = 1.0e3;

      for (int i = 0; i < grad.size(); ++i) {
        const double xi = (i < x.size() && std::isfinite(x[i])) ? x[i] : 1.0;
        grad[i] = penalty_gradient_scale * ((xi == 0.0) ? 1.0 : xi);
      }

      return std::numeric_limits<double>::max() / 100.0;
    }

    using Result = LaplaceResult<Model>;
    Result res;

    try {
      const auto timing_laplace_start = std::chrono::steady_clock::now();

      res = laplace_eval_at_u_star_persistent_structured(
          model, params, fixed_idx, random_idx, x, u_star, graph,
          structured_runtime, &last_logdet_x, &last_logdet_u, &last_logdet_grad,
          &last_logdet_grad_available, &timing_joint_ad_ms,
          &timing_logdet_gradient_ms, &timing_hessian_extract_ms,
          &timing_structured_logdet_ms, options);

      const auto timing_laplace_end = std::chrono::steady_clock::now();
      timing_laplace_eval_ms += std::chrono::duration<double, std::milli>(
                                    timing_laplace_end - timing_laplace_start)
                                    .count();
    } catch (const std::exception &e) {
      std::cerr
          << "L-BFGS: Laplace evaluation failed; returning penalty. reason="
          << e.what() << std::endl;

      grad.resize(x.size());
      // grad.setZero();
      grad.setConstant(1.0e100);
      last_grad = grad;
      last_x = x;
      last_fx = std::numeric_limits<double>::max() / 1.0e100;
      return last_fx;
    } catch (...) {
      std::cerr << "L-BFGS: Laplace evaluation failed with unknown exception; "
                   "returning penalty."
                << std::endl;

      grad.resize(x.size());
      grad.setZero();
      last_grad = grad;
      last_x = x;
      last_fx = std::numeric_limits<double>::max() / 1.0e100;
      return last_fx;
    }

    grad = to_eigen(res.grad_x);

    last_fx = res.value;
    last_joint_objective = res.joint_objective;
    last_laplace_logdet = res.laplace_logdet;
    last_laplace_constant = res.laplace_constant;
    last_grad = grad;
    last_x = x;

    const double gnorm = safe_eigen_norm(grad);

    if (std::isfinite(res.value) && std::isfinite(gnorm) &&
        (!best_available || res.value < best_fx)) {
      best_fx = res.value;
      best_grad_norm = gnorm;
      best_x = x;
      best_grad = grad;
      best_u_star = u_star;
      best_available = true;
    }

    if (std::isfinite(gnorm) && gnorm <= epsilon) {
      if (!has_best_converged || !std::isfinite(best_converged_fx) ||
          res.value < best_converged_fx) {
        best_converged_x = x;
        best_converged_grad = grad;
        best_converged_u_star = u_star;
        best_converged_fx = res.value;
        best_converged_iter = iter;
        best_converged_grad_norm = gnorm;
        has_best_converged = true;
      }

      print(iter, res.value, gnorm);
      throw LBFGSConvergedByGradient();
    }

    if ((iter % print_every) == 0 || iter == 1) {
      print(iter, res.value, gnorm);
    }

    const auto timing_eval_end = std::chrono::steady_clock::now();
    timing_total_ms += std::chrono::duration<double, std::milli>(
                           timing_eval_end - timing_eval_start)
                           .count();

    return res.value;
  }
};

template <typename Model>
OptResult
optimize_lbfgs(Model &model, ParameterVector &params,
               const LaplaceOptions &options = default_laplace_options(),
               int max_iterations = 100,
               double gradient_tolerance = 1.0e-4) {
  using namespace LBFGSpp;
  using namespace Eigen;

  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (fixed_idx.empty()) {
    throw std::runtime_error(
        "No fixed parameters found — optimizer has zero dimension");
  }

  VectorXd x(static_cast<Eigen::Index>(fixed_idx.size()));
  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    x[static_cast<Eigen::Index>(k)] =
        params.params[static_cast<size_t>(fixed_idx[k])].value;
  }

  LBFGSObjective<Model> fun(model, params, fixed_idx, random_idx, options);
  fun.print_every = 25;

  LBFGSParam<double> param;
  param.max_iterations = max_iterations;
  param.m = 20;
  param.max_linesearch = 50;
#ifdef QUADRA_LBFGS_GRAD_TOL
  param.epsilon = QUADRA_LBFGS_GRAD_TOL;
#else
  param.epsilon = gradient_tolerance;
#endif
  fun.epsilon = param.epsilon;

  LBFGSSolver<double> solver(param);
  double fx = std::numeric_limits<double>::quiet_NaN();
  int niter = 0;

  int line_search_recovery_attempts = 0;
  bool line_search_recovery_used = false;
  std::string line_search_recovery_message;

  constexpr int max_line_search_recovery_attempts = 2;
  constexpr double recovery_step_initial = 1.0e-3;
  constexpr double recovery_step_shrink = 0.5;
  constexpr double recovery_step_min = 1.0e-12;

  while (true) {
    try {
      niter = solver.minimize(fun, x, fx);

      // quadra_lbfgs_honest_convergence_report_v1
      double quadra_final_fixed_grad_norm =
          std::numeric_limits<double>::quiet_NaN();
      if (fun.last_grad.size() > 0) {
        quadra_final_fixed_grad_norm = 0.0;
        for (int quadra_i = 0; quadra_i < fun.last_grad.size(); ++quadra_i) {
          quadra_final_fixed_grad_norm +=
              fun.last_grad[quadra_i] * fun.last_grad[quadra_i];
        }
        quadra_final_fixed_grad_norm = std::sqrt(quadra_final_fixed_grad_norm);
      }

      const bool quadra_requested_tol_met =
          std::isfinite(quadra_final_fixed_grad_norm) &&
          quadra_final_fixed_grad_norm <= 1.0e-4;

      std::cout << "L-BFGS minimize status report" << std::endl;
      std::cout << "  iterations returned by solver: " << niter << std::endl;
      std::cout << "  final objective returned by solver: " << fx << std::endl;
      std::cout << "  final fixed-gradient norm: "
                << quadra_final_fixed_grad_norm << std::endl;
      std::cout << "  requested gradient tolerance: " << std::scientific
                << 1.0e-4 << std::defaultfloat << std::endl;
      std::cout << "  configured max-iteration field: " << 400
                << " (LBFGSpp max_iterations)" << std::endl;
      std::cout << "  requested tolerance met: "
                << (quadra_requested_tol_met ? "yes" : "no") << std::endl;
      std::cout
          << "  outer convergence interpretation: "
          << (quadra_requested_tol_met
                  ? "converged to requested gradient tolerance"
                  : "stopped before requested gradient tolerance; inspect "
                    "LBFGS status/max iterations/line search")
          << std::endl;
      break;
    } catch (const LBFGSConvergedByGradient &) {
      if (fun.has_best_converged) {
        std::cout << "L-BFGS: stopped at first iterate satisfying requested "
                     "fixed-effect gradient tolerance."
                  << std::endl;
        fx = fun.best_converged_fx;
        x = fun.best_converged_x;
        niter = fun.best_converged_iter;
        break;
      } else {
        throw;
      }
    } catch (const std::runtime_error &e) {
      const double gnorm = safe_eigen_norm(fun.last_grad);
      const double max_grad = (fun.last_grad.size() > 0)
                                  ? fun.last_grad.cwiseAbs().maxCoeff()
                                  : std::numeric_limits<double>::infinity();

      const std::string msg = e.what();

      std::cout << "L-BFGS runtime_error caught: " << msg << "\n";
      std::cout << "  gnorm = " << gnorm << "\n";
      std::cout << "  max|grad| = " << max_grad << "\n";

      const bool line_search_failed =
          msg.find("line search") != std::string::npos ||
          msg.find("Line search") != std::string::npos ||
          msg.find("sufficiently decrease") != std::string::npos;

      const double convergence_like_grad = 2e-2;

      if (gnorm <= param.epsilon) {
        std::cout << "L-BFGS: optimization reached convergence criterion "
                  << "(|grad| <= epsilon). max|grad| = " << max_grad
                  << std::endl;

        if (fun.last_x.size() == x.size()) {
          x = fun.last_x;
        }

        fx = fun.last_fx;
        niter = fun.iter;
        break;
      }

      if (line_search_failed &&
          line_search_recovery_attempts < max_line_search_recovery_attempts &&
          fun.last_x.size() == x.size() && fun.last_grad.size() == x.size() &&
          std::isfinite(fun.last_fx) && fun.last_grad.allFinite() &&
          safe_eigen_norm(fun.last_grad) > 0.0) {
        ++line_search_recovery_attempts;
        line_search_recovery_used = true;
        line_search_recovery_message =
            "L-BFGS line search stalled; recovered with gradient restart.";

        const Eigen::VectorXd x0 = fun.last_x;
        const Eigen::VectorXd g = fun.last_grad;
        const double f0 = fun.last_fx;

        bool accepted = false;
        double alpha = recovery_step_initial;
        const double min_meaningful_decrease =
            1.0e-10 * std::max(1.0, std::abs(f0));

        while (alpha >= recovery_step_min) {
          Eigen::VectorXd trial = x0 - alpha * g;
          Eigen::VectorXd trial_grad;
          const double f_trial = fun(trial, trial_grad);

          if (std::isfinite(f_trial) &&
              f_trial < f0 - min_meaningful_decrease) {
            x = trial;
            fx = f_trial;
            accepted = true;
            break;
          }

          alpha *= recovery_step_shrink;
        }

        if (accepted) {
          std::cout << "L-BFGS: line-search recovery accepted gradient restart "
                    << "step. attempt = " << line_search_recovery_attempts
                    << ", alpha = " << alpha << ", fx = " << fx << std::endl;

          // LBFGSSolver stores param by reference and is not assignable.
          // Calling minimize() again from the accepted recovery point
          // rebuilds the quasi-Newton history inside LBFGSpp.
          continue;
        }

        std::cout << "L-BFGS: line-search recovery failed to find a decreasing "
                  << "gradient step." << std::endl;
      }

      if (line_search_failed && max_grad < convergence_like_grad) {
        std::cout
            << "L-BFGS: line search failed after a small fixed-effect "
               "gradient. "
            << "Returning the last finite iterate as a non-converged result. "
            << "max|grad| = " << max_grad << std::endl;

        if (fun.last_x.size() == x.size()) {
          x = fun.last_x;
        }

        fx = fun.last_fx;
        niter = fun.iter;
        break;
      }

      if (line_search_failed && fun.last_x.size() == x.size() &&
          std::isfinite(fun.last_fx)) {
        std::cout
            << "L-BFGS: line search failed after recovery attempts. "
            << "Returning the best finite iterate as a non-converged result "
            << "so callers can inspect diagnostics. max|grad| = " << max_grad
            << std::endl;

        x = fun.last_x;
        fx = fun.last_fx;
        niter = fun.iter;

        if (line_search_recovery_message.empty()) {
          line_search_recovery_message =
              "L-BFGS line search failed; returned best finite iterate.";
        }

        break;
      }

      throw;
    }
  }

#ifdef QUADRA_PROFILE_OPTIMIZER_TIMING
  std::cout << "Quadra timing summary\n";
  std::cout << "  objective evals:       " << fun.timing_eval_count << "\n";
  std::cout << "  total eval ms:         " << fun.timing_total_ms << "\n";
  std::cout << "  mode solve ms:         " << fun.timing_mode_solve_ms << "\n";
  std::cout << "  laplace eval ms:       " << fun.timing_laplace_eval_ms
            << "\n";
  std::cout << "    joint AD ms:         " << fun.timing_joint_ad_ms << "\n";
  std::cout << "    logdet gradient ms:  " << fun.timing_logdet_gradient_ms
            << "\n";
  std::cout << "    Hessian extract ms:  " << fun.timing_hessian_extract_ms
            << "\n";
  std::cout << "    structured logdet ms:" << fun.timing_structured_logdet_ms
            << "\n";
  std::cout << "  other eval ms:         "
            << (fun.timing_total_ms - fun.timing_mode_solve_ms -
                fun.timing_laplace_eval_ms)
            << "\n";

#endif

  OptResult result;

  Eigen::VectorXd selected_x;
  std::vector<double> selected_u_hat;
  double selected_fx = std::numeric_limits<double>::quiet_NaN();
  double selected_grad_norm = std::numeric_limits<double>::infinity();

  if (fun.has_best_converged) {
    selected_x = fun.best_converged_x;
    selected_u_hat = fun.best_converged_u_star;
    selected_fx = fun.best_converged_fx;
    selected_grad_norm = fun.best_converged_grad_norm;
  } else if (fun.best_available) {
    selected_x = fun.best_x;
    selected_u_hat = fun.best_u_star;
    selected_fx = fun.best_fx;
    selected_grad_norm = fun.best_grad_norm;
  } else if (fun.last_x.size() == x.size()) {
    selected_x = fun.last_x;
    selected_u_hat = fun.last_u_star;
    selected_fx = std::isfinite(fun.last_fx) ? fun.last_fx : fx;
    selected_grad_norm = safe_eigen_norm(fun.last_grad);
  } else {
    selected_x = x;
    selected_u_hat = fun.last_u_star;
    selected_fx = fx;
    selected_grad_norm = safe_eigen_norm(fun.last_grad);
  }

  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    params.params[static_cast<size_t>(fixed_idx[k])].value =
        selected_x[static_cast<Eigen::Index>(k)];
  }

  result.par.assign(selected_x.data(), selected_x.data() + selected_x.size());
  result.u_hat = selected_u_hat;
  result.fixed_index = fixed_idx;
  result.random_index = random_idx;

  result.fixed_gradient_names.clear();
  result.fixed_gradient.clear();
  result.fixed_gradient_names.reserve(fixed_idx.size());

  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    const auto idx = static_cast<size_t>(fixed_idx[k]);
    result.fixed_gradient_names.push_back(params.params[idx].name);
  }

  if (fun.last_grad.size() == static_cast<Eigen::Index>(fixed_idx.size())) {
    result.fixed_gradient.assign(fun.last_grad.data(),
                                 fun.last_grad.data() + fun.last_grad.size());
  }

  result.value = selected_fx;
  result.joint_objective = fun.last_joint_objective;

#ifdef QUADRA_DEBUG_FD_FINAL_GRADIENT
  {
    const double eps = 1.0e-5;
    Eigen::VectorXd fd = Eigen::VectorXd::Zero(selected_x.size());

    for (Eigen::Index j = 0; j < selected_x.size(); ++j) {
      Eigen::VectorXd xp = selected_x;
      Eigen::VectorXd xm = selected_x;
      xp[j] += eps;
      xm[j] -= eps;

      Eigen::VectorXd gp_vec;
      Eigen::VectorXd gm_vec;

      const double fp = fun(xp, gp_vec);
      const double fm = fun(xm, gm_vec);

      fd[j] = (fp - fm) / (2.0 * eps);
    }

    std::cout << "Quadra final profiled FD gradient = " << fd.transpose()
              << "\n";
    std::cout << "Quadra final analytic gradient = "
              << fun.last_grad.transpose() << "\n";
    std::cout << "Quadra final profiled FD-analytic diff = "
              << (fd - fun.last_grad).transpose() << "\n";
  }
#endif
  result.laplace_logdet = fun.last_laplace_logdet;
  result.laplace_constant = fun.last_laplace_constant;
  result.iterations = niter;
  result.grad_norm = std::isfinite(selected_grad_norm)
                         ? selected_grad_norm
                         : std::numeric_limits<double>::infinity();

  result.converged =
      std::isfinite(result.grad_norm) && result.grad_norm <= param.epsilon;

  result.message =
      result.converged
          ? "converged to requested fixed-effect gradient tolerance"
          : "stopped before requested fixed-effect gradient tolerance";

  if (line_search_recovery_used) {
    result.message += "; ";
    result.message += line_search_recovery_message;
    result.message += " attempts=";
    result.message += std::to_string(line_search_recovery_attempts);
  }

  Eigen::VectorXd pattern_x = selected_x;

  if (!random_idx.empty()) {
    result.pattern = analyze_final_random_effect_pattern(
        model, params, pattern_x, result.u_hat, fixed_idx, random_idx, options);
  } else {
    result.pattern.available = false;
    result.pattern.detected_structure = "none";
    result.pattern.backend = "none";
    result.pattern.solver = "none";
    result.pattern.complexity = "none";
    result.pattern.random_effect_count = 0;
  }

  result.x = x;
  return result;
}

} // namespace quadra

#endif
