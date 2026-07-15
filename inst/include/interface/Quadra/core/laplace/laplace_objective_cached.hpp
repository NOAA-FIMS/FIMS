#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "laplace_objective.hpp"
#include "sparse_factorization_cache.hpp"

namespace quadra {

struct CachedLaplaceObjectiveState {
  SparseFactorizationCache factorization_m;
  bool analyzed_m = false;
};

struct CachedLaplaceObjectiveOptions {
  LaplaceObjectiveOptions objective_m;
  bool reuse_symbolic_factorization_m = true;
};

inline double
cached_laplace_logdet(const Eigen::SparseMatrix<double> &H,
                      CachedLaplaceObjectiveState &state,
                      const CachedLaplaceObjectiveOptions &options) {
  if (!options.reuse_symbolic_factorization_m) {
    SparseFactorizationCache cache;
    cache.compute(H);
    return cache.logdet();
  }

  if (!state.analyzed_m) {
    state.factorization_m.analyze_pattern(H);
    state.analyzed_m = true;
  }

  state.factorization_m.factorize(H);

  return state.factorization_m.logdet();
}

template <class Model>
inline LaplaceObjectiveResult
evaluate_laplace_objective_cached(Model &model,
                                  const std::vector<double> &fixed,
                                  const std::vector<double> &random_initial,
                                  const ParameterPartition &partition,
                                  CachedLaplaceObjectiveState &cache_state,
                                  const CachedLaplaceObjectiveOptions &options =
                                      CachedLaplaceObjectiveOptions()) {
  // First reuse the existing path to solve u_hat and compute H_uu.
  //
  // Then replace the logdet path with cached symbolic factorization.
  //
  // This is intentionally conservative: it validates cache integration
  // without changing Newton/Hessian behavior.
  LaplaceObjectiveOptions base_options = options.objective_m;

  LaplaceObjectiveResult result = evaluate_laplace_objective(
      model, fixed, random_initial, partition, base_options);

  if (!result.converged_m || !result.logdet_ok_m) {
    return result;
  }

  try {
    result.log_det_hessian_m =
        cached_laplace_logdet(result.hessian_random_m, cache_state, options);

    const double constant =
        base_options.include_constant_m
            ? 0.5 * static_cast<double>(partition.random_indices_m.size()) *
                  std::log(2.0 * M_PI)
            : 0.0;

    // Existing convention in laplace_objective.hpp:
    // laplace objective = joint + 0.5*logdet - constant
    result.laplace_objective_m =
        result.joint_objective_m + 0.5 * result.log_det_hessian_m - constant;

    result.logdet_ok_m = true;
  } catch (const std::exception &e) {
    result.logdet_ok_m = false;
    result.message_m = std::string("Cached logdet failed: ") + e.what();
  }

  return result;
}

template <class Model>
inline LaplaceObjectiveResult evaluate_laplace_objective_cached(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random_initial, const ParameterSet &parameters,
    CachedLaplaceObjectiveState &cache_state,
    const CachedLaplaceObjectiveOptions &options =
        CachedLaplaceObjectiveOptions()) {
  return evaluate_laplace_objective_cached(model, fixed, random_initial,
                                           partition_parameters(parameters),
                                           cache_state, options);
}

} // namespace quadra
