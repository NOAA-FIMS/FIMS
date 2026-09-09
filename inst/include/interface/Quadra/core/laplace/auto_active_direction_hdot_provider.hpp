#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace quadra {
namespace laplace {

struct ActiveDirectionDiscoveryResult {
  std::vector<int> active_directions;
  std::vector<double> hdot_norms;
  std::vector<int> hdot_nonzeros;
};

// Auto-active wrapper for sparse Hdot providers.
//
// BaseProvider must support:
//
//   Eigen::SparseMatrix<double> sparse(theta, uhat, theta_index)
//
// Discovery is explicit and deterministic. Construct the wrapper with a
// representative theta/uhat point. It probes all fixed-effect directions once,
// caches active directions, and skips inactive directions afterward.
//
// This is safe when the structural dependency of H_uu on theta is stable across
// the optimization region. That is true for many mixed-effects model components
// such as variance/precision parameters versus location-only parameters.
//
// For highly state-dependent sparsity, rediscover after major model-structure
// changes or use a conservative tolerance.
template <class BaseProvider> class AutoActiveDirectionHdotProvider {
public:
  AutoActiveDirectionHdotProvider(BaseProvider base_provider,
                                  const Eigen::VectorXd &discovery_theta,
                                  const Eigen::VectorXd &discovery_uhat,
                                  int theta_dim, int random_dim,
                                  double discovery_tol = 0.0)
      : base_provider_(std::move(base_provider)), theta_dim_(theta_dim),
        random_dim_(random_dim), discovery_tol_(discovery_tol) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (discovery_theta.size() != theta_dim_) {
      throw std::invalid_argument("discovery_theta has wrong length.");
    }
    if (discovery_uhat.size() != random_dim_) {
      throw std::invalid_argument("discovery_uhat has wrong length.");
    }
    if (discovery_tol_ < 0.0 || !std::isfinite(discovery_tol_)) {
      throw std::invalid_argument(
          "discovery_tol must be nonnegative and finite.");
    }

    discover(discovery_theta, discovery_uhat);
  }

  bool is_active(int theta_index) const {
    return std::binary_search(discovery_.active_directions.begin(),
                              discovery_.active_directions.end(), theta_index);
  }

  Eigen::SparseMatrix<double> sparse(const Eigen::VectorXd &theta,
                                     const Eigen::VectorXd &uhat,
                                     int theta_index) const {
    if (theta.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }
    if (theta_index < 0 || theta_index >= theta_dim_) {
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

  const ActiveDirectionDiscoveryResult &discovery() const { return discovery_; }

  const std::vector<int> &active_directions() const {
    return discovery_.active_directions;
  }

private:
  BaseProvider base_provider_;
  int theta_dim_;
  int random_dim_;
  double discovery_tol_;
  ActiveDirectionDiscoveryResult discovery_;

  void discover(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat) {
    discovery_.active_directions.clear();
    discovery_.hdot_norms.assign(static_cast<size_t>(theta_dim_), 0.0);
    discovery_.hdot_nonzeros.assign(static_cast<size_t>(theta_dim_), 0);

    for (int j = 0; j < theta_dim_; ++j) {
      const Eigen::SparseMatrix<double> Hdot =
          base_provider_.sparse(theta, uhat, j);

      const double norm = Hdot.norm();
      const int nnz = Hdot.nonZeros();

      discovery_.hdot_norms[static_cast<size_t>(j)] = norm;
      discovery_.hdot_nonzeros[static_cast<size_t>(j)] = nnz;

      if (nnz > 0 && norm > discovery_tol_) {
        discovery_.active_directions.push_back(j);
      }
    }

    std::sort(discovery_.active_directions.begin(),
              discovery_.active_directions.end());
  }
};

template <class BaseProvider>
auto make_auto_active_direction_hdot_provider(
    BaseProvider base_provider, const Eigen::VectorXd &discovery_theta,
    const Eigen::VectorXd &discovery_uhat, int theta_dim, int random_dim,
    double discovery_tol = 0.0) {
  return AutoActiveDirectionHdotProvider<BaseProvider>(
      std::move(base_provider), discovery_theta, discovery_uhat, theta_dim,
      random_dim, discovery_tol);
}

} // namespace laplace
} // namespace quadra
