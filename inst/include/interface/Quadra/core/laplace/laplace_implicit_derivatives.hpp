#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

#include "laplace_objective.hpp"
#include "random_effect_hessian.hpp"
#include "sparse_factorization_cache.hpp"
#include "../autodiff/laplace_graph_plan.hpp"

namespace quadra {

struct LaplaceImplicitDerivativeOptions {
  double hessian_drop_tol_m = 0.0;
};

struct LaplaceImplicitDerivativeResult {
  Eigen::MatrixXd H_u_theta_m;
  Eigen::MatrixXd du_dtheta_m;

  Eigen::SparseMatrix<double> H_uu_m;

  std::vector<double> fixed_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;

  bool converged_m = false;
  bool success_m = false;

  std::string message_m;
};

// Dense finite-difference mixed Hessian wrt:
// rows: random effects u
// cols: fixed effects theta

template <class Model>
inline Eigen::MatrixXd ad_mixed_hessian(Model &model,
                                        const std::vector<double> &fixed,
                                        const std::vector<double> &random,
                                        const ParameterPartition &partition) {
  const size_t n_fixed = fixed.size();
  const size_t n_random = random.size();

  if (partition.fixed_indices_m.size() != n_fixed) {
    throw std::invalid_argument(
        "ad_mixed_hessian: fixed vector has incorrect length");
  }

  if (partition.random_indices_m.size() != n_random) {
    throw std::invalid_argument(
        "ad_mixed_hessian: random vector has incorrect length");
  }

  const size_t n_total = n_fixed + n_random;

  std::vector<double> full(n_total, 0.0);

  for (size_t i = 0; i < n_fixed; ++i) {
    full[static_cast<size_t>(partition.fixed_indices_m[i])] = fixed[i];
  }

  for (size_t i = 0; i < n_random; ++i) {
    full[static_cast<size_t>(partition.random_indices_m[i])] = random[i];
  }

  had::ADGraph graph;
  had::g_ADGraph = &graph;

  ModelReportContext ctx;
  model.initialize(ctx);

  std::vector<AD> full_ad;
  full_ad.reserve(full.size());

  for (double x : full) {
    full_ad.emplace_back(x);
  }

  AD objective = model.template evaluate<AD>(full_ad, ctx);

  std::vector<had::VertexId> fixed_ids;
  std::vector<had::VertexId> random_ids;
  fixed_ids.reserve(n_fixed);
  random_ids.reserve(n_random);
  for (size_t i = 0; i < n_fixed; ++i)
    fixed_ids.push_back(
        full_ad[static_cast<size_t>(partition.fixed_indices_m[i])].varId);
  for (size_t i = 0; i < n_random; ++i)
    random_ids.push_back(
        full_ad[static_cast<size_t>(partition.random_indices_m[i])].varId);

  LaplaceGraphPlan graph_plan;
  graph_plan.Build(graph, fixed_ids, random_ids, objective.varId);
  PropagateLaplaceHessianRestricted(graph, graph_plan);

  Eigen::MatrixXd H = Eigen::MatrixXd::Zero(static_cast<Eigen::Index>(n_random),
                                            static_cast<Eigen::Index>(n_fixed));

  for (size_t i = 0; i < n_random; ++i) {
    const int random_full_index = partition.random_indices_m[i];

    for (size_t j = 0; j < n_fixed; ++j) {
      const int fixed_full_index = partition.fixed_indices_m[j];

      H(static_cast<Eigen::Index>(i), static_cast<Eigen::Index>(j)) =
          had::GetAdjoint(full_ad[static_cast<size_t>(random_full_index)],
                          full_ad[static_cast<size_t>(fixed_full_index)]);
    }
  }

  return H;
}

template <class Model>
inline Eigen::MatrixXd
finite_difference_mixed_hessian(Model &model, const std::vector<double> &fixed,
                                const std::vector<double> &random,
                                const ParameterPartition &partition,
                                double eps = 1e-6) {
  const size_t n_random = random.size();
  const size_t n_fixed = fixed.size();

  Eigen::MatrixXd H(n_random, n_fixed);

  for (size_t j = 0; j < n_fixed; ++j) {
    std::vector<double> theta_plus = fixed;
    std::vector<double> theta_minus = fixed;

    theta_plus[j] += eps;
    theta_minus[j] -= eps;

    auto g_plus =
        evaluate_random_effect_hessian(model, theta_plus, random, partition);

    auto g_minus =
        evaluate_random_effect_hessian(model, theta_minus, random, partition);

    for (size_t i = 0; i < n_random; ++i) {
      H(static_cast<int>(i), static_cast<int>(j)) =
          (g_plus.gradient_random_m[i] - g_minus.gradient_random_m[i]) /
          (2.0 * eps);
    }
  }

  return H;
}

template <class Model>
inline LaplaceImplicitDerivativeResult evaluate_laplace_implicit_derivatives(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const LaplaceImplicitDerivativeOptions &options =
        LaplaceImplicitDerivativeOptions()) {
  LaplaceImplicitDerivativeResult result;

  auto laplace =
      evaluate_laplace_objective(model, fixed, random_initial, partition);

  result.fixed_m = fixed;
  result.u_hat_m = laplace.u_hat_m;
  result.full_m = laplace.full_m;
  result.H_uu_m = laplace.hessian_random_m;
  result.converged_m = laplace.converged_m;

  if (!laplace.converged_m) {
    result.message_m = "Laplace objective failed.";
    return result;
  }

  result.H_u_theta_m =
      ad_mixed_hessian(model, fixed, laplace.u_hat_m, partition);

  SparseLDLTFactorizationCache factorization;

  try {
    factorization.analyze_pattern(laplace.hessian_random_m);

    factorization.factorize(laplace.hessian_random_m);

    result.du_dtheta_m = -factorization.solve(result.H_u_theta_m);
  } catch (const std::exception &e) {
    result.message_m =
        std::string("Cached sparse LDLT implicit derivative solve failed: ") +
        e.what();

    return result;
  }

  result.success_m = true;
  result.message_m = "Implicit derivatives computed.";

  return result;
}

template <class Model>
inline LaplaceImplicitDerivativeResult evaluate_laplace_implicit_derivatives(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const LaplaceImplicitDerivativeOptions &options =
        LaplaceImplicitDerivativeOptions()) {
  return evaluate_laplace_implicit_derivatives(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra
