#ifndef QUADRA_PUBLIC_STATS_DISTRIBUTIONS_HPP
#define QUADRA_PUBLIC_STATS_DISTRIBUTIONS_HPP

#include "../../../math/distributions.hpp"

#include <vector>

namespace quadra {
namespace stats {

template <typename T> T normal_logpdf(const T &x, const T &mean, const T &sd) {
  return quadra::dnorm(x, mean, sd, true);
}

template <typename T> T normal_nll(const T &x, const T &mean, const T &sd) {
  return -normal_logpdf(x, mean, sd);
}

template <typename T>
T iid_normal_logpdf(const std::vector<T> &x, const T &mean, const T &sd) {
  T log_density = T(0.0);
  for (const T &value : x) {
    log_density += normal_logpdf(value, mean, sd);
  }
  return log_density;
}

template <typename T>
T iid_normal_nll(const std::vector<T> &x, const T &mean, const T &sd) {
  return -iid_normal_logpdf(x, mean, sd);
}

template <typename T>
T dirichlet_multinomial_logpmf(const std::vector<int> &x,
                               const std::vector<T> &weight,
                               const T &concentration) {
  return quadra::ddirichlet_multinomial_robust(x, weight, concentration, true);
}

template <typename T>
T dirichlet_multinomial_nll(const std::vector<int> &x,
                            const std::vector<T> &weight,
                            const T &concentration) {
  return -dirichlet_multinomial_logpmf(x, weight, concentration);
}

} // namespace stats
} // namespace quadra

#endif
