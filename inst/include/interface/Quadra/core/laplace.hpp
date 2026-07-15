#include "laplace/exact_gradient_workspace.hpp"
#include <algorithm>
#include <random>
#ifndef QUADRA_LAPLACE_HPP
#define QUADRA_LAPLACE_HPP
#pragma once

#include "../external/eigen/Eigen/Dense"
#include "../external/eigen/Eigen/Sparse"
#include "../external/eigen/Eigen/SparseCholesky"
#include "../external/LBFGSpp/include/LBFGS.h"
#include "../model/parameter.hpp"
#include "autodiff.hpp"
#include "evaluation.hpp"
#include "laplace/laplace_evaluator_exact_gradient_integration.hpp"
#include "laplace/laplace_gradient_diagnostics.hpp"
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace quadra {

using Eigen::MatrixXd;
using Eigen::VectorXd;

//==================================================
// Laplace options
//==================================================
struct LaplaceOptions {
  // Trace strategy for tr(H^{-1} Hdot).
  // For very large random-effect systems Hutchinson avoids dense RHS solves.
  bool use_hutchinson_trace = true;
  int hutchinson_probes = 8;
  unsigned int hutchinson_seed = 12345;

  // Adaptive diagonal jitter for sparse factorizations.
  // Jitter is only applied if the unmodified Hessian fails to factorize.
  double jitter_initial = 1e-12;
  int jitter_max_attempts = 12;

  // Validation/debugging knobs.
  // Compile-time validation is still controlled by QUADRA_VALIDATE_HDOT,
  // but this flag lets the runtime call sites opt out if desired.
  bool validate_hdot = true;

  // Threshold for dropping sparse Hessian entries.
  // Use 0.0 for logdet paths so very small curvature is not dropped.
  double hessian_drop_tol = 0.0;
};

inline LaplaceOptions &default_laplace_options() {
  static LaplaceOptions options;
  return options;
}

//==============================
// Build fixed index map
//==============================
inline std::vector<int> build_fixed_index(const ParameterVector &params) {
  std::vector<int> idx;
  for (size_t i = 0; i < params.params.size(); ++i) {
    if (!params.params[i].is_random)
      idx.push_back(i);
  }
  return idx;
}

//==============================
// Build random index map
//==============================
inline std::vector<int> build_random_index(const ParameterVector &params) {
  std::vector<int> idx;
  for (size_t i = 0; i < params.params.size(); ++i) {
    if (params.params[i].is_random)
      idx.push_back(i);
  }
  return idx;
}

inline std::vector<double>
build_u_init_from_cache(const std::vector<int> &random_idx) {
  return std::vector<double>(random_idx.size(), 0.0);
}

inline void inject_fixed_params(const Eigen::VectorXd &x,
                                ParameterVector &params,
                                const std::vector<int> &fixed_idx) {
  assert(x.size() == fixed_idx.size());

  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    const int idx = fixed_idx[k];
    params.params[idx].value = x[k];
  }
}

template <typename Scalar>
inline void inject_fixed_params(const std::vector<Scalar> &x_ad,
                                std::vector<Scalar> &p,
                                const std::vector<int> &fixed_idx) {
  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    p[fixed_idx[k]] = x_ad[k];
  }
}

template <typename Scalar>
inline void inject_fixed_params(const Eigen::VectorXd &x,
                                std::vector<Scalar> &p,
                                const std::vector<int> &fixed_idx) {
  for (size_t k = 0; k < fixed_idx.size(); ++k)
    p[fixed_idx[k]] = Scalar(x[k]);
}

inline void inject_random_params(const std::vector<double> &u,
                                 ParameterVector &params,
                                 const std::vector<int> &random_idx) {
  assert(u.size() == random_idx.size());

  for (size_t k = 0; k < random_idx.size(); ++k) {
    const int idx = random_idx[k];
    params.params[idx].value = u[k];
  }
}

template <typename Scalar>
inline void inject_random_params(const std::vector<Scalar> &u_ad,
                                 std::vector<Scalar> &p,
                                 const std::vector<int> &random_idx) {
  for (size_t k = 0; k < random_idx.size(); ++k) {
    p[random_idx[k]] = u_ad[k];
  }
}

template <typename Scalar>
inline void inject_random_params(const std::vector<double> &u,
                                 std::vector<Scalar> &p,
                                 const std::vector<int> &random_idx) {
  for (size_t k = 0; k < random_idx.size(); ++k) {
    p[random_idx[k]] = Scalar(u[k]);
  }
}

template <typename T>
std::vector<T> pack_params(const std::vector<T> &u, const std::vector<T> &x,
                           const ParameterVector &params,
                           const std::vector<int> &random_idx,
                           const std::vector<int> &fixed_idx) {
  std::vector<T> p(params.params.size());

  int u_k = 0;
  int x_k = 0;

  for (size_t i = 0; i < p.size(); i++) {
    if (params.params[i].is_random) {
      p[i] = u[u_k++];
    } else {
      p[i] = x[x_k++];
    }
  }

  return p;
}

//==================================================
// Laplace-local Hessian pattern representation
//==================================================
// Do not name this HessianPattern. autodiff.hpp may define a
// graph-level HessianPattern helper for ADGraph sparsity discovery.
// Keeping the Laplace cache as SparseHessianPattern avoids redefinition
// errors and keeps this file independent of the exact autodiff helper API.
using SparseHessianPattern = std::vector<std::pair<int, int>>;

inline std::unordered_map<int, SparseHessianPattern> &laplace_pattern_cache() {
  static std::unordered_map<int, SparseHessianPattern> cache;
  return cache;
}

//==================================================
// Discover Hessian sparsity from had::ADGraph
//==================================================
// This replaces the older dense pattern probe. It reads the sparse
// edge-pushed Hessian storage that had::PropagateAdjoint() has already
// populated inside scope.backward(nll).
//
// NOTE: this is still a numeric sparsity pattern. If a structurally
// nonzero Hessian entry evaluates to exactly zero at the discovery point,
// it can be missed. Diagonals are included by default for Newton stability.
inline SparseHessianPattern discover_pattern_from_graph(
    const std::vector<AD> &p_full, const std::vector<int> &random_idx,
    bool symmetric = true, bool include_diagonal = true, double tol = 1e-12) {
  std::cout << "Quadra: Discovering Hessian pattern from AD graph for "
            << random_idx.size() << " random variables ...\n";

  const int n = static_cast<int>(random_idx.size());
  SparseHessianPattern pattern;

  if (n == 0 || had::g_ADGraph == nullptr)
    return pattern;

  std::unordered_map<had::VertexId, int> random_var_to_local;
  random_var_to_local.reserve(static_cast<size_t>(n));

  for (int local = 0; local < n; ++local) {
    const int full_index = random_idx[static_cast<size_t>(local)];
    random_var_to_local.emplace(p_full[full_index].varId, local);
  }

  std::set<std::pair<int, int>> unique_pairs;

  if (include_diagonal) {
    for (int i = 0; i < n; ++i)
      unique_pairs.emplace(i, i);
  } else {
    for (int i = 0; i < n; ++i) {
      const int full_index = random_idx[static_cast<size_t>(i)];
      const had::VertexId vi = p_full[full_index].varId;

      if (vi < had::g_ADGraph->selfSoEdges.size() &&
          std::abs(had::g_ADGraph->selfSoEdges[vi]) > tol) {
        unique_pairs.emplace(i, i);
      }
    }
  }

  // had stores an off-diagonal Hessian entry in soEdges[max_id]
  // under key min_id. Walk the graph-level sparse storage and retain
  // only entries where both endpoints are random-effect variables.
  for (had::VertexId hi = 0;
       hi < static_cast<had::VertexId>(had::g_ADGraph->soEdges.size()); ++hi) {
    auto hi_it = random_var_to_local.find(hi);
    if (hi_it == random_var_to_local.end())
      continue;

    const int i = hi_it->second;
    const auto &tree = had::g_ADGraph->soEdges[hi];

    for (const auto &node : tree.nodes) {
      if (std::abs(node.val) <= tol)
        continue;

      auto lo_it = random_var_to_local.find(node.key);
      if (lo_it == random_var_to_local.end())
        continue;

      const int j = lo_it->second;
      unique_pairs.emplace(i, j);
      if (symmetric)
        unique_pairs.emplace(j, i);
    }
  }

  pattern.reserve(unique_pairs.size());
  for (const auto &ij : unique_pairs)
    pattern.emplace_back(ij.first, ij.second);

  std::cout << "Quadra: Model structure aware now => Hessian pattern has "
            << pattern.size() << " entries.\n";
  return pattern;
}

inline const SparseHessianPattern &
get_pattern(const ADScope &scope, const std::vector<AD> &p_full,
            const std::vector<int> &random_idx) {
  // See extract_sparse_hessian(): g_ADGraph may have been changed by
  // nested derivative/logdet helper evaluations.
  had::g_ADGraph = &scope.graph;

  const int n = static_cast<int>(random_idx.size());
  auto &cache = laplace_pattern_cache();

  auto it = cache.find(n);
  if (it != cache.end())
    return it->second;

  auto pattern = discover_pattern_from_graph(p_full, random_idx);
  auto res = cache.emplace(n, std::move(pattern));
  return res.first->second;
}

inline SparseHessianPattern banded_hessian_pattern(int n, int bandwidth) {
  SparseHessianPattern pattern;

  for (int i = 0; i < n; ++i) {
    int j0 = std::max(0, i - bandwidth);
    int j1 = std::min(n - 1, i + bandwidth);

    for (int j = j0; j <= j1; ++j)
      pattern.emplace_back(i, j);
  }

  return pattern;
}

inline SparseHessianPattern dense_hessian_pattern(int n) {
  SparseHessianPattern pattern;
  pattern.reserve(n * n);

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      pattern.emplace_back(i, j);

  return pattern;
}

inline Eigen::SparseMatrix<double>
extract_sparse_hessian(const ADScope &scope, const std::vector<AD> &p_full,
                       const std::vector<int> &random_idx,
                       const SparseHessianPattern &pattern,
                       double drop_tol = 1e-12) {
  // Important:
  // had::g_ADGraph is global/thread-local. Other helper calls may build
  // temporary graphs and leave this pointer changed. Always restore it
  // before reading adjoints/Hessian entries from this ADScope.
  had::g_ADGraph = &scope.graph;

  const int n = (int)random_idx.size();

  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(pattern.size());

  for (const auto &[i, j] : pattern) {
    double hij = scope.hess(p_full[random_idx[i]], p_full[random_idx[j]]);
    if (std::abs(hij) > drop_tol)
      triplets.emplace_back(i, j, hij);
  }

  Eigen::SparseMatrix<double> H(n, n);
  H.setFromTriplets(triplets.begin(), triplets.end());
  return H;
}

inline double sparse_logdet_ldlt(const Eigen::SparseMatrix<double> &H) {
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
  solver.compute(H);

  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("Sparse LDLT factorization failed");
  }

  const auto &D = solver.vectorD();

  double logdet = 0.0;

  for (int i = 0; i < D.size(); ++i) {
    if (D[i] <= 0.0) {
      throw std::runtime_error("Sparse Hessian is not positive definite");
    }

    logdet += std::log(D[i]);
  }

  return logdet;
}

//==================================================
// Sparse factorization helpers
//
// Adaptive jitter is only applied if the original Hessian fails
// to factorize. This avoids biasing gradients near valid optima
// while still protecting against near-singular random-effect
// Hessians during stress tests or weakly identified models.
//==================================================
inline Eigen::SparseMatrix<double>
add_diagonal_jitter(const Eigen::SparseMatrix<double> &H, double jitter) {
  Eigen::SparseMatrix<double> H_reg = H;
  H_reg.makeCompressed();

  for (int k = 0; k < H_reg.outerSize(); ++k) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(H_reg, k); it; ++it) {
      if (it.row() == it.col()) {
        it.valueRef() += jitter;
      }
    }
  }

  return H_reg;
}

inline Eigen::SparseMatrix<double> factorize_with_adaptive_jitter(
    const Eigen::SparseMatrix<double> &H,
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> &solver,
    const char *context,
    const LaplaceOptions &options = default_laplace_options()) {
  Eigen::SparseMatrix<double> H_factor = H;
  H_factor.makeCompressed();

  solver.compute(H_factor);

  if (solver.info() == Eigen::Success) {
    return H_factor;
  }

  double jitter = options.jitter_initial;

  for (int attempt = 0; attempt < options.jitter_max_attempts; ++attempt) {
    H_factor = add_diagonal_jitter(H, jitter);

    solver.compute(H_factor);

    if (solver.info() == Eigen::Success) {
      std::cout << "Quadra: " << context
                << " succeeded with diagonal jitter = " << jitter << "\\n";

      return H_factor;
    }

    jitter *= 10.0;
  }

  throw std::runtime_error(std::string(context) +
                           ": sparse factorization failed");
}

