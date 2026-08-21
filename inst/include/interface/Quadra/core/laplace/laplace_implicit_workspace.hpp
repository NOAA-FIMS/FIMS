#ifndef QUADRA_LAPLACE_IMPLICIT_WORKSPACE_HPP
#define QUADRA_LAPLACE_IMPLICIT_WORKSPACE_HPP

#include "laplace_implicit_derivatives.hpp"
#include "sparse_factorization_cache.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace quadra {

struct LaplaceImplicitWorkspace {
  Eigen::MatrixXd H_u_theta_m;
  Eigen::MatrixXd du_dtheta_m;

  Eigen::SparseMatrix<double> H_uu_m;

  std::shared_ptr<SparseLDLTFactorizationCache> factorization_m =
      std::make_shared<SparseLDLTFactorizationCache>();

  std::vector<double> fixed_m;
  std::vector<double> u_hat_m;
  std::vector<double> full_m;

  double implicit_derivatives_ms_m = 0.0;
  double factorization_ms_m = 0.0;
  double total_ms_m = 0.0;

  bool converged_m = false;
  bool success_m = false;

  std::string message_m;
};

template <class Model>
inline LaplaceImplicitWorkspace build_laplace_implicit_workspace(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial,
    const ParameterPartition &partition,
    const LaplaceImplicitDerivativeOptions &options =
        LaplaceImplicitDerivativeOptions()) {
  LaplaceImplicitWorkspace result;

  using clock = std::chrono::steady_clock;

  const auto total_start = clock::now();

  const auto implicit_start = clock::now();

  auto implicit = evaluate_laplace_implicit_derivatives(
      model, fixed, random_initial, partition, options);

  const auto implicit_end = clock::now();

  result.implicit_derivatives_ms_m =
      std::chrono::duration<double, std::milli>(implicit_end - implicit_start)
          .count();

  result.H_u_theta_m = implicit.H_u_theta_m;
  result.du_dtheta_m = implicit.du_dtheta_m;
  result.H_uu_m = implicit.H_uu_m;

  result.fixed_m = implicit.fixed_m;
  result.u_hat_m = implicit.u_hat_m;
  result.full_m = implicit.full_m;

  result.converged_m = implicit.converged_m;

  if (!implicit.success_m) {
    result.success_m = false;
    result.message_m = implicit.message_m;
    return result;
  }

  try {
    const auto factorization_start = clock::now();

    result.factorization_m->analyze_pattern(result.H_uu_m);

    result.factorization_m->factorize(result.H_uu_m);

    const auto factorization_end = clock::now();

    result.factorization_ms_m = std::chrono::duration<double, std::milli>(
                                    factorization_end - factorization_start)
                                    .count();
  } catch (const std::exception &e) {
    result.success_m = false;
    result.message_m =
        std::string("Failed building implicit derivative workspace: ") +
        e.what();

    return result;
  }

  const auto total_end = clock::now();

  result.total_ms_m =
      std::chrono::duration<double, std::milli>(total_end - total_start)
          .count();

  result.success_m = true;
  result.message_m = "Laplace implicit workspace built successfully.";

  return result;
}

template <class Model>
inline LaplaceImplicitWorkspace build_laplace_implicit_workspace(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    const LaplaceImplicitDerivativeOptions &options =
        LaplaceImplicitDerivativeOptions()) {
  return build_laplace_implicit_workspace(
      model, fixed, random_initial, partition_parameters(parameters), options);
}

} // namespace quadra

#endif
