#ifndef QUADRA_PUBLIC_STATS_TRANSFORMS_HPP
#define QUADRA_PUBLIC_STATS_TRANSFORMS_HPP

#include <cmath>

namespace quadra {
namespace stats {

template <typename T> T positive_from_unconstrained(const T &x) {
  using std::exp;
  return exp(x);
}

template <typename T> T correlation_from_unconstrained(const T &x) {
  using std::exp;
  return T(2.0) / (T(1.0) + exp(-x)) - T(1.0);
}

} // namespace stats
} // namespace quadra

#endif
