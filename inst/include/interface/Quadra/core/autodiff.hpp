#ifndef QUADRA_AUTODIFF_HPP
#define QUADRA_AUTODIFF_HPP

#pragma once

#include <algorithm>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef QUADRA_USE_ORIGINAL_HAD
#include "had_quadra.hpp"
#else
#include "had/had.h"
#endif

#include "../external/eigen/Eigen/Dense"
#include "../external/eigen/Eigen/Sparse"

namespace quadra {

using AD = had::AReal;

#ifndef QUADRA_USE_ORIGINAL_HAD
using AD3 = had::ThirdOrderScalar;
#endif

enum class DerivativeLevel { Value, Gradient, Hessian, ThirdDirectional };

template <typename T> struct DerivativeTraits {
  static constexpr DerivativeLevel level = DerivativeLevel::Value;
};

template <> struct DerivativeTraits<had::AReal> {
  static constexpr DerivativeLevel level = DerivativeLevel::Hessian;
};

#ifndef QUADRA_USE_ORIGINAL_HAD
template <> struct DerivativeTraits<had::ThirdOrderScalar> {
  static constexpr DerivativeLevel level = DerivativeLevel::ThirdDirectional;
};
#endif

struct ADResult {
  double value = 0.0;
  Eigen::VectorXd gradient;
  Eigen::MatrixXd hessian;
};

#ifndef QUADRA_USE_ORIGINAL_HAD
struct ThirdDirectionalResult {
  double value = 0.0;
  double first = 0.0;  // df(x)[d]
  double second = 0.0; // d^T H(x) d
  double third = 0.0;  // D^3 f(x)[d,d,d]
};
#endif

//--------------------------------------------------
// Scalar value extraction. Keep core code from depending
// directly on .val so double, AReal, and AD3 all work.
//--------------------------------------------------
inline double value_of(const double &x) { return x; }

inline double value_of(const had::AReal &x) {
#ifndef QUADRA_USE_ORIGINAL_HAD
  if (had::g_ADGraph) {
    return had::g_ADGraph->vertices[x.varId].primal;
  }
#endif
  return x.val;
}

#ifndef QUADRA_USE_ORIGINAL_HAD
inline double value_of(const had::ThirdOrderScalar &x) { return x.val; }
#endif

template <typename T> inline double value_of_arithmetic_or_ad(const T &x) {
  if constexpr (std::is_arithmetic_v<T>)
    return static_cast<double>(x);
  else
    return value_of(x);
}

//--------------------------------------------------
// Reusable tape context. This centralizes graph lifetime
// and makes it cheaper to reuse a graph across evaluations.
//--------------------------------------------------
struct TapeContext {
  had::ADGraph graph;

  TapeContext() { reset(); }

  TapeContext(const TapeContext &) = delete;
  TapeContext &operator=(const TapeContext &) = delete;

  void reset() {
    had::g_ADGraph = &graph;
    graph.Clear();
  }

  void forward() {
    had::g_ADGraph = &graph;
    had::Forward(graph);
  }

  void zero_adjoints() {
    had::g_ADGraph = &graph;
    had::ZeroAdjoints(graph);
  }
};

struct ADScope {
  had::ADGraph &graph;

  explicit ADScope(had::ADGraph &g) : graph(g) {
    had::g_ADGraph = &graph;
    graph.Clear();
  }

  template <typename T> double value(const T &loss) const {
    return value_of_arithmetic_or_ad(loss);
  }

  void forward() const {
    had::g_ADGraph = &graph;
    had::Forward(graph);
  }

  void zero_adjoints() const {
    had::g_ADGraph = &graph;
    had::ZeroAdjoints(graph);
  }

  void backward(const had::AReal &loss) {
    had::SetAdjoint(loss, 1.0);
    had::PropagateAdjoint();
  }

  double grad(const had::AReal &x) const { return had::GetAdjoint(x); }

  double hess(const had::AReal &x, const had::AReal &y) const {
    return had::GetAdjoint(x, y);
  }

