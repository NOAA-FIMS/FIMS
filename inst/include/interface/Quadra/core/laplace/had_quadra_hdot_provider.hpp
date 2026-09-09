#pragma once

#include <Eigen/Dense>
#include <stdexcept>
#include <type_traits>
#include <utility>

// Important: Quadra now uses this wrapper, not direct old had.h assumptions.
#include "../had_quadra.hpp"

namespace quadra {
namespace laplace {

// -----------------------------------------------------------------------------
// Finite-difference Hdot provider fallback
// -----------------------------------------------------------------------------
//
// This is useful as:
//   - a correctness-preserving fallback;
//   - a runtime debugging implementation;
//   - a reference implementation for the had_quadra AD provider.
//
// HessianUUFn must be callable as:
//
//   Eigen::MatrixXd hessian_uu_at_fixed_uhat(theta, uhat)
//
// and MUST NOT re-optimize uhat.
//
template <class HessianUUFn> class FiniteDifferenceHdotProvider {
public:
  explicit FiniteDifferenceHdotProvider(HessianUUFn hessian_uu_at_fixed_uhat,
                                        double step = 1.0e-6,
                                        bool relative_step = true)
      : hessian_uu_at_fixed_uhat_(std::move(hessian_uu_at_fixed_uhat)),
        step_(step), relative_step_(relative_step) {
    if (!(step_ > 0.0) || !std::isfinite(step_)) {
      throw std::invalid_argument(
          "FiniteDifferenceHdotProvider step must be positive and finite.");
    }
  }

  Eigen::MatrixXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat,
                             int theta_index) const {
    if (theta_index < 0 || theta_index >= theta.size()) {
      throw std::out_of_range("theta_index out of range.");
    }

    const double h = relative_step_
                         ? step_ * std::max(1.0, std::abs(theta[theta_index]))
                         : step_;

    Eigen::VectorXd plus = theta;
    Eigen::VectorXd minus = theta;
    plus[theta_index] += h;
    minus[theta_index] -= h;

    const Eigen::MatrixXd H_plus = hessian_uu_at_fixed_uhat_(plus, uhat);
    const Eigen::MatrixXd H_minus = hessian_uu_at_fixed_uhat_(minus, uhat);

    if (H_plus.rows() != H_minus.rows() || H_plus.cols() != H_minus.cols()) {
      throw std::invalid_argument(
          "H_plus and H_minus must have matching dimensions.");
    }

    return (H_plus - H_minus) / (2.0 * h);
  }

private:
  HessianUUFn hessian_uu_at_fixed_uhat_;
  double step_;
  bool relative_step_;
};

template <class HessianUUFn>
auto make_finite_difference_hdot_provider(HessianUUFn hessian_uu_at_fixed_uhat,
                                          double step = 1.0e-6,
                                          bool relative_step = true) {
  return FiniteDifferenceHdotProvider<HessianUUFn>(
      std::move(hessian_uu_at_fixed_uhat), step, relative_step);
}

// -----------------------------------------------------------------------------
// had_quadra Hdot provider scaffold
// -----------------------------------------------------------------------------
//
// This is the intended production hook.
//
// The mathematical target is:
//
//   Hdot_j(a,b) = d^3 f(theta, uhat)
//                 / d theta_j d u_a d u_b
//
// at fixed uhat.
//
// In directional form, for theta direction e_j and random-effect directions
// e_a and e_b:
//
//   D^3 f[(e_j, 0), (0, e_a), (0, e_b)]
//
// becomes the (a,b) entry of Hdot_j.
//
// The implementation belongs here once the exact current had_quadra.hpp API
// names are confirmed.
//
// Expected future shape:
//
//   1. Build a combined parameter vector x = [theta, uhat].
//   2. For each theta_j, seed direction dtheta = e_j.
//   3. Use had_quadra third-directional support to recover the mixed
//      theta/u/u third derivative.
//   4. Assemble dense Hdot_j first.
//   5. Later replace dense assembly with sparse pattern-aware contractions.
//
// Keeping this as a class lets the exact Laplace gradient layer remain stable.
template <class JointObjectiveFn> class HadQuadraHdotProvider {
public:
  explicit HadQuadraHdotProvider(JointObjectiveFn joint_objective)
      : joint_objective_(std::move(joint_objective)) {}

  Eigen::MatrixXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat,
                             int theta_index) const {
    return compute_hdot(theta, uhat, theta_index);
  }

private:
  JointObjectiveFn joint_objective_;

  Eigen::MatrixXd compute_hdot(const Eigen::VectorXd &theta,
                               const Eigen::VectorXd &uhat,
                               int theta_index) const {
    (void)theta;
    (void)uhat;
    (void)theta_index;
    (void)joint_objective_;

    throw std::logic_error(
        "HadQuadraHdotProvider::compute_hdot is a scaffold. "
        "Bind this method to core/autodiff/had_quadra.hpp third-directional "
        "derivative support.");
  }
};

template <class JointObjectiveFn>
auto make_had_quadra_hdot_provider(JointObjectiveFn joint_objective) {
  return HadQuadraHdotProvider<JointObjectiveFn>(std::move(joint_objective));
}

} // namespace laplace
} // namespace quadra
