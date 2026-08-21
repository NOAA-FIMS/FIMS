#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

#include "had_quadra_replay_reuse_lazy_implicit_hdot_provider.hpp"
#include "sparse_huu_factorization.hpp"

namespace quadra {
namespace laplace {

template <class CombinedObjectiveFn> class PersistentStreamingHdotProviderPool {
public:
  using DirectionProvider = std::function<Eigen::VectorXd(int)>;
  using Provider =
      HadQuadraReplayReuseLazyImplicitHdotProvider<CombinedObjectiveFn,
                                                   DirectionProvider>;

  PersistentStreamingHdotProviderPool(CombinedObjectiveFn combined_objective,
                                      int theta_dim, int random_dim,
                                      RandomHessianPattern pattern,
                                      std::vector<int> active_directions,
                                      double drop_tol = 0.0,
                                      std::size_t expected_vertex_count = 0)
      : combined_objective_(std::move(combined_objective)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(pattern)),
        active_directions_(std::move(active_directions)), drop_tol_(drop_tol),
        expected_vertex_count_(expected_vertex_count) {}

  void ensure_workers(int requested_workers) {
    const int worker_count =
        std::min(requested_workers,
                 std::max(1, static_cast<int>(active_directions_.size())));
    if (worker_count == static_cast<int>(providers_.size()))
      return;
    for (const auto &provider : providers_)
      retired_tape_build_count_ += provider->tape_build_count();
    providers_.clear();
    for (int worker = 0; worker < worker_count; ++worker) {
      std::vector<int> directions;
      for (size_t k = static_cast<size_t>(worker);
           k < active_directions_.size();
           k += static_cast<size_t>(worker_count))
        directions.push_back(active_directions_[k]);
      providers_.push_back(std::make_unique<Provider>(
          combined_objective_, DirectionProvider{}, theta_dim_, random_dim_,
          pattern_, std::move(directions), drop_tol_, expected_vertex_count_));
    }
  }

  Provider &provider(int worker) {
    return *providers_.at(static_cast<size_t>(worker));
  }

  int worker_count() const { return static_cast<int>(providers_.size()); }

  int tape_build_count() const {
    int count = retired_tape_build_count_;
    for (const auto &provider : providers_)
      count += provider->tape_build_count();
    return count;
  }

  std::shared_ptr<const had::SharedHessianTopology> FreezeHessianTopology(
      std::shared_ptr<const had::SharedHessianTopology> candidate = nullptr) {
    for (auto &provider : providers_)
      candidate = provider->FreezeHessianTopology(candidate);
    return candidate;
  }

private:
  CombinedObjectiveFn combined_objective_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  double drop_tol_;
  std::size_t expected_vertex_count_;
  int retired_tape_build_count_ = 0;
  std::vector<std::unique_ptr<Provider>> providers_;
};

struct SparseExactLaplaceIterationWorkspaceOptions {
  double hdot_drop_tol = 0.0;
  // Number of independent total-Hdot direction workers.
  //
  // 1 preserves the minimum-memory streaming path. Values greater than 1
  // create one AD graph per worker. Workers share the iteration's sparse
  // H_uu factorization behind a short solve/trace lock. The
  // effective count is min(hdot_workers, active_directions.size()).
  // Each worker contracts and discards one sparse Hdot at a time, so Hdot
  // storage itself remains streaming.
  int hdot_workers = 1;
};

// Sparse per-iteration exact Laplace workspace.
//
// This is the sparse-H_uu analog of ExactLaplaceIterationWorkspace.
// It owns:
//   - sparse H_uu
//   - sparse SimplicialLDLT factorization
//   - lazy implicit direction solves
//   - cached Hdot matrices
//   - cached logdet gradient contribution
template <class CombinedObjectiveFn, class CrossDerivativeFn>
class SparseExactLaplaceIterationWorkspace {
public:
  SparseExactLaplaceIterationWorkspace(
      CombinedObjectiveFn combined_objective,
      CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
      RandomHessianPattern random_hessian_pattern,
      std::vector<int> active_directions, const Eigen::VectorXd &theta,
      const Eigen::VectorXd &uhat, const Eigen::SparseMatrix<double> &Huu,
      SparseExactLaplaceIterationWorkspaceOptions options =
          SparseExactLaplaceIterationWorkspaceOptions{},
      PersistentStreamingHdotProviderPool<CombinedObjectiveFn>
          *persistent_hdot_pool = nullptr)
      : combined_objective_(std::move(combined_objective)),
        cross_derivative_fn_(std::move(cross_derivative_fn)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(random_hessian_pattern)),
        active_directions_(std::move(active_directions)), theta_(theta),
        uhat_(uhat), Huu_(Huu), factor_(Huu), options_(options),
        persistent_hdot_pool_(persistent_hdot_pool) {
    validate();

    std::sort(active_directions_.begin(), active_directions_.end());
    active_directions_.erase(
        std::unique(active_directions_.begin(), active_directions_.end()),
        active_directions_.end());
  }