inline double sparse_logdet_llt(const Eigen::SparseMatrix<double> &H) {
  Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> solver;
  solver.compute(H);

  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("Sparse LLT factorization failed");
  }

  Eigen::SparseMatrix<double> L = solver.matrixL();

  double logdet = 0.0;

  for (int k = 0; k < L.outerSize(); ++k) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(L, k); it; ++it) {
      if (it.row() == it.col()) {
        if (it.value() <= 0.0) {
          throw std::runtime_error("Non-positive Cholesky diagonal");
        }

        logdet += 2.0 * std::log(it.value());
      }
    }
  }

  return logdet;
}
//==================================================
// Solve for random effects u* via Newton
//==================================================
inline void propagate_first_order(had::ADGraph &graph,
                                  had::VertexId objective_vertex) {
  had::ZeroAdjoints(graph);
  graph.vertices[objective_vertex].w = 1.0;
  for (had::VertexId vertex_id =
           static_cast<had::VertexId>(graph.vertices.size() - 1);
       vertex_id > 0; --vertex_id) {
    had::ADVertex &vertex = graph.vertices[vertex_id];
    if (vertex.e1.to == vertex_id)
      continue;
    const double adjoint = vertex.w;
    graph.vertices[vertex.e1.to].w += adjoint * vertex.e1.w;
    if (vertex.e2.to != vertex_id) {
      graph.vertices[vertex.e2.to].w += adjoint * vertex.e2.w;
    }
  }
}

template <typename Model>
struct FirstOrderJointEvaluation {
  double value = 0.0;
  Eigen::VectorXd fixed_gradient;
  Eigen::VectorXd random_gradient;
};

template <typename Model>
FirstOrderJointEvaluation<Model> evaluate_joint_first_order(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &random, const std::vector<int> &fixed_idx,
    const std::vector<int> &random_idx) {
  had::ADGraph graph;
  ADScope scope(graph);
  std::vector<AD> full;
  full.reserve(params.size());
  for (int i = 0; i < params.size(); ++i)
    full.emplace_back(AD(0.0));
  inject_fixed_params(theta, full, fixed_idx);
  std::vector<double> random_std(random.data(), random.data() + random.size());
  inject_random_params(random_std, full, random_idx);
  AD objective = model(full);
  propagate_first_order(graph, objective.varId);
  FirstOrderJointEvaluation<Model> out;
  out.value = graph.vertices[objective.varId].primal;
  out.fixed_gradient.resize(static_cast<Eigen::Index>(fixed_idx.size()));
  out.random_gradient.resize(static_cast<Eigen::Index>(random_idx.size()));
  for (size_t i = 0; i < fixed_idx.size(); ++i)
    out.fixed_gradient[static_cast<Eigen::Index>(i)] =
        had::GetAdjoint(full[fixed_idx[i]]);
  for (size_t i = 0; i < random_idx.size(); ++i)
    out.random_gradient[static_cast<Eigen::Index>(i)] =
        had::GetAdjoint(full[random_idx[i]]);
  return out;
}

template <typename Model>
Eigen::MatrixXd dense_random_hessian_from_gradient_fd(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &random, const std::vector<int> &fixed_idx,
    const std::vector<int> &random_idx, double relative_step = 1e-5) {
  const Eigen::Index n = random.size();
  Eigen::MatrixXd hessian(n, n);
  for (Eigen::Index column = 0; column < n; ++column) {
    const double step = relative_step * std::max(1.0, std::abs(random[column]));
    Eigen::VectorXd plus = random;
    Eigen::VectorXd minus = random;
    plus[column] += step;
    minus[column] -= step;
    const auto gplus = evaluate_joint_first_order(
        model, params, theta, plus, fixed_idx, random_idx).random_gradient;
    const auto gminus = evaluate_joint_first_order(
        model, params, theta, minus, fixed_idx, random_idx).random_gradient;
    hessian.col(column) = (gplus - gminus) / (2.0 * step);
  }
  return 0.5 * (hessian + hessian.transpose());
}

template <typename Model>
std::vector<double> solve_random_effects_laplace(
    Model &model, ParameterVector &params, const Eigen::VectorXd &x,
    const std::vector<int> &fixed_idx, const std::vector<int> &random_idx,
    had::ADGraph &graph, const std::vector<double> *u_init_override = nullptr) {
  const double tol = 1e-8;

  std::vector<double> u = (u_init_override != nullptr &&
                           u_init_override->size() == random_idx.size())
                              ? *u_init_override
                              : std::vector<double>(random_idx.size(), 0.0);

  Eigen::VectorXd random_values = Eigen::Map<Eigen::VectorXd>(
      u.data(), static_cast<Eigen::Index>(u.size()));
  auto objective = [&](const Eigen::VectorXd &candidate,
                       Eigen::VectorXd &gradient) -> double {
    ADScope scope(graph);
    std::vector<AD> p_full;
    p_full.reserve(params.size());
    for (int i = 0; i < params.size(); ++i)
      p_full.emplace_back(AD(0.0));
    inject_fixed_params(x, p_full, fixed_idx);
    std::vector<double> candidate_std(candidate.data(),
                                      candidate.data() + candidate.size());
    inject_random_params(candidate_std, p_full, random_idx);
    AD nll = model(p_full);
    propagate_first_order(graph, nll.varId);
    gradient.resize(candidate.size());
    for (size_t i = 0; i < random_idx.size(); ++i)
      gradient[static_cast<Eigen::Index>(i)] =
          had::GetAdjoint(p_full[random_idx[i]]);
    return graph.vertices[nll.varId].primal;
  };
  LBFGSpp::LBFGSParam<double> inner_options;
  inner_options.max_iterations = 200;
  inner_options.epsilon = tol;
  inner_options.m = 10;
  inner_options.max_linesearch = 30;
  LBFGSpp::LBFGSSolver<double> inner_solver(inner_options);
  double objective_value = 0.0;
  try {
    inner_solver.minimize(objective, random_values, objective_value);
  } catch (const std::exception &) {
    // LBFGSpp can report a line-search stop at a useful finite mode. Validate
    // the returned point below through the next Laplace evaluation.
  }
  return std::vector<double>(random_values.data(),
                             random_values.data() + random_values.size());
}

//==================================================
// Compute sparse random-effect Hessian at current params
//==================================================
template <typename Model>
Eigen::SparseMatrix<double>
compute_random_hessian_sparse(Model &model, ParameterVector &params) {
  had::ADGraph *previous_graph = had::g_ADGraph;

  had::ADGraph graph;
  ADScope scope(graph);

  const std::vector<int> random_idx = build_random_index(params);

  std::vector<AD> p_full;
  p_full.reserve(static_cast<size_t>(params.size()));

  for (int i = 0; i < params.size(); ++i) {
    p_full.emplace_back(AD(params.params[static_cast<size_t>(i)].value));
  }

  AD nll = model(p_full);
  scope.backward(nll);

  const auto &pattern = get_pattern(scope, p_full, random_idx);

  Eigen::SparseMatrix<double> H =
      extract_sparse_hessian(scope, p_full, random_idx, pattern);

  had::g_ADGraph = previous_graph;
  return H;
}

//==================================================
// Laplace log-determinant at supplied fixed/random state
//==================================================
template <typename Model>
double laplace_logdet(Model &model, ParameterVector &params,
                      const Eigen::VectorXd &theta,
                      const Eigen::VectorXd &u_hat) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  inject_fixed_params(theta, params, fixed_idx);

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  inject_random_params(u, params, random_idx);

  Eigen::SparseMatrix<double> H = compute_random_hessian_sparse(model, params);

  return sparse_logdet_ldlt(H);
}

