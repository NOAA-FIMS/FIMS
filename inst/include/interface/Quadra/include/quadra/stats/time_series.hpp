#ifndef QUADRA_PUBLIC_STATS_TIME_SERIES_HPP
#define QUADRA_PUBLIC_STATS_TIME_SERIES_HPP

#include "distributions.hpp"

#include <cmath>
#include <vector>

namespace quadra {
namespace stats {

// Stationary AR(1), parameterized by the innovation standard deviation:
//   x[0] ~ Normal(mean, innovation_sd / sqrt(1 - phi^2))
//   x[t] ~ Normal(mean + phi * (x[t - 1] - mean), innovation_sd)
template <typename T>
T ar1_stationary_logpdf(const std::vector<T> &x, const T &mean, const T &phi,
                        const T &innovation_sd) {
  using std::sqrt;
  if (x.empty()) {
    return T(0.0);
  }

  const T marginal_sd = innovation_sd / sqrt(T(1.0) - phi * phi);
  T log_density = normal_logpdf(x[0], mean, marginal_sd);

  for (std::size_t i = 1; i < x.size(); ++i) {
    const T conditional_mean = mean + phi * (x[i - 1] - mean);
    log_density += normal_logpdf(x[i], conditional_mean, innovation_sd);
  }

  return log_density;
}

template <typename T>
T ar1_stationary_nll(const std::vector<T> &x, const T &mean, const T &phi,
                     const T &innovation_sd) {
  return -ar1_stationary_logpdf(x, mean, phi, innovation_sd);
}

// Stationary AR(1), parameterized by the marginal standard deviation.
// This convention matches RTMB::dautoreg(..., scale = marginal_sd).
template <typename T>
T ar1_marginal_logpdf(const std::vector<T> &x, const T &mean, const T &phi,
                      const T &marginal_sd) {
  using std::sqrt;
  const T innovation_sd = marginal_sd * sqrt(T(1.0) - phi * phi);
  return ar1_stationary_logpdf(x, mean, phi, innovation_sd);
}

template <typename T>
T ar1_marginal_nll(const std::vector<T> &x, const T &mean, const T &phi,
                   const T &marginal_sd) {
  return -ar1_marginal_logpdf(x, mean, phi, marginal_sd);
}

// Transform independent innovations into a finite MA(1) process:
//   x[t] = mean + innovation[t + 1] + theta * innovation[t]
// The innovations vector therefore contains one more element than the result.
template <typename T>
std::vector<T> ma1_from_innovations(const std::vector<T> &innovations,
                                    const T &mean, const T &theta) {
  if (innovations.size() < 2) {
    return {};
  }

  std::vector<T> process(innovations.size() - 1);
  for (std::size_t i = 0; i < process.size(); ++i) {
    process[i] = mean + innovations[i + 1] + theta * innovations[i];
  }
  return process;
}

template <typename T>
T ma1_innovations_logpdf(const std::vector<T> &innovations,
                         const T &innovation_sd) {
  return iid_normal_logpdf(innovations, T(0.0), innovation_sd);
}

template <typename T>
T ma1_innovations_nll(const std::vector<T> &innovations,
                      const T &innovation_sd) {
  return -ma1_innovations_logpdf(innovations, innovation_sd);
}

} // namespace stats
} // namespace quadra

#endif
