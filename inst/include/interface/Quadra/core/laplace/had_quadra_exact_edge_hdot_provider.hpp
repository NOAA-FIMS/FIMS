#pragma once

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../had_quadra.hpp"

namespace quadra {
namespace laplace {

// Exact directional edge-pushed dense Hdot provider.
//
// CombinedObjectiveFn must be callable as:
//
//   template <class AD>
//   AD operator()(const std::vector<AD>& x) const;
//
// where x = [theta, u].
//
// For a fixed-effect index j, this provider seeds the primal tangent
//
//   d = [e_j, 0_u]
//
// then extracts
//
//   Hdot_j(a,b) = D H_uu(a,b)[d]
//
// from had::GetAdjointDot(u_a, u_b) after
// had::PropagateAdjointDirectional().
template <class CombinedObjectiveFn> class HadQuadraExactEdgeHdotProvider {
public:
  HadQuadraExactEdgeHdotProvider(CombinedObjectiveFn combined_objective,
                                 int theta_dim, int random_dim,
                                 double drop_tol = 0.0)
      : combined_objective_(std::move(combined_objective)),
        theta_dim_(theta_dim), random_dim_(random_dim), drop_tol_(drop_tol) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (drop_tol_ < 0.0 || !std::isfinite(drop_tol_)) {
      throw std::invalid_argument("drop_tol must be nonnegative and finite.");
    }
  }

  Eigen::MatrixXd operator()(const Eigen::VectorXd &theta,
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

    Eigen::MatrixXd Hdot(random_dim_, random_dim_);
    Hdot.setZero();

    for (int a = 0; a < random_dim_; ++a) {
      const int ia = theta_dim_ + a;
      for (int b = 0; b < random_dim_; ++b) {
        const int ib = theta_dim_ + b;
        double value = had::GetAdjointDot(x[static_cast<size_t>(ia)],
                                          x[static_cast<size_t>(ib)]);
        if (std::abs(value) <= drop_tol_) {
          value = 0.0;
        }
        Hdot(a, b) = value;
      }
    }

    // Avoid leaving a dangling global graph pointer.
    had::g_ADGraph = nullptr;

    return 0.5 * (Hdot + Hdot.transpose());
  }

private:
  CombinedObjectiveFn combined_objective_;
  int theta_dim_;
  int random_dim_;
  double drop_tol_;
};

template <class CombinedObjectiveFn>
auto make_had_quadra_exact_edge_hdot_provider(
    CombinedObjectiveFn combined_objective, int theta_dim, int random_dim,
    double drop_tol = 0.0) {
  return HadQuadraExactEdgeHdotProvider<CombinedObjectiveFn>(
      std::move(combined_objective), theta_dim, random_dim, drop_tol);
}

} // namespace laplace
} // namespace quadra
