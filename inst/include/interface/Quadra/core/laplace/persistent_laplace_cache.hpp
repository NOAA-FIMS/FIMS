#pragma once

#include <Eigen/Dense>

#include <cmath>
#include <stdexcept>
#include <string>

namespace quadra {
namespace laplace {

struct LaplaceEvaluation {
  double marginal = 0.0;
  double joint = 0.0;
  double logdet = 0.0;
  double correction = 0.0;
  double grad_norm = 0.0;
  int random_size = 0;
  int nnz_hessian = 0;
  std::string backend = "unknown";
};

struct LaplaceCacheState {
  bool initialized = false;
  Eigen::VectorXd uhat;
  int cold_evaluations = 0;
  int warm_evaluations = 0;
  int cached_evaluations = 0;
};

template <class ModelAdapter> class PersistentLaplaceCache {
public:
  explicit PersistentLaplaceCache(ModelAdapter adapter)
      : adapter_(std::move(adapter)) {
    state_.uhat = adapter_.initial_random();
    if (state_.uhat.size() != adapter_.random_size()) {
      throw std::invalid_argument("PersistentLaplaceCache: initial_random size "
                                  "does not match random_size");
    }
  }

  const LaplaceCacheState &state() const { return state_; }

  const Eigen::VectorXd &uhat() const {
    if (!state_.initialized) {
      throw std::runtime_error(
          "PersistentLaplaceCache: uhat requested before initialization");
    }
    return state_.uhat;
  }

  void reset() {
    state_.initialized = false;
    state_.uhat = adapter_.initial_random();
  }

  // Full cold evaluation from adapter-provided initial random effects.
  LaplaceEvaluation evaluate_cold() {
    state_.uhat = adapter_.solve_random_effects(adapter_.initial_random());
    state_.initialized = true;
    ++state_.cold_evaluations;
    return adapter_.evaluate_at_random(state_.uhat);
  }

  // Warm-start solve from the previously cached uhat.
  LaplaceEvaluation evaluate_warm() {
    if (!state_.initialized) {
      return evaluate_cold();
    }

    state_.uhat = adapter_.solve_random_effects(state_.uhat);
    ++state_.warm_evaluations;
    return adapter_.evaluate_at_random(state_.uhat);
  }

  // Evaluate Laplace at cached uhat without re-solving.
  LaplaceEvaluation evaluate_cached_no_solve() {
    if (!state_.initialized) {
      throw std::runtime_error("PersistentLaplaceCache: cached evaluation "
                               "requested before initialization");
    }

    ++state_.cached_evaluations;
    return adapter_.evaluate_at_random(state_.uhat);
  }

  // Common production default:
  //   first call cold-solves,
  //   subsequent calls warm-start.
  LaplaceEvaluation evaluate() {
    if (!state_.initialized) {
      return evaluate_cold();
    }
    return evaluate_warm();
  }

  ModelAdapter &adapter() { return adapter_; }
  const ModelAdapter &adapter() const { return adapter_; }

private:
  ModelAdapter adapter_;
  LaplaceCacheState state_;
};

} // namespace laplace
} // namespace quadra
