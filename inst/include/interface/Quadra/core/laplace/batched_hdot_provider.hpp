#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <chrono>
#include <stdexcept>
#include <utility>
#include <vector>

#include "had_quadra_replay_reuse_lazy_implicit_hdot_provider.hpp"

namespace quadra {
namespace laplace {

struct BatchedHdotProviderResult {
  std::vector<int> directions;
  std::vector<Eigen::SparseMatrix<double>> Hdots;
  std::vector<double> direction_ms;
  std::vector<int> direction_nnz;

  int total_nonzeros() const {
    int total = 0;
    for (int n : direction_nnz) {
      total += n;
    }
    return total;
  }

  double total_direction_ms() const {
    double total = 0.0;
    for (double x : direction_ms) {
      total += x;
    }
    return total;
  }
};

// Batched directional Hdot provider scaffold.
//
// This is the public seam for future multi-tangent propagation. The current
// implementation delegates each direction to the existing replay-reuse lazy
// implicit provider, but callers now depend on a batched API rather than a
// direction-by-direction loop.
//
// DirectionProvider must be callable as:
//   Eigen::VectorXd operator()(int theta_index) const;
//
// and returns du*/dtheta_j.
template <class CombinedObjectiveFn, class DirectionProvider>
class BatchedHdotProvider {
public:
  BatchedHdotProvider(CombinedObjectiveFn combined_objective,
                      DirectionProvider direction_provider, int theta_dim,
                      int random_dim,
                      RandomHessianPattern random_hessian_pattern,
                      double hdot_drop_tol = 0.0)
      : combined_objective_(std::move(combined_objective)),
        direction_provider_(std::move(direction_provider)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(random_hessian_pattern)),
        hdot_drop_tol_(hdot_drop_tol) {
    if (theta_dim_ <= 0 || random_dim_ <= 0) {
      throw std::invalid_argument(
          "BatchedHdotProvider dimensions must be positive.");
    }
  }

  BatchedHdotProviderResult compute(const Eigen::VectorXd &theta,
                                    const Eigen::VectorXd &uhat,
                                    const std::vector<int> &directions) const {
    if (theta.size() != theta_dim_) {
      throw std::invalid_argument(
          "BatchedHdotProvider::compute: theta has wrong length.");
    }
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument(
          "BatchedHdotProvider::compute: uhat has wrong length.");
    }

    BatchedHdotProviderResult out;
    out.directions = directions;
    out.Hdots.reserve(directions.size());
    out.direction_ms.reserve(directions.size());
    out.direction_nnz.reserve(directions.size());

    for (int direction : directions) {
      if (direction < 0 || direction >= theta_dim_) {
        throw std::out_of_range("BatchedHdotProvider direction out of range.");
      }

      const auto start = std::chrono::steady_clock::now();

      auto single_provider =
          make_had_quadra_replay_reuse_lazy_implicit_hdot_provider(
              combined_objective_, direction_provider_, theta_dim_, random_dim_,
              pattern_, std::vector<int>{direction}, hdot_drop_tol_);

      auto all = single_provider.compute_all_sparse(theta, uhat);

      if (static_cast<int>(all.size()) != theta_dim_) {
        throw std::runtime_error(
            "single Hdot provider returned wrong vector length.");
      }

      Eigen::SparseMatrix<double> Hdot = all[static_cast<size_t>(direction)];

      const auto end = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;

      out.direction_ms.push_back(elapsed.count());
      out.direction_nnz.push_back(static_cast<int>(Hdot.nonZeros()));
      out.Hdots.push_back(std::move(Hdot));
    }

    return out;
  }

  // Return theta_dim-sized Hdot vector, with zeros for inactive directions.
  std::vector<Eigen::SparseMatrix<double>>
  compute_all_sparse(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
                     const std::vector<int> &directions) const {
    auto batch = compute(theta, uhat, directions);

    std::vector<Eigen::SparseMatrix<double>> out;
    out.reserve(static_cast<size_t>(theta_dim_));

    for (int j = 0; j < theta_dim_; ++j) {
      out.emplace_back(random_dim_, random_dim_);
    }

    for (size_t k = 0; k < batch.directions.size(); ++k) {
      out[static_cast<size_t>(batch.directions[k])] = std::move(batch.Hdots[k]);
    }

    return out;
  }

private:
  CombinedObjectiveFn combined_objective_;
  DirectionProvider direction_provider_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  double hdot_drop_tol_;
};

template <class CombinedObjectiveFn, class DirectionProvider>
auto make_batched_hdot_provider(CombinedObjectiveFn combined_objective,
                                DirectionProvider direction_provider,
                                int theta_dim, int random_dim,
                                RandomHessianPattern random_hessian_pattern,
                                double hdot_drop_tol = 0.0) {
  return BatchedHdotProvider<CombinedObjectiveFn, DirectionProvider>(
      std::move(combined_objective), std::move(direction_provider), theta_dim,
      random_dim, std::move(random_hessian_pattern), hdot_drop_tol);
}

} // namespace laplace
} // namespace quadra
