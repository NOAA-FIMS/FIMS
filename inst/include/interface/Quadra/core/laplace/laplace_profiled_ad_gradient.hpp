#ifndef QUADRA_LAPLACE_PROFILED_AD_GRADIENT_HPP
#define QUADRA_LAPLACE_PROFILED_AD_GRADIENT_HPP

#include "../autodiff.hpp"

#include <Eigen/Dense>

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace quadra {

struct ProfiledADGradientResult {
  double estimate_m = std::numeric_limits<double>::quiet_NaN();

  Eigen::VectorXd gradient_fixed_m;
  Eigen::VectorXd gradient_random_m;

  bool success_m = false;
  std::string message_m;
};

template <typename Functor>
ProfiledADGradientResult
evaluate_profiled_ad_gradient_blocks(Functor &&f,
                                     const std::vector<double> &fixed,
                                     const std::vector<double> &random) {
  ProfiledADGradientResult result;

  const Eigen::Index n_fixed = static_cast<Eigen::Index>(fixed.size());

  const Eigen::Index n_random = static_cast<Eigen::Index>(random.size());

  result.gradient_fixed_m = Eigen::VectorXd::Zero(n_fixed);

  result.gradient_random_m = Eigen::VectorXd::Zero(n_random);

  if (n_fixed == 0) {
    result.message_m = "No fixed effects supplied.";
    return result;
  }

  try {
    had::ADGraph graph;
    had::g_ADGraph = &graph;

    std::vector<quadra::AD> fixed_ad;
    fixed_ad.reserve(fixed.size());

    for (double x : fixed) {
      fixed_ad.emplace_back(x);
    }

    std::vector<quadra::AD> random_ad;
    random_ad.reserve(random.size());

    for (double x : random) {
      random_ad.emplace_back(x);
    }

    const quadra::AD y = f(fixed_ad, random_ad);

    result.estimate_m = value_of(y);

    if (!std::isfinite(result.estimate_m)) {
      result.message_m = "Derived quantity estimate is non-finite.";
      return result;
    }

    had::ZeroAdjoints(graph);
    had::SetAdjoint(y, 1.0);
    had::PropagateAdjoint();

    for (Eigen::Index i = 0; i < n_fixed; ++i) {
      result.gradient_fixed_m[i] =
          had::GetAdjoint(fixed_ad[static_cast<std::size_t>(i)]);
    }

    for (Eigen::Index i = 0; i < n_random; ++i) {
      result.gradient_random_m[i] =
          had::GetAdjoint(random_ad[static_cast<std::size_t>(i)]);
    }

    if (!result.gradient_fixed_m.allFinite() ||
        !result.gradient_random_m.allFinite()) {
      result.message_m = "Derived gradient contains non-finite values.";
      return result;
    }

    result.success_m = true;
    result.message_m = "Profiled AD gradient blocks evaluated successfully.";

    return result;
  } catch (const std::exception &e) {
    result.message_m = std::string("Profiled AD gradient failed: ") + e.what();
    return result;
  } catch (...) {
    result.message_m = "Profiled AD gradient failed with unknown exception.";
    return result;
  }
}

} // namespace quadra

#endif