  void clear() { graph.Clear(); }
};

//--------------------------------------------------
// Reset accumulated AD adjoints without clearing graph structure
//--------------------------------------------------

inline void forward(TapeContext &tape) { tape.forward(); }

inline void zero_adjoints(had::ADGraph &graph) { had::ZeroAdjoints(graph); }

inline void zero_adjoints(TapeContext &tape) { tape.zero_adjoints(); }

//--------------------------------------------------
// Reusable tape value update and forward replay helpers
//--------------------------------------------------
inline void set_value(had::AReal &x, const double value) {
  had::SetValue(x, value);
} //--------------------------------------------------
// Convert Eigen/std vectors to scalar vectors
//--------------------------------------------------
template <typename Scalar = AD>
inline std::vector<Scalar> to_scalar(const Eigen::VectorXd &x) {
  std::vector<Scalar> out;
  out.reserve(static_cast<size_t>(x.size()));

  for (int i = 0; i < x.size(); ++i)
    out.emplace_back(static_cast<double>(x[i]));

  return out;
}

template <typename Scalar = AD>
inline std::vector<Scalar> to_scalar(const std::vector<double> &x) {
  std::vector<Scalar> out;
  out.reserve(x.size());

  for (double v : x)
    out.emplace_back(v);

  return out;
}

inline std::vector<AD> to_ad(const Eigen::VectorXd &x) {
  return to_scalar<AD>(x);
}

inline std::vector<AD> to_ad(const std::vector<double> &x) {
  return to_scalar<AD>(x);
}

#ifndef QUADRA_USE_ORIGINAL_HAD
inline std::vector<AD3> to_ad3(const std::vector<double> &x,
                               const std::vector<double> &direction) {
  if (x.size() != direction.size())
    throw std::invalid_argument(
        "to_ad3: x and direction must have the same size");

  std::vector<AD3> out;
  out.reserve(x.size());
  for (size_t i = 0; i < x.size(); ++i)
    out.emplace_back(x[i], direction[i]);
  return out;
}
#endif

//--------------------------------------------------
// Extract gradients AFTER reverse pass
//--------------------------------------------------
inline Eigen::VectorXd extract_gradient(const std::vector<AD> &x) {
  Eigen::VectorXd g(static_cast<int>(x.size()));

  for (size_t i = 0; i < x.size(); ++i)
    g[static_cast<int>(i)] = had::GetAdjoint(x[i]);

  return g;
}

inline double get_hessian(const AD &xi, const AD &xj) {
  return had::GetAdjoint(xi, xj);
}

#ifndef QUADRA_USE_ORIGINAL_HAD
//--------------------------------------------------
// Exact directional derivatives up to third order.
// Model must be templated on scalar type.
//--------------------------------------------------
template <typename ModelOrFunctor>
ThirdDirectionalResult
evaluate_third_directional(ModelOrFunctor &&f, const std::vector<double> &x,
                           const std::vector<double> &direction) {
  auto had_res = had::evaluate_directional_derivatives3(
      std::forward<ModelOrFunctor>(f), x, direction);

  ThirdDirectionalResult out;
  out.value = had_res.value;
  out.first = had_res.first;
  out.second = had_res.second;
  out.third = had_res.third;
  return out;
}
#endif

struct HessianPattern {
  int n = 0;
  std::vector<Eigen::Triplet<double>> triplets;
  std::vector<std::pair<int, int>> pairs;

  Eigen::SparseMatrix<double> to_sparse_matrix() const {
    Eigen::SparseMatrix<double> H(n, n);
    H.setFromTriplets(triplets.begin(), triplets.end());
    return H;
  }
};

inline HessianPattern extract_hessian_pattern_from_graph(
    const std::vector<had::AReal> &independent_vars, bool symmetric = true) {
  HessianPattern pattern;
  pattern.n = static_cast<int>(independent_vars.size());

  std::set<std::pair<int, int>> unique_pairs;

  for (int i = 0; i < pattern.n; ++i) {
    const auto vi = independent_vars[i].varId;

    // Diagonal
    if (vi < had::g_ADGraph->selfSoEdges.size() &&
        had::g_ADGraph->selfSoEdges[vi] != had::Real(0.0)) {
      unique_pairs.insert({i, i});
    }

    // Off-diagonal
    if (vi < had::g_ADGraph->soEdges.size()) {
      const auto &tree = had::g_ADGraph->soEdges[vi];

      for (const auto &node : tree.nodes) {
        auto vj = node.key;

        for (int j = 0; j < pattern.n; ++j) {
          if (independent_vars[j].varId == vj && node.val != had::Real(0.0)) {

            unique_pairs.insert({i, j});

            if (symmetric) {
              unique_pairs.insert({j, i});
            }
          }
        }
      }
    }
  }

  for (auto [i, j] : unique_pairs) {
    pattern.pairs.push_back({i, j});
    pattern.triplets.emplace_back(i, j, 1.0);
  }

  return pattern;
}

} // namespace quadra

#endif // QUADRA_AUTODIFF_HPP