//==================================================
// trace(H^{-1} Hdot), using an existing sparse factorization
//==================================================
//==================================================
// Stochastic Hutchinson trace estimator
//
// Approximates:
//
//     trace(H^{-1} Hdot)
//
// using:
//
//     E[zᵀ H^{-1} Hdot z]
//
// with Rademacher (+/-1) probe vectors.
//
// This avoids catastrophic dense materialization for large
// random-effect systems.
//==================================================
template <typename SolverType>
double logdet_directional_derivative_from_hdot(
    SolverType &solver, const Eigen::SparseMatrix<double> &Hdot,
    const LaplaceOptions &options = default_laplace_options()) {
  if (Hdot.rows() != Hdot.cols()) {
    throw std::invalid_argument(
        "logdet_directional_derivative_from_hdot: Hdot not square");
  }

  const Eigen::Index n = Hdot.rows();

  if (!options.use_hutchinson_trace) {
    // Deterministic exact trace for small/moderate systems.
    // This should not be used for very large random-effect systems.
    Eigen::MatrixXd rhs = Eigen::MatrixXd(Hdot);
    Eigen::MatrixXd X = solver.solve(rhs);

    if (solver.info() != Eigen::Success) {
      throw std::runtime_error(
          "logdet_directional_derivative_from_hdot: dense trace solve failed");
    }

    return X.diagonal().sum();
  }

  std::mt19937 rng(options.hutchinson_seed);
  std::uniform_int_distribution<int> rademacher(0, 1);

  double trace_est = 0.0;

  for (int sample = 0; sample < options.hutchinson_probes; ++sample) {
    Eigen::VectorXd z(n);

    for (Eigen::Index i = 0; i < n; ++i) {
      z[i] = (rademacher(rng) == 0) ? -1.0 : 1.0;
    }

    Eigen::VectorXd y = Hdot * z;
    Eigen::VectorXd x = solver.solve(y);

    if (solver.info() != Eigen::Success) {
      throw std::runtime_error("logdet_directional_derivative_from_hdot: "
                               "Hutchinson sparse solve failed");
    }

    trace_est += z.dot(x);
  }

  return trace_est / static_cast<double>(options.hutchinson_probes);
}

//==================================================
// Finite-difference directional derivative of random Hessian
// Hdot = d H_u(theta)[direction]
//==================================================

//==================================================
// Implicit sensitivity of optimized random effects
//
// u*(theta) satisfies f_u(theta, u*) = 0.
// Differentiating:
//
//     H_uu du*/dtheta_i + H_u theta_i = 0
//
// so:
//
//     du*/dtheta_i = - H_uu^{-1} H_u theta_i
//
// This avoids re-solving the random effects for theta +/- eps.
//==================================================
template <typename Model>
Eigen::VectorXd implicit_du_dtheta_i(Model &model, ParameterVector &params,
                                     const Eigen::VectorXd &theta,
                                     const Eigen::VectorXd &u_hat,
                                     Eigen::Index theta_i) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (theta_i < 0 || theta_i >= theta.size()) {
    throw std::out_of_range("implicit_du_dtheta_i: theta_i out of range");
  }

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  had::ADGraph graph;
  ADScope scope(graph);

  std::vector<AD> p_full;
  p_full.reserve(static_cast<size_t>(params.size()));

  for (int i = 0; i < params.size(); ++i) {
    p_full.emplace_back(AD(0.0));
  }

  inject_fixed_params(theta, p_full, fixed_idx);
  inject_random_params(u, p_full, random_idx);

  AD nll = model(p_full);
  scope.backward(nll);

  const auto &pattern = get_pattern(scope, p_full, random_idx);

  Eigen::SparseMatrix<double> Huu =
      extract_sparse_hessian(scope, p_full, random_idx, pattern);

  Eigen::VectorXd Hu_theta(static_cast<Eigen::Index>(random_idx.size()));

  const int fixed_full_index = fixed_idx[static_cast<size_t>(theta_i)];

  for (size_t r = 0; r < random_idx.size(); ++r) {
    Hu_theta[static_cast<Eigen::Index>(r)] =
        scope.hess(p_full[random_idx[r]], p_full[fixed_full_index]);
  }

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
  solver.compute(Huu);

  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("implicit_du_dtheta_i: H_uu factorization failed");
  }

  Eigen::VectorXd du = -solver.solve(Hu_theta);

  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("implicit_du_dtheta_i: solve failed");
  }

  return du;
}

//==================================================
// Fast implicit sensitivities for all fixed effects
//
// Reuses one H_uu factorization and computes:
//
//     du*/dtheta_i = - H_uu^{-1} H_u theta_i
//
// for every fixed-effect direction.
//
// Columns of the returned matrix correspond to fixed effects.
//==================================================
template <typename Model>
Eigen::MatrixXd implicit_du_dtheta_all(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat,
    const Eigen::SparseMatrix<double> *Huu_reuse = nullptr,
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> *solver_reuse =
        nullptr) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  had::ADGraph graph;
  ADScope scope(graph);

  std::vector<AD> p_full;
  p_full.reserve(static_cast<size_t>(params.size()));

  for (int i = 0; i < params.size(); ++i) {
    p_full.emplace_back(AD(0.0));
  }

  inject_fixed_params(theta, p_full, fixed_idx);
  inject_random_params(u, p_full, random_idx);

  AD nll = model(p_full);
  scope.backward(nll);

  Eigen::SparseMatrix<double> Huu_local;
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver_local;

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> *solver_ptr = solver_reuse;

  if (solver_ptr == nullptr) {
    if (Huu_reuse != nullptr) {
      solver_local.compute(*Huu_reuse);
    } else {
      const auto &pattern = get_pattern(scope, p_full, random_idx);

      Huu_local = extract_sparse_hessian(scope, p_full, random_idx, pattern);

      solver_local.compute(Huu_local);
    }

    if (solver_local.info() != Eigen::Success) {
      throw std::runtime_error(
          "implicit_du_dtheta_all: H_uu factorization failed");
    }

    solver_ptr = &solver_local;
  }

  Eigen::MatrixXd Hu_theta(static_cast<Eigen::Index>(random_idx.size()),
                           static_cast<Eigen::Index>(fixed_idx.size()));

  for (size_t j = 0; j < fixed_idx.size(); ++j) {
    const int fixed_full_index = fixed_idx[j];

    for (size_t r = 0; r < random_idx.size(); ++r) {
      Hu_theta(static_cast<Eigen::Index>(r), static_cast<Eigen::Index>(j)) =
          scope.hess(p_full[random_idx[r]], p_full[fixed_full_index]);
    }
  }

  Eigen::MatrixXd du = -solver_ptr->solve(Hu_theta);

  if (solver_ptr->info() != Eigen::Success) {
    throw std::runtime_error("implicit_du_dtheta_all: solve failed");
  }

  return du;
}

