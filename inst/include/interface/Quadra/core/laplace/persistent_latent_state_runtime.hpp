#pragma once

#include "persistent_random_effect_state.hpp"
#include "persistent_structured_runtime.hpp"

namespace quadra {
namespace laplace {

class PersistentLatentStateRuntime {
public:
  PersistentLatentStateRuntime() = default;

  void clear() {
    random_effects_.clear();
    structured_.clear();
  }

  bool has_random_effects() const { return random_effects_.initialized(); }

  bool has_structure() const { return structured_.initialized; }

  PersistentRandomEffectState &random_effects() { return random_effects_; }

  const PersistentRandomEffectState &random_effects() const {
    return random_effects_;
  }

  template <class ModelContext, class SolverPolicy>
  auto solve_random_effects(ModelContext &context, SolverPolicy &solver) {
    Eigen::VectorXd initial_x;

    if (random_effects_.initialized()) {
      initial_x = random_effects_.xhat();
    } else {
      initial_x = solver.initial_x(context);
    }

    auto result = solver.solve(context, initial_x);

    random_effects_.update_xhat(result.xhat, result.status);

    return result;
  }

  PersistentStructuredRuntimeState &structured() { return structured_; }

  const PersistentStructuredRuntimeState &structured() const {
    return structured_;
  }

private:
  PersistentRandomEffectState random_effects_;
  PersistentStructuredRuntimeState structured_;
};

} // namespace laplace
} // namespace quadra
