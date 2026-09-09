#pragma once

#include <Eigen/Dense>

#include <cstddef>
#include <utility>
#include <vector>

#include "exact_gradient_workspace.hpp"

namespace quadra {
namespace laplace {

struct LaplaceExactGradientEvaluation {
  double objective = 0.0;
  Eigen::VectorXd gradient;
  Eigen::VectorXd trace_terms;
};

// Callback-driven evaluator scaffold over ExactGradientWorkspace.
//
// v1 intentionally avoids imposing a model concept. Existing evaluators can
// provide lambdas for graph construction, direction seeding, selected inverse
// access, and Hdot pattern extraction.
class LaplaceExactGradientEvaluator {
public:
  LaplaceExactGradientEvaluator() = default;

  LaplaceExactGradientEvaluator(const LaplaceExactGradientEvaluator &) = delete;
  LaplaceExactGradientEvaluator &
  operator=(const LaplaceExactGradientEvaluator &) = delete;

  LaplaceExactGradientEvaluator(LaplaceExactGradientEvaluator &&) = default;
  LaplaceExactGradientEvaluator &
  operator=(LaplaceExactGradientEvaluator &&) = default;

  template <class Builder, class DirectionProvider,
            class SelectedInverseAccessor>
  LaplaceExactGradientEvaluation
  Evaluate(Builder &&builder, std::vector<had::AReal> *fixed_effects,
           std::vector<had::AReal> *random_effects, std::size_t n_directions,
           DirectionProvider &&direction_provider,
           SelectedInverseAccessor &&selected_inverse,
           const std::vector<SparseHdotPatternEntry> &hdot_pattern,
           double joint_objective, double logdet_huu,
           const Eigen::VectorXd &joint_envelope_gradient) {
    workspace_.Build(std::forward<Builder>(builder), fixed_effects,
                     random_effects);

    workspace_.PropagateBaseAdjoint();

    workspace_.SeedTotalDirections(
        n_directions, std::forward<DirectionProvider>(direction_provider));

    workspace_.PropagateDirectionalBatch();

    const ExactGradientEvaluation assembled = workspace_.AssembleExactGradient(
        joint_objective, logdet_huu, joint_envelope_gradient,
        std::forward<SelectedInverseAccessor>(selected_inverse), hdot_pattern);

    LaplaceExactGradientEvaluation out;
    out.objective = assembled.objective;
    out.gradient = assembled.gradient;
    out.trace_terms = assembled.trace_terms;
    return out;
  }

  ExactGradientWorkspace &Workspace() { return workspace_; }
  const ExactGradientWorkspace &Workspace() const { return workspace_; }

private:
  ExactGradientWorkspace workspace_;
};

} // namespace laplace
} // namespace quadra