//==================================================
// Same as random_hessian_directional_implicit_fd(), but accepts
// a precomputed du*/dtheta_i vector. This avoids refactorizing
// H_uu inside every fixed-effect direction.
//==================================================
template <typename Model>
Eigen::SparseMatrix<double> random_hessian_directional_implicit_fd_with_du(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat, Eigen::Index theta_i,
    const Eigen::VectorXd &du, double eps = 1e-5) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (theta_i < 0 || theta_i >= theta.size()) {
    throw std::out_of_range(
        "random_hessian_directional_implicit_fd_with_du: theta_i out of range");
  }

  Eigen::VectorXd theta_plus = theta;
  Eigen::VectorXd theta_minus = theta;

  theta_plus[theta_i] += eps;
  theta_minus[theta_i] -= eps;

  Eigen::VectorXd u_plus = u_hat + eps * du;

  Eigen::VectorXd u_minus = u_hat - eps * du;

  std::vector<double> u_plus_std(u_plus.data(), u_plus.data() + u_plus.size());

  std::vector<double> u_minus_std(u_minus.data(),
                                  u_minus.data() + u_minus.size());

  inject_fixed_params(theta_plus, params, fixed_idx);
  inject_random_params(u_plus_std, params, random_idx);

  Eigen::SparseMatrix<double> Hplus =
      compute_random_hessian_sparse(model, params);

  inject_fixed_params(theta_minus, params, fixed_idx);
  inject_random_params(u_minus_std, params, random_idx);

  Eigen::SparseMatrix<double> Hminus =
      compute_random_hessian_sparse(model, params);

  std::vector<double> u_base(u_hat.data(), u_hat.data() + u_hat.size());

  inject_fixed_params(theta, params, fixed_idx);
  inject_random_params(u_base, params, random_idx);

  return (Hplus - Hminus) * (0.5 / eps);
}

//==================================================
// Implicit-direction finite-difference derivative of H_uu
//
// Instead of expensive profiled FD:
//
//     H(theta +/- eps, u*(theta +/- eps))
//
// this uses:
//
//     u*(theta +/- eps e_i)
//       ~= u*(theta) +/- eps du*/dtheta_i
//
// and computes:
//
//     Hdot_i ~= [H(theta+eps e_i, u+eps du_i)
//              - H(theta-eps e_i, u-eps du_i)] / (2 eps)
//
// This is still a finite-difference bridge, but it avoids nested
// random-effect Newton solves for each fixed-effect direction.
//==================================================
template <typename Model>
Eigen::SparseMatrix<double> random_hessian_directional_implicit_fd(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat, Eigen::Index theta_i, double eps = 1e-5) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (theta_i < 0 || theta_i >= theta.size()) {
    throw std::out_of_range(
        "random_hessian_directional_implicit_fd: theta_i out of range");
  }

  Eigen::VectorXd du =
      implicit_du_dtheta_i(model, params, theta, u_hat, theta_i);

  Eigen::VectorXd theta_plus = theta;
  Eigen::VectorXd theta_minus = theta;

  theta_plus[theta_i] += eps;
  theta_minus[theta_i] -= eps;

  Eigen::VectorXd u_plus = u_hat + eps * du;

  Eigen::VectorXd u_minus = u_hat - eps * du;

  std::vector<double> u_plus_std(u_plus.data(), u_plus.data() + u_plus.size());

  std::vector<double> u_minus_std(u_minus.data(),
                                  u_minus.data() + u_minus.size());

  inject_fixed_params(theta_plus, params, fixed_idx);
  inject_random_params(u_plus_std, params, random_idx);

  Eigen::SparseMatrix<double> Hplus =
      compute_random_hessian_sparse(model, params);

  inject_fixed_params(theta_minus, params, fixed_idx);
  inject_random_params(u_minus_std, params, random_idx);

  Eigen::SparseMatrix<double> Hminus =
      compute_random_hessian_sparse(model, params);

  std::vector<double> u_base(u_hat.data(), u_hat.data() + u_hat.size());

  inject_fixed_params(theta, params, fixed_idx);
  inject_random_params(u_base, params, random_idx);

  return (Hplus - Hminus) * (0.5 / eps);
}

template <typename Model>
Eigen::SparseMatrix<double> random_hessian_directional_fd(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat, const Eigen::VectorXd &direction,
    double eps = 1e-5) {
  if (theta.size() != direction.size()) {
    throw std::invalid_argument(
        "random_hessian_directional_fd: theta and direction sizes differ");
  }

  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  Eigen::VectorXd theta_plus = theta + eps * direction;

  Eigen::VectorXd theta_minus = theta - eps * direction;

  inject_fixed_params(theta_plus, params, fixed_idx);
  inject_random_params(u, params, random_idx);

  Eigen::SparseMatrix<double> Hplus =
      compute_random_hessian_sparse(model, params);

  inject_fixed_params(theta_minus, params, fixed_idx);
  inject_random_params(u, params, random_idx);

  Eigen::SparseMatrix<double> Hminus =
      compute_random_hessian_sparse(model, params);

  const auto timing_hdot_end = std::chrono::steady_clock::now();

  // Restore baseline state for caller hygiene.
  inject_fixed_params(theta, params, fixed_idx);
  inject_random_params(u, params, random_idx);

  return (Hplus - Hminus) * (0.5 / eps);
}

//==================================================
// Finite-difference Laplace logdet gradient contribution
//
// Returns the gradient of 0.5 * log det(H_u) wrt fixed effects.
// This is intentionally written through Hdot + trace(H^{-1}Hdot)
// so exact third-order AD can replace random_hessian_directional_fd()
// later without changing this public interface.
//==================================================

