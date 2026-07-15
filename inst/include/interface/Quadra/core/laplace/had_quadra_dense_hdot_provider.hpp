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

// had_quadra-backed dense Hdot provider.
//
// CombinedObjectiveFn must be callable as:
//
//   template <class AD>
//   AD operator()(const std::vector<AD>& x) const;
//
// where x = [theta, u].
//
// This provider computes the directional derivative of the full Hessian
// with respect to direction [e_theta_j, 0_u], then extracts the u/u block.
//
// Mathematically:
//
//   full directional Hessian = D_x [ Hessian_xx f(x) ] [d]
//
// and the random/random block is:
//
//   Hdot_j(a,b) = d/dtheta_j H_uu(a,b)
//
// evaluated at fixed uhat.
template <class CombinedObjectiveFn> class HadQuadraDenseHdotProvider {
public:
  HadQuadraDenseHdotProvider(CombinedObjectiveFn combined_objective,
                             int theta_dim, int random_dim, double eps = 1.0e-6)
      : combined_objective_(std::move(combined_objective)),
        theta_dim_(theta_dim), random_dim_(random_dim), eps_(eps) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (!(eps_ > 0.0) || !std::isfinite(eps_)) {
      throw std::invalid_argument("eps must be positive and finite.");
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

    std::vector<had::Real> x(static_cast<size_t>(theta_dim_ + random_dim_));
    std::vector<had::Real> direction(x.size(), had::Real(0.0));

    for (int j = 0; j < theta_dim_; ++j) {
      x[static_cast<size_t>(j)] = theta[j];
    }
    for (int i = 0; i < random_dim_; ++i) {
      x[static_cast<size_t>(theta_dim_ + i)] = uhat[i];
    }

    direction[static_cast<size_t>(theta_index)] = had::Real(1.0);

    const had::DenseMatrix dH =
        had::hessian_directional_derivative_central_difference(
            combined_objective_, x, direction, eps_);

    if (static_cast<int>(dH.size()) != theta_dim_ + random_dim_) {
      throw std::runtime_error(
          "had_quadra directional Hessian returned wrong row count.");
    }

    Eigen::MatrixXd Hdot(random_dim_, random_dim_);
    Hdot.setZero();

    for (int a = 0; a < random_dim_; ++a) {
      const int row = theta_dim_ + a;
      if (static_cast<int>(dH[static_cast<size_t>(row)].size()) !=
          theta_dim_ + random_dim_) {
        throw std::runtime_error(
            "had_quadra directional Hessian returned wrong column count.");
      }

      for (int b = 0; b < random_dim_; ++b) {
        const int col = theta_dim_ + b;
        Hdot(a, b) = dH[static_cast<size_t>(row)][static_cast<size_t>(col)];
      }
    }

    // Numerical central differences can produce tiny asymmetry.
    return 0.5 * (Hdot + Hdot.transpose());
  }

private:
  CombinedObjectiveFn combined_objective_;
  int theta_dim_;
  int random_dim_;
  double eps_;
};

template <class CombinedObjectiveFn>
auto make_had_quadra_dense_hdot_provider(CombinedObjectiveFn combined_objective,
                                         int theta_dim, int random_dim,
                                         double eps = 1.0e-6) {
  return HadQuadraDenseHdotProvider<CombinedObjectiveFn>(
      std::move(combined_objective), theta_dim, random_dim, eps);
}

} // namespace laplace
} // namespace quadra