  Eigen::VectorXd u_direction(int theta_index) const {
    if (theta_index < 0 || theta_index >= theta_dim_) {
      throw std::out_of_range("theta_index out of range.");
    }

    if (cached_u_directions_.rows() == random_dim_ &&
        cached_u_directions_.cols() == theta_dim_)
      return cached_u_directions_.col(theta_index);

    Eigen::VectorXd rhs = cross_derivative_fn_(theta_index);
    if (rhs.size() != random_dim_) {
      throw std::runtime_error(
          "cross derivative callback returned wrong length.");
    }

    return -factor_.solve(rhs);
  }

  std::vector<Eigen::SparseMatrix<double>> compute_total_hdot_all() const {
    auto direction_provider = [this](int theta_index) -> Eigen::VectorXd {
      return this->u_direction(theta_index);
    };

    auto hdot_provider =
        make_had_quadra_replay_reuse_lazy_implicit_hdot_provider(
            combined_objective_, direction_provider, theta_dim_, random_dim_,
            pattern_, active_directions_, options_.hdot_drop_tol);

    return hdot_provider.compute_all_sparse(theta_, uhat_);
  }

  void prepare_trace_terms() const {
    if (trace_terms_prepared_) {
      return;
    }

    cached_logdet_gradient_ = Eigen::VectorXd::Zero(theta_dim_);
    selected_inverse_ =
        std::make_unique<TakahashiSelectedInverse>(factor_.selected_inverse());
    auto direction_provider = [this](int theta_index) -> Eigen::VectorXd {
      return this->u_direction(theta_index);
    };
    if (persistent_hdot_pool_) {
      persistent_hdot_pool_->ensure_workers(1);
      auto &provider = persistent_hdot_pool_->provider(0);
      try {
        const auto contraction = provider.compute_trace_contraction(
            theta_, uhat_, [this](int row, int col) {
              return selected_inverse_->value(row, col);
            });
        if (contraction.input_gradient.size() != theta_dim_ + random_dim_)
          throw std::runtime_error(
              "reverse Hessian trace contraction returned wrong length.");
        const Eigen::VectorXd random_trace =
            contraction.input_gradient.tail(random_dim_);
        // For U = -H_uu^{-1} H_{u theta}, compute U' * random_trace
        // without forming or storing every column of U. Since H_uu is
        // symmetric, one solve gives
        //
        //   U' r = -H_{u theta}' H_uu^{-1} r.
        //
        // The dense direction matrix is still prepared below if the bounded
        // direct contraction falls back to scalar directional Hdot.
        const Eigen::VectorXd solved_random_trace = factor_.solve(random_trace);
        for (int direction : active_directions_) {
          const Eigen::VectorXd cross = cross_derivative_fn_(direction);
          if (cross.size() != random_dim_)
            throw std::runtime_error(
                "cross derivative callback returned wrong length.");
          cached_logdet_gradient_[direction] =
              contraction.input_gradient[direction] -
              cross.dot(solved_random_trace);
        }
        trace_terms_prepared_ = true;
        return;
      } catch (const std::length_error &) {
        // Fall through to the bounded-memory scalar directional path.
      } catch (const std::out_of_range &) {
        // The selected-inverse sparsity did not cover a required trace slot.
      }

      prepare_u_directions_from_callback();
      provider.set_u_direction_provider(direction_provider);
      provider.for_each_sparse(
          theta_, uhat_,
          [this](int direction, Eigen::SparseMatrix<double> hdot) {
            if (hdot.nonZeros() > 0)
              cached_logdet_gradient_[direction] = 0.5 * trace_hdot(hdot);
          });
      trace_terms_prepared_ = true;
      return;
    }
    prepare_u_directions_from_callback();
    if (options_.hdot_workers > 1 && active_directions_.size() > 1) {
      const int worker_count = std::min(
          options_.hdot_workers, static_cast<int>(active_directions_.size()));
      std::vector<std::future<std::vector<std::pair<int, double>>>> futures;
      for (int worker = 0; worker < worker_count; ++worker) {
        std::vector<int> directions;
        for (size_t k = static_cast<size_t>(worker);
             k < active_directions_.size();
             k += static_cast<size_t>(worker_count))
          directions.push_back(active_directions_[k]);
        futures.push_back(std::async(std::launch::async, [this, directions]() {
          auto local_direction = [this](int j) { return this->u_direction(j); };
          auto provider =
              make_had_quadra_replay_reuse_lazy_implicit_hdot_provider(
                  combined_objective_, local_direction, theta_dim_, random_dim_,
                  pattern_, directions, options_.hdot_drop_tol);
          std::vector<std::pair<int, double>> values;
          provider.for_each_sparse(
              theta_, uhat_,
              [this, &values](int direction, Eigen::SparseMatrix<double> hdot) {
                const double trace = trace_hdot(hdot);
                values.emplace_back(direction, 0.5 * trace);
              });
          return values;
        }));
      }
      for (auto &future : futures)
        for (const auto &entry : future.get())
          cached_logdet_gradient_[entry.first] = entry.second;
      trace_terms_prepared_ = true;
      return;
    }
    auto hdot_provider =
        make_had_quadra_replay_reuse_lazy_implicit_hdot_provider(
            combined_objective_, direction_provider, theta_dim_, random_dim_,
            pattern_, active_directions_, options_.hdot_drop_tol);
    hdot_provider.for_each_sparse(
        theta_, uhat_, [this](int direction, Eigen::SparseMatrix<double> hdot) {
          if (hdot.nonZeros() > 0)
            cached_logdet_gradient_[direction] = 0.5 * trace_hdot(hdot);
        });

    trace_terms_prepared_ = true;
  }