//==================================================
// Exact directional derivative of H_uu using directional edge-pushing
//
// Computes:
//
//     Hdot = D H_uu(theta, u*) [theta_direction, u_direction]
//
// This is the intended replacement for:
//
//     (Hplus - Hminus) / (2 eps)
//
// and avoids finite-difference Hessian rebuilds.
//
// Requires had_quadra_hdot.hpp / updated had_quadra.h support for:
//     had::PropagateAdjointDirectional()
//     had::GetAdjointDot(...)
//==================================================
template <typename Model>
Eigen::SparseMatrix<double> random_hessian_directional_exact(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat, Eigen::Index theta_i,
    const Eigen::VectorXd &du, const SparseHessianPattern &pattern) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (theta_i < 0 || theta_i >= theta.size()) {
    throw std::out_of_range(
        "random_hessian_directional_exact: theta_i out of range");
  }

  had::ADGraph graph;
  ADScope scope(graph);

  std::vector<AD> p_full;
  p_full.reserve(static_cast<size_t>(params.size()));

  for (int i = 0; i < params.size(); ++i) {
    p_full.emplace_back(AD(0.0));
  }

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  inject_fixed_params(theta, p_full, fixed_idx);
  inject_random_params(u, p_full, random_idx);

  // Seed full primal tangent:
  // theta direction is e_i, random direction is du*/dtheta_i.
  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    const double d = (static_cast<Eigen::Index>(k) == theta_i) ? 1.0 : 0.0;

    p_full[fixed_idx[k]].dot = d;
    graph.vertices[p_full[fixed_idx[k]].varId].dot = d;
  }

  for (size_t r = 0; r < random_idx.size(); ++r) {
    const double d = du[static_cast<Eigen::Index>(r)];

    p_full[random_idx[r]].dot = d;
    graph.vertices[p_full[random_idx[r]].varId].dot = d;
  }

  AD nll = model(p_full);

  had::SetAdjoint(nll, 1.0);
  had::PropagateAdjointDirectional();

  // Ensure scope/had reads this graph.
  had::g_ADGraph = &scope.graph;

  const int n = static_cast<int>(random_idx.size());

  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(pattern.size());

  for (const auto &[i, j] : pattern) {
    const double hij_dot =
        had::GetAdjointDot(p_full[random_idx[static_cast<size_t>(i)]],
                           p_full[random_idx[static_cast<size_t>(j)]]);

    if (std::abs(hij_dot) > 1e-12) {
      triplets.emplace_back(i, j, hij_dot);
    }
  }

  Eigen::SparseMatrix<double> Hdot(n, n);
  Hdot.setFromTriplets(triplets.begin(), triplets.end());

  return Hdot;
}

template <typename Model>
std::vector<Eigen::SparseMatrix<double>> random_hessian_directional_exact_all(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat, const Eigen::MatrixXd &du_dtheta,
    const SparseHessianPattern &pattern) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (du_dtheta.rows() != u_hat.size() || du_dtheta.cols() != theta.size()) {
    throw std::invalid_argument(
        "random_hessian_directional_exact_all: du_dtheta has wrong shape");
  }

  had::ADGraph graph;
  ADScope scope(graph);

  std::vector<AD> p_full;
  p_full.reserve(static_cast<size_t>(params.size()));

  for (int i = 0; i < params.size(); ++i) {
    p_full.emplace_back(AD(0.0));
  }

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  inject_fixed_params(theta, p_full, fixed_idx);
  inject_random_params(u, p_full, random_idx);

  AD nll = model(p_full);

  had::g_ADGraph = &scope.graph;

  const int n = static_cast<int>(random_idx.size());
  std::vector<Eigen::SparseMatrix<double>> out(
      static_cast<size_t>(theta.size()));

  for (Eigen::Index theta_i = 0; theta_i < theta.size(); ++theta_i) {
    // Reset primal tangents.
    for (size_t k = 0; k < fixed_idx.size(); ++k) {
      const double d = (static_cast<Eigen::Index>(k) == theta_i) ? 1.0 : 0.0;
      p_full[static_cast<size_t>(fixed_idx[k])].dot = d;
      graph.vertices[p_full[static_cast<size_t>(fixed_idx[k])].varId].dot = d;
    }

    for (size_t r = 0; r < random_idx.size(); ++r) {
      const double d = du_dtheta(static_cast<Eigen::Index>(r), theta_i);
      p_full[static_cast<size_t>(random_idx[r])].dot = d;
      graph.vertices[p_full[static_cast<size_t>(random_idx[r])].varId].dot = d;
    }

    laplace::reset_had_quadra_directional_reverse_state(graph);
    laplace::retangent_had_quadra_graph(graph);

    had::SetAdjoint(nll, 1.0);
    had::PropagateAdjointDirectional();

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(pattern.size());

    for (const auto &[i, j] : pattern) {
      const double hij_dot = had::GetAdjointDot(
          p_full[static_cast<size_t>(random_idx[static_cast<size_t>(i)])],
          p_full[static_cast<size_t>(random_idx[static_cast<size_t>(j)])]);

      if (std::abs(hij_dot) > 1e-12) {
        triplets.emplace_back(i, j, hij_dot);
      }
    }

    Eigen::SparseMatrix<double> Hdot(n, n);
    Hdot.setFromTriplets(triplets.begin(), triplets.end());
    Hdot.makeCompressed();

    out[static_cast<size_t>(theta_i)] = std::move(Hdot);
  }

  return out;
}

template <typename Model, typename SelectedInverseAccessor>
Eigen::VectorXd random_hessian_trace_terms_exact_workspace(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat, const Eigen::MatrixXd &du_dtheta,
    const SparseHessianPattern &pattern,
    SelectedInverseAccessor &&selected_inverse) {
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  if (du_dtheta.rows() != u_hat.size() || du_dtheta.cols() != theta.size()) {
    throw std::invalid_argument("random_hessian_trace_terms_exact_workspace: "
                                "du_dtheta has wrong shape");
  }

  std::vector<laplace::SparseHdotPatternEntry> workspace_pattern;
  workspace_pattern.reserve(pattern.size());
  for (const auto &[i, j] : pattern) {
    workspace_pattern.emplace_back(i, j);
  }

  laplace::ExactGradientWorkspace workspace;
  std::vector<had::AReal> fixed_effects;
  std::vector<had::AReal> random_effects;

  auto builder = [&]() {
    fixed_effects.clear();
    random_effects.clear();

    for (Eigen::Index i = 0; i < theta.size(); ++i) {
      fixed_effects.emplace_back(theta[i]);
    }
    for (Eigen::Index i = 0; i < u_hat.size(); ++i) {
      random_effects.emplace_back(u_hat[i]);
    }

    std::vector<AD> p_full;
    p_full.reserve(static_cast<std::size_t>(params.size()));
    for (int ip = 0; ip < params.size(); ++ip) {
      p_full.emplace_back(AD(0.0));
    }

    for (std::size_t k = 0; k < fixed_idx.size(); ++k) {
      p_full[static_cast<std::size_t>(fixed_idx[k])] = fixed_effects[k];
    }
    for (std::size_t k = 0; k < random_idx.size(); ++k) {
      p_full[static_cast<std::size_t>(random_idx[k])] = random_effects[k];
    }

    return model(p_full);
  };

  workspace.Build(builder, &fixed_effects, &random_effects);

  workspace.PropagateBaseAdjoint();

  workspace.SeedTotalDirections(
      static_cast<std::size_t>(theta.size()),
      [&](std::size_t k, Eigen::VectorXd &theta_direction,
          Eigen::VectorXd &random_direction) {
        theta_direction = Eigen::VectorXd::Zero(theta.size());
        random_direction = du_dtheta.col(static_cast<Eigen::Index>(k));
        theta_direction[static_cast<Eigen::Index>(k)] = 1.0;
      });

  workspace.PropagateDirectionalBatch();

  return workspace.TraceTermsSelectedInverse(
      std::forward<SelectedInverseAccessor>(selected_inverse),
      workspace_pattern);
}

