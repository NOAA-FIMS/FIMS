#ifndef QUADRA_LAPLACE_PROFILED_DERIVED_REPORT_HPP
#define QUADRA_LAPLACE_PROFILED_DERIVED_REPORT_HPP

#include "laplace_implicit_workspace.hpp"
#include "laplace_profiled_ad_gradient.hpp"
#include "laplace_profiled_delta_method_vector.hpp"

#include <Eigen/Dense>

#include <functional>
#include <string>
#include <vector>

namespace quadra {

struct ProfiledDerivedQuantity {
  std::string name_m;

  std::function<AD(const std::vector<AD> &, const std::vector<AD> &)> fun_m;
};

struct LaplaceProfiledDerivedReport {
  std::vector<std::string> names_m;
  LaplaceProfiledDeltaMethodVectorResult delta_m;

  bool success_m = false;
  std::string message_m;
};

inline LaplaceProfiledDerivedReport compute_laplace_profiled_derived_report(
    const std::vector<ProfiledDerivedQuantity> &quantities,
    const std::vector<double> &theta_hat, const std::vector<double> &u_hat,
    const Eigen::MatrixXd &theta_covariance,
    const LaplaceImplicitWorkspace &workspace) {
  LaplaceProfiledDerivedReport report;

  if (!workspace.success_m) {
    report.message_m =
        "Cannot compute profiled derived report: workspace failed: " +
        workspace.message_m;
    return report;
  }

  if (quantities.empty()) {
    report.message_m =
        "Cannot compute profiled derived report: no quantities supplied.";
    return report;
  }

  const Eigen::Index n_derived = static_cast<Eigen::Index>(quantities.size());

  Eigen::VectorXd estimates = Eigen::VectorXd::Zero(n_derived);

  Eigen::MatrixXd G_theta = Eigen::MatrixXd::Zero(
      n_derived, static_cast<Eigen::Index>(theta_hat.size()));

  Eigen::MatrixXd G_u =
      Eigen::MatrixXd::Zero(n_derived, static_cast<Eigen::Index>(u_hat.size()));

  report.names_m.reserve(quantities.size());

  for (std::size_t i = 0; i < quantities.size(); ++i) {
    report.names_m.push_back(quantities[i].name_m);

    const auto blocks = evaluate_profiled_ad_gradient_blocks(
        quantities[i].fun_m, theta_hat, u_hat);

    if (!blocks.success_m) {
      report.message_m = "Failed profiled AD gradient for " +
                         quantities[i].name_m + ": " + blocks.message_m;
      return report;
    }

    estimates[static_cast<Eigen::Index>(i)] = blocks.estimate_m;

    G_theta.row(static_cast<Eigen::Index>(i)) =
        blocks.gradient_fixed_m.transpose();

    G_u.row(static_cast<Eigen::Index>(i)) =
        blocks.gradient_random_m.transpose();
  }

  report.delta_m = compute_laplace_profiled_delta_method_vector(
      estimates, G_theta, G_u, workspace.du_dtheta_m, theta_covariance);

  if (!report.delta_m.success_m) {
    report.message_m =
        "Profiled vector delta method failed: " + report.delta_m.message_m;
    return report;
  }

  report.success_m = true;
  report.message_m = "Laplace profiled derived report computed successfully.";

  return report;
}

} // namespace quadra

#endif