  Eigen::VectorXd
  gradient(const Eigen::VectorXd &joint_envelope_gradient) const {
    if (joint_envelope_gradient.size() != theta_dim_) {
      throw std::invalid_argument("joint_envelope_gradient has wrong length.");
    }

    prepare_trace_terms();
    return joint_envelope_gradient + cached_logdet_gradient_;
  }

  const Eigen::VectorXd &logdet_gradient_contribution() const {
    prepare_trace_terms();
    return cached_logdet_gradient_;
  }

  double logdet_huu() const { return factor_.logdet(); }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

  const Eigen::SparseMatrix<double> &Huu() const { return Huu_; }

private:
  CombinedObjectiveFn combined_objective_;
  CrossDerivativeFn cross_derivative_fn_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  Eigen::VectorXd theta_;
  Eigen::VectorXd uhat_;
  Eigen::SparseMatrix<double> Huu_;
  SparseHuuFactorization factor_;
  mutable std::mutex factor_mutex_;
  SparseExactLaplaceIterationWorkspaceOptions options_;
  PersistentStreamingHdotProviderPool<CombinedObjectiveFn>
      *persistent_hdot_pool_;

  mutable bool trace_terms_prepared_ = false;
  mutable Eigen::VectorXd cached_logdet_gradient_;
  mutable Eigen::MatrixXd cached_u_directions_;
  mutable std::unique_ptr<TakahashiSelectedInverse> selected_inverse_;

  void prepare_u_directions_from_callback() const {
    Eigen::MatrixXd cross = Eigen::MatrixXd::Zero(random_dim_, theta_dim_);
    for (int direction : active_directions_) {
      Eigen::VectorXd column = cross_derivative_fn_(direction);
      if (column.size() != random_dim_)
        throw std::runtime_error(
            "cross derivative callback returned wrong length.");
      cross.col(direction) = column;
    }
    cached_u_directions_ = -factor_.solve(cross);
  }

  double trace_hdot(const Eigen::SparseMatrix<double> &hdot) const {
    if (hdot.nonZeros() == 0)
      return 0.0;
    if (selected_inverse_ && selected_inverse_->supports(hdot))
      return selected_inverse_->trace_inverse_times(hdot);
    std::lock_guard<std::mutex> lock(factor_mutex_);
    return factor_.trace_inverse_times_streaming(hdot);
  }

  void validate() const {
    if (theta_dim_ <= 0 || random_dim_ <= 0) {
      throw std::invalid_argument("dimensions must be positive.");
    }
    if (options_.hdot_workers < 1)
      throw std::invalid_argument("hdot_workers must be positive.");
    if (theta_.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat_.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }
    if (Huu_.rows() != random_dim_ || Huu_.cols() != random_dim_) {
      throw std::invalid_argument("Huu has wrong dimensions.");
    }
    for (const auto &entry : pattern_) {
      if (entry.first < 0 || entry.first >= random_dim_ || entry.second < 0 ||
          entry.second >= random_dim_) {
        throw std::out_of_range("random Hessian pattern entry out of range.");
      }
    }
  }
};

template <class CombinedObjectiveFn, class CrossDerivativeFn>
auto make_sparse_exact_laplace_iteration_workspace(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat, const Eigen::SparseMatrix<double> &Huu,
    SparseExactLaplaceIterationWorkspaceOptions options =
        SparseExactLaplaceIterationWorkspaceOptions{},
    PersistentStreamingHdotProviderPool<CombinedObjectiveFn>
        *persistent_hdot_pool = nullptr) {
  return SparseExactLaplaceIterationWorkspace<CombinedObjectiveFn,
                                              CrossDerivativeFn>(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern),
      std::move(active_directions), theta, uhat, Huu, options,
      persistent_hdot_pool);
}

} // namespace laplace
} // namespace quadra