//==================================================
// Exact Laplace log-determinant gradient contribution
//
// Computes gradient of:
//
//     0.5 * log det(H_uu(theta, u*(theta)))
//
// using:
//
//     du*/dtheta_i = - H_uu^{-1} H_{u theta_i}
//
// and exact directional Hessian propagation:
//
//     Hdot_i = D H_uu [e_i, du*/dtheta_i]
//
// No finite-difference Hplus/Hminus path is used in production.
//
// Note:
// The derivative propagation is exact. The trace may still be stochastic
// if logdet_directional_derivative_from_hdot(...) uses Hutchinson probes.
//==================================================
template <typename Model>
Eigen::VectorXd laplace_logdet_gradient_exact(
    Model &model, ParameterVector &params, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &u_hat,
    const LaplaceOptions &options = default_laplace_options()) {
  const auto timing_logdet_exact_start = std::chrono::steady_clock::now();
  const auto fixed_idx = build_fixed_index(params);
  const auto random_idx = build_random_index(params);

  std::vector<double> u(u_hat.data(), u_hat.data() + u_hat.size());

  // Keep ParameterVector state synchronized with the evaluation point.
  inject_fixed_params(theta, params, fixed_idx);
  inject_random_params(u, params, random_idx);

  // --------------------------------------------------
  // Build baseline graph once.
  // This gives us:
  //   1. the graph-discovered H_uu sparsity pattern,
  //   2. the baseline H_uu numeric values,
  //   3. the matrix used for log-det trace solves.
  // --------------------------------------------------
  const auto timing_baseline_start = std::chrono::steady_clock::now();

  had::ADGraph pattern_graph;
  ADScope pattern_scope(pattern_graph);

  std::vector<AD> p_pattern;
  p_pattern.reserve(static_cast<size_t>(params.size()));

  for (int ip = 0; ip < params.size(); ++ip) {
    p_pattern.emplace_back(AD(0.0));
  }

  inject_fixed_params(theta, p_pattern, fixed_idx);
  inject_random_params(u, p_pattern, random_idx);

  AD nll_pattern = model(p_pattern);
  pattern_scope.backward(nll_pattern);

  const auto &get_pattern_for_logdet =
      get_pattern(pattern_scope, p_pattern, random_idx);

  Eigen::SparseMatrix<double> H =
      extract_sparse_hessian(pattern_scope, p_pattern, random_idx,
                             get_pattern_for_logdet, options.hessian_drop_tol);

  const auto timing_baseline_end = std::chrono::steady_clock::now();

  // --------------------------------------------------
  // Factorize H_uu.
  //
  // Adaptive jitter is applied only if the unmodified H fails.
  // --------------------------------------------------
  const auto timing_factor_start = std::chrono::steady_clock::now();

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;

  Eigen::SparseMatrix<double> H_factor = factorize_with_adaptive_jitter(
      H, solver, "laplace_logdet_gradient_exact", options);

  const auto timing_factor_end = std::chrono::steady_clock::now();

  // --------------------------------------------------
  // Compute all implicit random-effect sensitivities:
  //
  //     dU.col(i) = du*/dtheta_i
  //
  // Reuse the same H_uu factorization used for trace solves.
  // --------------------------------------------------
  const auto timing_du_start = std::chrono::steady_clock::now();

  Eigen::MatrixXd dU =
      implicit_du_dtheta_all(model, params, theta, u_hat, &H_factor, &solver);

  laplace::diagnostics::print_du_dtheta_summary(dU);

  const auto timing_du_end = std::chrono::steady_clock::now();

  const auto timing_hdot_start = std::chrono::steady_clock::now();

  // Propagate every fixed-effect direction in one batched reverse sweep.
  // The former implementation rebuilt directional reverse state and traversed
  // the complete FIMS graph once per fixed effect.
  auto selected_inverse = [&](int row, int col) -> double {
    Eigen::VectorXd e = Eigen::VectorXd::Zero(H.rows());
    e[col] = 1.0;
    Eigen::VectorXd inverse_column = solver.solve(e);
    if (solver.info() != Eigen::Success) {
      throw std::runtime_error(
          "laplace_logdet_gradient_exact: selected inverse solve failed");
    }
    return inverse_column[row];
  };

  const Eigen::VectorXd trace_terms =
      random_hessian_trace_terms_exact_workspace(
          model, params, theta, u_hat, dU, get_pattern_for_logdet,
          selected_inverse);
  Eigen::VectorXd grad = 0.5 * trace_terms;

#ifdef QUADRA_DEBUG_LOGDET_THETA_ONLY_VS_TOTAL
  {
    const Eigen::MatrixXd zero_dU =
        Eigen::MatrixXd::Zero(u_hat.size(), theta.size());

    const auto Hdots_theta_only = random_hessian_directional_exact_all(
        model, params, theta, u_hat, zero_dU, get_pattern_for_logdet);

    Eigen::VectorXd theta_only = Eigen::VectorXd::Zero(theta.size());
    for (Eigen::Index i = 0; i < theta.size(); ++i) {
      theta_only[i] =
          0.5 *
          logdet_directional_derivative_from_hdot(
              solver, Hdots_theta_only[static_cast<std::size_t>(i)], options);
    }

    laplace::diagnostics::print_theta_only_vs_total_logdet_gradient(theta_only,
                                                                    grad);
  }
#endif

#ifdef QUADRA_DEBUG_HDOT_EXACT_VS_FD_TRACE
  {
    Eigen::VectorXd fd_trace = Eigen::VectorXd::Zero(theta.size());
    Eigen::VectorXd exact_trace = Eigen::VectorXd::Zero(theta.size());
    Eigen::VectorXd rel_hdot_err = Eigen::VectorXd::Zero(theta.size());

    for (Eigen::Index i = 0; i < theta.size(); ++i) {
      const Eigen::SparseMatrix<double> Hdot_fd =
          random_hessian_directional_implicit_fd_with_du(
              model, params, theta, u_hat, i, dU.col(i), 1.0e-5);

      const Eigen::SparseMatrix<double> &Hdot_exact =
          Hdots[static_cast<std::size_t>(i)];

      fd_trace[i] = 0.5 * logdet_directional_derivative_from_hdot(
                              solver, Hdot_fd, options);
      exact_trace[i] = 0.5 * logdet_directional_derivative_from_hdot(
                                 solver, Hdot_exact, options);

      const Eigen::SparseMatrix<double> diff = Hdot_exact - Hdot_fd;
      rel_hdot_err[i] = diff.norm() / std::max(1.0e-12, Hdot_fd.norm());
    }

    laplace::diagnostics::print_hdot_exact_vs_fd_trace(exact_trace, fd_trace,
                                                       rel_hdot_err);
  }
#endif

  const auto timing_hdot_end = std::chrono::steady_clock::now();

#ifdef QUADRA_VALIDATE_EXACT_GRADIENT_WORKSPACE
  auto selected_inverse = [&](int row, int col) -> double {
    Eigen::VectorXd e = Eigen::VectorXd::Zero(H.rows());
    e[col] = 1.0;
    Eigen::VectorXd x = solver.solve(e);
    return x[row];
  };

  const Eigen::VectorXd workspace_trace =
      random_hessian_trace_terms_exact_workspace(model, params, theta, u_hat,
                                                 dU, get_pattern_for_logdet,
                                                 selected_inverse);

  Eigen::VectorXd trusted_trace = Eigen::VectorXd::Zero(theta.size());
  for (Eigen::Index ii = 0; ii < theta.size(); ++ii) {
    trusted_trace[ii] = 2.0 * grad[ii];
  }

  const double workspace_rel_err = (workspace_trace - trusted_trace).norm() /
                                   std::max(1.0e-12, trusted_trace.norm());

  std::cout << "ExactGradientWorkspace trace rel_err=" << workspace_rel_err
            << " workspace_norm=" << workspace_trace.norm()
            << " trusted_norm=" << trusted_trace.norm() << "\n";
#endif

  // Restore baseline state for caller hygiene.
  inject_fixed_params(theta, params, fixed_idx);
  inject_random_params(u, params, random_idx);

  const auto timing_logdet_exact_end = std::chrono::steady_clock::now();
  const double total_ms =
      std::chrono::duration<double, std::milli>(timing_logdet_exact_end -
                                                timing_logdet_exact_start)
          .count();
  const double baseline_ms = std::chrono::duration<double, std::milli>(
                                 timing_baseline_end - timing_baseline_start)
                                 .count();
  const double factor_ms = std::chrono::duration<double, std::milli>(
                               timing_factor_end - timing_factor_start)
                               .count();
  const double du_ms =
      std::chrono::duration<double, std::milli>(timing_du_end - timing_du_start)
          .count();
  const double hdot_ms = std::chrono::duration<double, std::milli>(
                             timing_hdot_end - timing_hdot_start)
                             .count();

#ifdef QUADRA_PROFILE_LAPLACE_LOGDET_GRADIENT
  std::cout << "laplace_logdet_gradient_exact ms = " << total_ms
            << " baseline=" << baseline_ms << " factor=" << factor_ms
            << " du=" << du_ms << " hdot_trace=" << hdot_ms << "\n";
#endif
  return grad;
}

