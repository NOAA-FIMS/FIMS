#ifndef QUADRA_DISTRIBUTIONS_HPP
#define QUADRA_DISTRIBUTIONS_HPP

#include "special_functions.hpp"
#include <cmath>
#include <vector>

namespace quadra {

constexpr double LOG_SQRT_2PI = 0.91893853320467274178;
constexpr double LOG_2PI = 1.83787706640934548356;

// --------------------------------------------------
// Normal
// --------------------------------------------------
template <typename T>
T dnorm(const T &x, const T &mean, const T &sd, bool give_log = true) {
  T z = (x - mean) / sd;
  T logp = -0.5 * z * z - log(sd) - LOG_SQRT_2PI;
  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Lognormal
// --------------------------------------------------
template <typename T>
T dlnorm(const T &x, const T &meanlog, const T &sdlog, bool give_log = true) {
  T z = (log(x) - meanlog) / sdlog;
  T logp = -0.5 * z * z - log(sdlog) - log(x) - LOG_SQRT_2PI;
  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Poisson
// --------------------------------------------------
template <typename T>
T dpois(const int &x, const T &lambda, bool give_log = true) {
  T xx = T(static_cast<double>(x));
  double lg = std::lgamma(static_cast<double>(x) + 1.0); // precompute for AD
  T logp = xx * log(lambda) - lambda - T(lg);

  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Binomial
// --------------------------------------------------
template <typename T>
T dbinom(const int &x, const int &n, const T &p, bool give_log = true) {
  T log_choose =
      std::lgamma(n + 1.0) - std::lgamma(x + 1.0) - std::lgamma(n - x + 1.0);

  T logp = log_choose + x * log(p) + (n - x) * log(1.0 - p);
  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Multinomial
// --------------------------------------------------
template <typename T>
T dmultinom(const std::vector<int> &x, const std::vector<T> &p,
            bool give_log = true) {
  int N = 0;
  for (int xi : x)
    N += xi;

  T logp = std::lgamma(N + 1.0);

  for (size_t i = 0; i < x.size(); ++i) {
    logp -= std::lgamma(x[i] + 1.0);
    logp += x[i] * log(p[i]);
  }

  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Gamma: shape/rate parameterization
// --------------------------------------------------
template <typename T>
T dgamma(const T &x, const T &shape, const T &rate, bool give_log = true) {
  T logp =
      shape * log(rate) - lgamma(shape) + (shape - 1.0) * log(x) - rate * x;

  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Beta
// --------------------------------------------------
template <typename T>
T dbeta(const T &x, const T &a, const T &b, bool give_log = true) {
  T logp = lgamma(a + b) - lgamma(a) - lgamma(b) + (a - 1.0) * log(x) +
           (b - 1.0) * log(1.0 - x);

  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Negative binomial: mean/overdispersion form
//
// var = mu + mu^2 / phi
// --------------------------------------------------
template <typename T>
T dnbinom2(const int &x, const T &mu, const T &phi, bool give_log = true) {
  T logp = lgamma(x + phi) - lgamma(phi) - std::lgamma(x + 1.0) +
           phi * log(phi / (phi + mu)) + x * log(mu / (phi + mu));

  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Dirichlet-multinomial
//
// alpha_i = total_concentration * p_i
// --------------------------------------------------
template <typename T>
T ddirichlet_multinomial(const std::vector<int> &x, const std::vector<T> &p,
                         const T &theta, bool give_log = true) {
  int N = 0;
  for (int xi : x)
    N += xi;

  T logp = std::lgamma(N + 1.0);
  for (int xi : x)
    logp -= std::lgamma(xi + 1.0);

  T alpha_sum = theta;

  logp += lgamma(alpha_sum);
  logp -= lgamma(alpha_sum + N);

  for (size_t i = 0; i < x.size(); ++i) {
    T alpha_i = theta * p[i];

    logp += lgamma(alpha_i + x[i]);
    logp -= lgamma(alpha_i);
  }

  return give_log ? logp : exp(logp);
}

// --------------------------------------------------
// Dirichlet-multinomial linear form
//
// Linear form based on expected proportions p_i and
// overdispersion theta:
//
// alpha_i = p_i / theta
// alpha_0 = 1 / theta
//
// Smaller theta -> multinomial-like.
// Larger theta -> more overdispersed.
// --------------------------------------------------
template <typename T>
T ddirichlet_multinomial_linear(const std::vector<int> &x,
                                const std::vector<T> &p, const T &theta,
                                bool give_log = true) {
  int N = 0;
  for (int xi : x)
    N += xi;

  T alpha0 = 1.0 / theta;

  T logp = std::lgamma(N + 1.0);
  for (int xi : x)
    logp -= std::lgamma(xi + 1.0);

  logp += lgamma(alpha0);
  logp -= lgamma(alpha0 + N);

  for (size_t i = 0; i < x.size(); ++i) {
    T alpha_i = p[i] / theta;

    logp += lgamma(alpha_i + x[i]);
    logp -= lgamma(alpha_i);
  }

  return give_log ? logp : exp(logp);
}

} // namespace quadra

#endif // QUADRA_DISTRIBUTIONS_HPP