#pragma once

#include "random_effect_hessian.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace quadra {
namespace laplace {

// Public-stats adapter over Quadra's promoted low-memory Hessian workspace.
// It keeps the recorded graph and restricted propagation plan persistent while
// retaining the direct-value guard required for parameter-dependent topology.
template <class Model> class ReusableRandomEffectTape {
public:
  ReusableRandomEffectTape(Model &model, const std::vector<double> &fixed,
                           const std::vector<double> &random,
                           const ParameterPartition &partition,
                           bool validate_topology = true,
                           bool collect_reports = true,
                           bool compute_mixed_derivatives = true)
      : model_(&model), partition_(partition), fixed_size_(fixed.size()),
        random_size_(random.size()), validate_topology_(validate_topology),
        collect_reports_(collect_reports),
        compute_mixed_derivatives_(compute_mixed_derivatives) {
    if (fixed_size_ != partition_.fixed_indices_m.size() ||
        random_size_ != partition_.random_indices_m.size()) {
      throw std::invalid_argument(
          "ReusableRandomEffectTape: parameter dimensions do not match");
    }
    record(fixed, random);
  }

  RandomEffectHessianResult evaluate(const std::vector<double> &fixed,
                                     const std::vector<double> &random,
                                     double drop_tol = 0.0) {
    return evaluate_impl(fixed, random, drop_tol, true);
  }

  std::size_t rebuild_count() const { return rebuild_count_; }

private:
  Model *model_;
  ParameterPartition partition_;
  std::size_t fixed_size_;
  std::size_t random_size_;
  std::unique_ptr<RandomEffectHessianWorkspace<Model>> workspace_;
  std::size_t rebuild_count_ = 0;
  bool topology_probe_validated_ = false;
  bool validate_topology_ = true;
  bool collect_reports_ = true;
  bool compute_mixed_derivatives_ = true;
  std::vector<double> topology_probe_fixed_;

  void record(const std::vector<double> &fixed,
              const std::vector<double> &random) {
    workspace_.reset(new RandomEffectHessianWorkspace<Model>(
        *model_, fixed, random, partition_, collect_reports_));
    topology_probe_validated_ = false;
    topology_probe_fixed_ = fixed;
  }

  double direct_value(const std::vector<double> &fixed,
                      const std::vector<double> &random) const {
    ModelReportContext context;
    model_->initialize(context);
    return evaluate_fixed_random<Model, double>(*model_, fixed, random,
                                                partition_, context);
  }

  RandomEffectHessianResult evaluate_impl(const std::vector<double> &fixed,
                                          const std::vector<double> &random,
                                          double drop_tol, bool allow_rebuild) {
    if (fixed.size() != fixed_size_ || random.size() != random_size_) {
      throw std::invalid_argument(
          "ReusableRandomEffectTape::evaluate: parameter dimensions changed");
    }
    if (fixed != topology_probe_fixed_) {
      topology_probe_validated_ = false;
      topology_probe_fixed_ = fixed;
    }
    RandomEffectHessianResult result =
        workspace_->Evaluate(fixed, random, drop_tol);
    if (!validate_topology_) {
      if (compute_mixed_derivatives_) {
        const auto fixed_mixed =
            workspace_->EvaluateFixedGradientMixedHessian(fixed, random);
        result.gradient_fixed_m.assign(
            fixed_mixed.fixed_gradient_m.data(),
            fixed_mixed.fixed_gradient_m.data() +
                fixed_mixed.fixed_gradient_m.size());
        result.mixed_hessian_m = fixed_mixed.mixed_hessian_m;
      }
      return result;
    }
    const double direct = direct_value(fixed, random);
    const double scale =
        1.0 + std::max(std::abs(direct), std::abs(result.objective_value_m));
    bool stale = std::abs(direct - result.objective_value_m) > 1e-11 * scale;

    if (allow_rebuild && !stale && !topology_probe_validated_) {
      std::vector<double> probe = random;
      for (std::size_t i = 0; i < probe.size(); ++i) {
        probe[i] += 0.137 * static_cast<double>(i + 1) /
                    static_cast<double>(probe.size() + 1);
      }
      const auto replay_probe = workspace_->Evaluate(fixed, probe, drop_tol);
      const double direct_probe = direct_value(fixed, probe);
      const double probe_scale =
          1.0 + std::max(std::abs(direct_probe),
                         std::abs(replay_probe.objective_value_m));
      stale = std::abs(direct_probe - replay_probe.objective_value_m) >
              1e-11 * probe_scale;
      topology_probe_validated_ = !stale;
      result = workspace_->Evaluate(fixed, random, drop_tol);
      if (topology_probe_validated_)
        (void)workspace_->FreezeHessianTopology();
    }

    if (allow_rebuild && stale) {
      record(fixed, random);
      ++rebuild_count_;
      return evaluate_impl(fixed, random, drop_tol, false);
    }

    if (compute_mixed_derivatives_) {
      const auto fixed_mixed =
          workspace_->EvaluateFixedGradientMixedHessian(fixed, random);
      result.gradient_fixed_m.assign(fixed_mixed.fixed_gradient_m.data(),
                                     fixed_mixed.fixed_gradient_m.data() +
                                         fixed_mixed.fixed_gradient_m.size());
      result.mixed_hessian_m = fixed_mixed.mixed_hessian_m;
    }
    return result;
  }
};

} // namespace laplace
} // namespace quadra