// Backward-compatible wrapper.
// Deprecated name: the default path is exact-Hdot, not finite-difference.
template <typename Model>
Eigen::VectorXd laplace_logdet_gradient_fd(Model &model,
                                           ParameterVector &params,
                                           const Eigen::VectorXd &theta,
                                           const Eigen::VectorXd &u_hat,
                                           double /*eps*/ = 1e-5) {
  return laplace_logdet_gradient_exact(model, params, theta, u_hat,
                                       default_laplace_options());
}

template <typename Model> struct LaplaceResult {

  // Component breakdown of the Laplace objective:
  //
  //   value = joint_objective + 0.5 * laplace_logdet - laplace_constant
  //
  // These are intentionally stored for diagnostics/reporting and for
  // optimizer-side bookkeeping. They do not change the objective math.
  double joint_objective = 0.0;
  double laplace_logdet = 0.0;
  double laplace_constant = 0.0;

  double value;
  std::vector<double> grad_x;
  std::vector<double> grad_u;
};

template <typename Model>
LaplaceResult<Model> laplace_eval_at_u_star(
    Model &model, ParameterVector &params, const std::vector<int> &fixed_idx,
    const std::vector<int> &random_idx, const Eigen::VectorXd &x,
    const std::vector<double> &u_star, had::ADGraph &graph,
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

  AD nll = model(p_full);

  scope.backward(nll);

  res.grad_x.resize(fixed_idx.size());
  for (size_t k = 0; k < fixed_idx.size(); ++k) {
    res.grad_x[k] = scope.grad(p_full[fixed_idx[k]]);
  }

  // Add fixed-effect contribution from 0.5 * log det(H_u).
  // This is currently finite-difference based through Hdot + trace(H^{-1}Hdot).
  {
    Eigen::Map<const Eigen::VectorXd> u_star_eigen(
        u_star.data(), static_cast<Eigen::Index>(u_star.size()));

    Eigen::VectorXd g_logdet =
        laplace_logdet_gradient_exact(model, params, x, u_star_eigen, options);

    // laplace_logdet_gradient_exact builds temporary AD graphs.
    // Restore the graph for this outer evaluation before any
    // further grad/hess access through scope.
    had::g_ADGraph = &scope.graph;

    for (size_t k = 0; k < fixed_idx.size(); ++k) {
      res.grad_x[k] += g_logdet[static_cast<Eigen::Index>(k)];
    }
  }

  res.grad_u.resize(random_idx.size());
  for (size_t k = 0; k < random_idx.size(); ++k) {
    res.grad_u[k] = scope.grad(p_full[random_idx[k]]);
  }

  const auto &pattern = get_pattern(scope, p_full, random_idx);

  Eigen::SparseMatrix<double> H =
      extract_sparse_hessian(scope, p_full, random_idx, pattern);

  double logdet = sparse_logdet_ldlt(H);
  // Or, if vectorD() is unavailable:
  // double logdet = sparse_logdet_llt(H);

  const double laplace_constant =
      0.5 * static_cast<double>(random_idx.size()) * std::log(2.0 * M_PI);

  res.value = value_of(nll) + 0.5 * logdet - laplace_constant;

  return res;
}

#ifndef QUADRA_USE_ORIGINAL_HAD
//==================================================
// Optional third-order directional diagnostic.
// This evaluates D^k f(x)[direction,...] for k = 0,1,2,3
// using the scalar-templated model path. It is intentionally
// separate from LBFGS/Laplace so it can be enabled only when needed.
//==================================================
template <typename Model>
ThirdDirectionalResult
third_directional_fixed_effects(Model &model, const Eigen::VectorXd &x,
                                const Eigen::VectorXd &direction) {
  if (x.size() != direction.size())
    throw std::invalid_argument(
        "third_directional_fixed_effects: x and direction must have same size");

  std::vector<double> xv(static_cast<size_t>(x.size()));
  std::vector<double> dv(static_cast<size_t>(direction.size()));
  for (int i = 0; i < x.size(); ++i) {
    xv[static_cast<size_t>(i)] = x[i];
    dv[static_cast<size_t>(i)] = direction[i];
  }

  return evaluate_third_directional(
      [&](const std::vector<AD3> &x_ad3) -> AD3 { return model(x_ad3); }, xv,
      dv);
}
#endif

} // namespace quadra

#endif // QUADRA_LAPLACE_HPP
