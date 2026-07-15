#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace quadra {
namespace laplace {

// Wraps an Hdot provider and skips directions whose Hdot is known
// structurally zero.
//
// BaseProvider must support:
//
//   Eigen::SparseMatrix<double> sparse(theta, uhat, theta_index)
//
// and may also support dense operator(), but this wrapper provides both.
//
// active_direction_indices are fixed-effect indices j for which
//
//   dH_uu / dtheta_j
//
// may be nonzero.
//
// If j is inactive, this wrapper returns a zero Hdot without invoking the base
// provider. This avoids AD graph replay and directional propagation for
// structurally zero directions.
template <class BaseProvider> class ActiveDirectionHdotProvider {
public:
  ActiveDirectionHdotProvider(BaseProvider base_provider, int random_dim,
                              std::vector<int> active_direction_indices)
      : base_provider_(std::move(base_provider)), random_dim_(random_dim),
        active_(std::move(active_direction_indices)) {
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }

    std::sort(active_.begin(), active_.end());
    active_.erase(std::unique(active_.begin(), active_.end()), active_.end());

    for (int idx : active_) {
      if (idx < 0) {
        throw std::out_of_range("active direction index must be nonnegative.");
      }
    }
  }

  bool is_active(int theta_index) const {
    return std::binary_search(active_.begin(), active_.end(), theta_index);
  }

  Eigen::SparseMatrix<double> sparse(const Eigen::VectorXd &theta,
                                     const Eigen::VectorXd &uhat,
                                     int theta_index) const {
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }
    if (theta_index < 0 || theta_index >= theta.size()) {
      throw std::out_of_range("theta_index out of range.");
    }

    if (!is_active(theta_index)) {
      Eigen::SparseMatrix<double> zero(random_dim_, random_dim_);
      zero.makeCompressed();
      return zero;
    }

    return base_provider_.sparse(theta, uhat, theta_index);
  }

  Eigen::MatrixXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat,
                             int theta_index) const {
    return Eigen::MatrixXd(sparse(theta, uhat, theta_index));
  }

  const std::vector<int> &active_directions() const { return active_; }

private:
  BaseProvider base_provider_;
  int random_dim_;
  std::vector<int> active_;
};

template <class BaseProvider>
auto make_active_direction_hdot_provider(
    BaseProvider base_provider, int random_dim,
    std::vector<int> active_direction_indices) {
  return ActiveDirectionHdotProvider<BaseProvider>(
      std::move(base_provider), random_dim,
      std::move(active_direction_indices));
}

} // namespace laplace
} // namespace quadra
