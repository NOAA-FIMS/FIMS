#pragma once

#include "had_quadra_replay_reuse_lazy_implicit_hdot_provider.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace quadra {
namespace laplace {

struct ReusableHdotTimings {
  double validation_ms = 0.0;
  double direction_setup_ms = 0.0;
  double reverse_ms = 0.0;
  double contraction_ms = 0.0;
};

// Public-stats adapter over the promoted restricted reverse-Hessian engine.
// Each worker records independent mutable graph state and may share Quadra's
// immutable Hessian topology with its prototype.
template <class CombinedObjectiveFn> class ReusableTotalHdotTape {
public:
  using DirectionProvider = std::function<Eigen::VectorXd(int)>;
  using Provider =
      HadQuadraReplayReuseLazyImplicitHdotProvider<CombinedObjectiveFn,
                                                   DirectionProvider>;

  ReusableTotalHdotTape(CombinedObjectiveFn objective, int theta_dim,
                        int random_dim, RandomHessianPattern pattern,
                        std::vector<int> active_directions,
                        const Eigen::VectorXd &theta,
                        const Eigen::VectorXd &uhat, double drop_tol = 0.0)
      : objective_fn_(std::move(objective)), theta_dim_(theta_dim),
        random_dim_(random_dim), pattern_(std::move(pattern)),
        active_directions_(std::move(active_directions)), drop_tol_(drop_tol) {
    validate_active_directions();
    build_provider(theta, uhat, nullptr);
  }

  ReusableTotalHdotTape(ReusableTotalHdotTape &prototype,
                        std::vector<int> active_directions)
      : objective_fn_(prototype.objective_fn_),
        theta_dim_(prototype.theta_dim_), random_dim_(prototype.random_dim_),
        pattern_(prototype.pattern_),
        active_directions_(std::move(active_directions)),
        drop_tol_(prototype.drop_tol_) {
    validate_active_directions();
    build_provider(prototype.discovery_theta_, prototype.discovery_uhat_,
                   prototype.shared_topology_);
  }

  template <class UDirectionProvider>
  std::vector<Eigen::SparseMatrix<double>>
  compute(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
          UDirectionProvider &&u_direction_provider) {
    ensure_topology_current(theta, uhat);
    provider_->set_u_direction_provider(DirectionProvider(
        std::forward<UDirectionProvider>(u_direction_provider)));
    return provider_->compute_all_sparse(theta, uhat);
  }

  template <class UDirectionProvider, class SelectedInverseAccessor>
  std::vector<double>
  compute_trace_terms(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
                      UDirectionProvider &&u_direction_provider,
                      SelectedInverseAccessor &&selected_inverse) {
    using Clock = std::chrono::steady_clock;
    last_timings_ = {};
    DirectionProvider directions(
        std::forward<UDirectionProvider>(u_direction_provider));
    ensure_topology_current(theta, uhat);
    provider_->set_u_direction_provider(directions);
    const auto start = Clock::now();
    const auto contraction = provider_->compute_trace_contraction(
        theta, uhat, std::forward<SelectedInverseAccessor>(selected_inverse));
    last_timings_.reverse_ms =
        std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    std::vector<double> traces(static_cast<std::size_t>(theta_dim_), 0.0);
    for (int j : active_directions_) {
      double value = contraction.input_gradient[j];
      const Eigen::VectorXd u_direction = directions(j);
      value += contraction.input_gradient.tail(random_dim_).dot(u_direction);
      // The restricted reverse contraction is seeded with 0.5 * H^{-1}
      // because its native workspace returns the log-determinant gradient
      // contribution directly.  This adapter's established contract is the
      // unscaled trace; ExactLaplaceEvaluator applies the outer 0.5.
      traces[static_cast<std::size_t>(j)] = 2.0 * value;
    }
    return traces;
  }

  std::size_t rebuild_count() const { return rebuild_count_; }
  std::size_t shared_topology_owner_count() const {
    // Each worker owns one adapter handle and one graph handle to the same
    // immutable topology, so report logical worker owners rather than raw
    // shared_ptr handles.
    return shared_topology_
               ? (static_cast<std::size_t>(shared_topology_.use_count()) + 1U) /
                     2U
               : 1U;
  }
  std::size_t shared_operation_owner_count() const {
    return shared_topology_owner_count();
  }
  std::size_t operation_count() const { return 0; }
  const ReusableHdotTimings &last_timings() const { return last_timings_; }

  void set_active_directions(std::vector<int> active_directions) {
    active_directions_ = std::move(active_directions);
    validate_active_directions();
    build_provider(discovery_theta_, discovery_uhat_, shared_topology_);
  }

private:
  CombinedObjectiveFn objective_fn_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  double drop_tol_;
  Eigen::VectorXd discovery_theta_;
  Eigen::VectorXd discovery_uhat_;
  std::unique_ptr<Provider> provider_;
  std::shared_ptr<const had::SharedHessianTopology> shared_topology_;
  ReusableHdotTimings last_timings_;
  std::size_t rebuild_count_ = 0;
  Eigen::VectorXd topology_validated_theta_;

  void validate_active_directions() {
    if (theta_dim_ <= 0 || random_dim_ <= 0)
      throw std::invalid_argument("ReusableTotalHdotTape dimensions invalid");
    std::sort(active_directions_.begin(), active_directions_.end());
    active_directions_.erase(
        std::unique(active_directions_.begin(), active_directions_.end()),
        active_directions_.end());
    for (int direction : active_directions_) {
      if (direction < 0 || direction >= theta_dim_)
        throw std::invalid_argument(
            "ReusableTotalHdotTape active direction out of range");
    }
  }

  void
  build_provider(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
                 std::shared_ptr<const had::SharedHessianTopology> topology) {
    discovery_theta_ = theta;
    discovery_uhat_ = uhat;
    topology_validated_theta_ = theta;
    DirectionProvider zero_direction = [this](int) {
      return Eigen::VectorXd::Zero(random_dim_);
    };
    provider_.reset(new Provider(objective_fn_, zero_direction, theta_dim_,
                                 random_dim_, pattern_, active_directions_,
                                 drop_tol_));
    (void)provider_->compute_all_sparse(theta, uhat);
    shared_topology_ = provider_->FreezeHessianTopology(std::move(topology));
  }

  void ensure_topology_current(const Eigen::VectorXd &theta,
                               const Eigen::VectorXd &uhat) {
    if (theta.size() == topology_validated_theta_.size() &&
        (theta.array() == topology_validated_theta_.array()).all())
      return;
    Eigen::VectorXd probe = uhat;
    for (int i = 0; i < probe.size(); ++i)
      probe[i] += 0.137 * static_cast<double>(i + 1) /
                  static_cast<double>(probe.size() + 1);
    if (!provider_->replay_matches(theta, probe)) {
      build_provider(theta, uhat, nullptr);
      ++rebuild_count_;
    }
    topology_validated_theta_ = theta;
  }
};

} // namespace laplace
} // namespace quadra
