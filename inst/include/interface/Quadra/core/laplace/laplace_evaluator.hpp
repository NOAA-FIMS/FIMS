#pragma once

#include "persistent_latent_state_runtime.hpp"

namespace quadra {
namespace laplace {

template <class ModelContext, class SolverPolicy, class EvaluationPolicy>
class LaplaceEvaluator {
public:
  LaplaceEvaluator() = default;

  explicit LaplaceEvaluator(
      SolverPolicy solver_policy,
      EvaluationPolicy evaluation_policy = EvaluationPolicy())
      : solver_policy_(solver_policy), evaluation_policy_(evaluation_policy) {}

  auto evaluate(ModelContext &context) {
    auto solve_result = runtime_.solve_random_effects(context, solver_policy_);

    auto eval_result = evaluation_policy_.evaluate(
        context, runtime_.random_effects().xhat(), runtime_.structured());

    return eval_result;
  }

  void clear() { runtime_.clear(); }

  PersistentLatentStateRuntime &runtime() { return runtime_; }

  const PersistentLatentStateRuntime &runtime() const { return runtime_; }

  SolverPolicy &solver_policy() { return solver_policy_; }

  EvaluationPolicy &evaluation_policy() { return evaluation_policy_; }

private:
  PersistentLatentStateRuntime runtime_;
  SolverPolicy solver_policy_;
  EvaluationPolicy evaluation_policy_;
};

} // namespace laplace
} // namespace quadra
