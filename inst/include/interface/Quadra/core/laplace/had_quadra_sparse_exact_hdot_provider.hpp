#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../had_quadra.hpp"

namespace quadra {
namespace laplace {

using RandomHessianPattern = std::vector<std::pair<int, int>>;

inline RandomHessianPattern dense_random_hessian_pattern(int random_dim) {
  if (random_dim <= 0) {
    throw std::invalid_argument("random_dim must be positive.");
  }

  RandomHessianPattern pattern;
  pattern.reserve(static_cast<size_t>(random_dim * (random_dim + 1) / 2));

  for (int i = 0; i < random_dim; ++i) {
    for (int j = 0; j <= i; ++j) {
      pattern.emplace_back(i, j);
    }
  }

  return pattern;
}

inline RandomHessianPattern diagonal_random_hessian_pattern(int random_dim) {
  if (random_dim <= 0) {
    throw std::invalid_argument("random_dim must be positive.");
  }

  RandomHessianPattern pattern;
  pattern.reserve(static_cast<size_t>(random_dim));

  for (int i = 0; i < random_dim; ++i) {
    pattern.emplace_back(i, i);
  }

  return pattern;
}

template <class CombinedObjectiveFn> class HadQuadraSparseExactHdotProvider {
public:
  HadQuadraSparseExactHdotProvider(CombinedObjectiveFn combined_objective,
                                   int theta_dim, int random_dim,
                                   RandomHessianPattern pattern,
                                   double drop_tol = 0.0)
      : combined_objective_(std::move(combined_objective)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(pattern)), drop_tol_(drop_tol) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (drop_tol_ < 0.0 || !std::isfinite(drop_tol_)) {
      throw std::invalid_argument("drop_tol must be nonnegative and finite.");
    }

    for (const auto &entry : pattern_) {
      if (entry.first < 0 || entry.first >= random_dim_ || entry.second < 0 ||
          entry.second >= random_dim_) {
        throw std::out_of_range("random Hessian pattern entry out of range.");
      }
    }
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

    had::ADGraph graph;
    had::g_ADGraph = &graph;

    std::vector<had::AReal> x;
    x.reserve(static_cast<size_t>(theta_dim_ + random_dim_));

    for (int j = 0; j < theta_dim_; ++j) {
      x.emplace_back(theta[j]);
      const double d = (j == theta_index) ? 1.0 : 0.0;
      x.back().dot = d;
      graph.vertices[x.back().varId].dot = d;
    }

    for (int i = 0; i < random_dim_; ++i) {
      x.emplace_back(uhat[i]);
      x.back().dot = 0.0;
      graph.vertices[x.back().varId].dot = 0.0;
    }

    had::AReal y = combined_objective_(x);

    had::SetAdjoint(y, had::Real(1.0));
    had::PropagateAdjointDirectional();

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(pattern_.size() * 2);

    for (const auto &entry : pattern_) {
      const int a = entry.first;
      const int b = entry.second;

      const int ia = theta_dim_ + a;
      const int ib = theta_dim_ + b;

      double value = had::GetAdjointDot(x[static_cast<size_t>(ia)],
                                        x[static_cast<size_t>(ib)]);

      if (std::abs(value) <= drop_tol_) {
        continue;
      }

      triplets.emplace_back(a, b, value);
      if (a != b) {
        triplets.emplace_back(b, a, value);
      }
    }

    had::g_ADGraph = nullptr;

    Eigen::SparseMatrix<double> Hdot(random_dim_, random_dim_);
    Hdot.setFromTriplets(triplets.begin(), triplets.end());
    Hdot.makeCompressed();
    return Hdot;
  }

  Eigen::MatrixXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat,
                             int theta_index) const {
    return Eigen::MatrixXd(sparse(theta, uhat, theta_index));
  }

  const RandomHessianPattern &pattern() const { return pattern_; }

private:
  CombinedObjectiveFn combined_objective_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  double drop_tol_;
};

template <class CombinedObjectiveFn>
auto make_had_quadra_sparse_exact_hdot_provider(
    CombinedObjectiveFn combined_objective, int theta_dim, int random_dim,
    RandomHessianPattern pattern, double drop_tol = 0.0) {
  return HadQuadraSparseExactHdotProvider<CombinedObjectiveFn>(
      std::move(combined_objective), theta_dim, random_dim, std::move(pattern),
      drop_tol);
}

} // namespace laplace
} // namespace quadra
