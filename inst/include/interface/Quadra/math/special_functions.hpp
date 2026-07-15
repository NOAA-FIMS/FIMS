#ifndef QUADRA_SPECIAL_FUNCTIONS_HPP
#define QUADRA_SPECIAL_FUNCTIONS_HPP
#include "../core/autodiff.hpp"

inline double digamma(double x) {
  double result = 0.0;

  while (x < 6.0) {
    result -= 1.0 / x;
    x += 1.0;
  }

  double inv = 1.0 / x;
  double inv2 = inv * inv;

  result += std::log(x) - 0.5 * inv -
            inv2 * (1.0 / 12.0 - inv2 * (1.0 / 120.0 - inv2 * (1.0 / 252.0)));

  return result;
}

inline double trigamma(double x) {
  double result = 0.0;

  while (x < 6.0) {
    result += 1.0 / (x * x);
    x += 1.0;
  }

  double inv = 1.0 / x;
  double inv2 = inv * inv;

  result +=
      inv + 0.5 * inv2 +
      inv2 * inv * (1.0 / 6.0 - inv2 * (1.0 / 30.0 + inv2 * (1.0 / 42.0)));

  return result;
}

inline had::AReal lgamma(const had::AReal &x) {
  double val = std::lgamma(x.val);

  had::AReal ret = had::NewAReal(val);

  had::AddEdge(ret, x,
               digamma(x.val), // first derivative
               trigamma(x.val) // second derivative
  );

  return ret;
}

// inline double lgamma(const double x)
// {
//     return std::lgamma(x);
// }

#endif // QUADRA_SPECIAL_FUNCTIONS_HPP