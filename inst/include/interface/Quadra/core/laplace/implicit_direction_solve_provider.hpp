#pragma once

#include <Eigen/Dense>
#include <stdexcept>
#include <utility>

namespace quadra {
namespace laplace {

// Lazy implicit direction provider.
//
// Computes
//
//   du*/dtheta_j = - H_uu^{-1} f_{u theta_j}
//
// on demand, using a factorization of H_uu reused across all requested
// directions.
//
// CrossDerivativeFn must be callable as:
//
//   Eigen::VectorXd operator()(int theta_index) const;
//
// and return f_{u theta_j}, the fixed-u mixed derivative of the joint
// objective gradient wrt random effects and theta_j.
//
// This class deliberately accepts f_{u theta_j} as a callback so the
// AD/analytic source of cross derivatives can evolve independently from the
// solve/reuse mechanics.
template <class CrossDerivativeFn> class ImplicitDirectionSolveProvider {
public:
  ImplicitDirectionSolveProvider(const Eigen::MatrixXd &Huu,
                                 CrossDerivativeFn cross_derivative_fn)
      : ldlt_(Huu), n_(static_cast<int>(Huu.rows())),
        cross_derivative_fn_(std::move(cross_derivative_fn)) {
    if (Huu.rows() != Huu.cols()) {
      throw std::invalid_argument("Huu must be square.");
    }
    if (n_ <= 0) {
      throw std::invalid_argument("Huu must be nonempty.");
    }
    if (ldlt_.info() != Eigen::Success) {
      throw std::runtime_error("LDLT factorization failed.");
    }
  }

  Eigen::VectorXd operator()(int theta_index) const {
    Eigen::VectorXd rhs = cross_derivative_fn_(theta_index);

    if (rhs.size() != n_) {
      throw std::runtime_error(
          "cross derivative callback returned wrong length.");
    }

    return -ldlt_.solve(rhs);
  }

  int random_dim() const { return n_; }

private:
  Eigen::LDLT<Eigen::MatrixXd> ldlt_;
  int n_;
  CrossDerivativeFn cross_derivative_fn_;
};

template <class CrossDerivativeFn>
auto make_implicit_direction_solve_provider(
    const Eigen::MatrixXd &Huu, CrossDerivativeFn cross_derivative_fn) {
  return ImplicitDirectionSolveProvider<CrossDerivativeFn>(
      Huu, std::move(cross_derivative_fn));
}

} // namespace laplace
} // namespace quadra
