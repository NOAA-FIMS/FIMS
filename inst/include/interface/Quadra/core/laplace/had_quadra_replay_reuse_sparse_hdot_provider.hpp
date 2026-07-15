#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../had_quadra.hpp"
#include "had_quadra_sparse_exact_hdot_provider.hpp"
#include "sparse_trace_contraction.hpp"

namespace quadra {
namespace laplace {

// Reset graph state needed before each directional reverse sweep while keeping
// the already-replayed graph topology and objective variable alive.
inline void reset_had_quadra_directional_reverse_state(had::ADGraph &graph) {
  // Zero first-order adjoints and directional first-order adjoints.
  for (auto &v : graph.vertices) {
    v.w = had::Real(0.0);
    v.wDot = had::Real(0.0);
  }

  // Clear Hessian and directional-Hessian accumulators. PropagateAdjoint-
  // Directional also clears/resizes these, but doing it explicitly here makes
  // the reuse contract clear and robust across future had_quadra changes.
  for (auto &e : graph.soEdges) {
    e.Clear();
  }
  for (auto &e : graph.soEdgesDot) {
    e.Clear();
  }

  graph.selfSoEdges.assign(graph.vertices.size(), had::Real(0.0));
  graph.selfSoEdgesDot.assign(graph.vertices.size(), had::Real(0.0));
}

// Set primal tangent direction on both AReal handles and graph vertices.
inline void seed_had_quadra_direction(std::vector<had::AReal> &x,
                                      had::ADGraph &graph, int theta_dim,
                                      int random_dim, int theta_index) {
  const int n = theta_dim + random_dim;

  if (theta_index < 0 || theta_index >= theta_dim) {
    throw std::out_of_range("theta_index out of range.");
  }

  if (static_cast<int>(x.size()) != n) {
    throw std::invalid_argument("x has wrong length.");
  }

  for (int k = 0; k < n; ++k) {
    const double d = (k == theta_index) ? 1.0 : 0.0;
    x[static_cast<size_t>(k)].dot = d;
    graph.vertices[x[static_cast<size_t>(k)].varId].dot = d;
  }
}

inline void retangent_had_quadra_graph(had::ADGraph &graph) {
  using had::OpCode;
  using had::Real;

  auto vertex_dot = [&](had::VertexId id) -> Real {
    return graph.vertices[id].dot;
  };

  for (had::VertexId vid = 1;
       vid < static_cast<had::VertexId>(graph.vertices.size()); ++vid) {
    had::ADVertex &v = graph.vertices[vid];

    if (v.op == OpCode::Independent) {
      continue;
    }

    v.e1.dw = Real(0.0);
    v.e2.dw = Real(0.0);
    v.soWDot = Real(0.0);

    const Real c = v.constant;

    switch (v.op) {
    case OpCode::Add: {
      v.dot = vertex_dot(v.left) + vertex_dot(v.right);
      break;
    }

    case OpCode::AddConstant: {
      v.dot = vertex_dot(v.left);
      break;
    }

    case OpCode::Subtract: {
      v.dot = vertex_dot(v.left) - vertex_dot(v.right);
      break;
    }

    case OpCode::SubtractConstant: {
      v.dot = vertex_dot(v.left);
      break;
    }

    case OpCode::ConstantSubtract: {
      v.dot = -vertex_dot(v.left);
      break;
    }

    case OpCode::Multiply: {
      const Real lp = graph.vertices[v.left].primal;
      const Real rp = graph.vertices[v.right].primal;
      const Real ld = vertex_dot(v.left);
      const Real rd = vertex_dot(v.right);

      v.dot = ld * rp + lp * rd;
      v.e1.dw = rd;
      v.e2.dw = ld;
      v.soWDot = Real(0.0);
      break;
    }

    case OpCode::MultiplyConstant: {
      v.dot = c * vertex_dot(v.left);
      break;
    }

    case OpCode::DivideConstant: {
      v.dot = vertex_dot(v.left) / c;
      break;
    }

    case OpCode::ConstantDivide: {
      const Real xp = graph.vertices[v.left].primal;
      const Real xd = vertex_dot(v.left);

      const Real f2 = Real(2.0) * c / (xp * xp * xp);
      const Real f3 = Real(-6.0) * c / (xp * xp * xp * xp);

      v.dot = v.e1.w * xd;
      v.e1.dw = f2 * xd;
      v.soWDot = f3 * xd;
      break;
    }

    case OpCode::Divide: {
      const Real a = graph.vertices[v.left].primal;
      const Real b = graph.vertices[v.right].primal;
      const Real ad = vertex_dot(v.left);
      const Real bd = vertex_dot(v.right);

      v.dot = (ad * b - a * bd) / (b * b);
      v.e1.dw = -bd / (b * b);
      v.e2.dw = -ad / (b * b) + Real(2.0) * a * bd / (b * b * b);
      v.soWDot = Real(2.0) * bd / (b * b * b);
      break;
    }

    case OpCode::Exp: {
      const Real xd = vertex_dot(v.left);
      const Real fp = std::exp(graph.vertices[v.left].primal);

      v.dot = fp * xd;
      v.e1.dw = fp * xd;
      v.soWDot = fp * xd;
      break;
    }

    case OpCode::Log: {
      const Real xp = graph.vertices[v.left].primal;
      const Real xd = vertex_dot(v.left);

      const Real f1 = Real(1.0) / xp;
      const Real f2 = -Real(1.0) / (xp * xp);
      const Real f3 = Real(2.0) / (xp * xp * xp);

      v.dot = f1 * xd;
      v.e1.dw = f2 * xd;
      v.soWDot = f3 * xd;
      break;
    }

    case OpCode::Sqrt: {
      const Real xp = graph.vertices[v.left].primal;
      const Real xd = vertex_dot(v.left);
      const Real sqrt_x = std::sqrt(xp);

      const Real f1 = Real(0.5) / sqrt_x;
      const Real f2 = -Real(0.25) / (xp * sqrt_x);
      const Real f3 = Real(0.375) / (xp * xp * sqrt_x);

      v.dot = f1 * xd;
      v.e1.dw = f2 * xd;
      v.soWDot = f3 * xd;
      break;
    }

    case OpCode::Negate: {
      v.dot = -vertex_dot(v.left);
      break;
    }

    default:
      throw std::logic_error("retangent_had_quadra_graph: unsupported OpCode.");
    }
  }
}

// Replay-reuse provider.
//
// CombinedObjectiveFn must be callable as:
//
//   template <class AD>
//   AD operator()(const std::vector<AD>& x) const;
//
// where x = [theta, u].
template <class CombinedObjectiveFn>
class HadQuadraReplayReuseSparseHdotProvider {
public:
  HadQuadraReplayReuseSparseHdotProvider(CombinedObjectiveFn combined_objective,
                                         int theta_dim, int random_dim,
                                         RandomHessianPattern pattern,
                                         std::vector<int> active_directions,
                                         double drop_tol = 0.0)
      : combined_objective_(std::move(combined_objective)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(pattern)),
        active_directions_(std::move(active_directions)), drop_tol_(drop_tol) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (drop_tol_ < 0.0 || !std::isfinite(drop_tol_)) {
      throw std::invalid_argument("drop_tol must be nonnegative and finite.");
    }

    std::sort(active_directions_.begin(), active_directions_.end());
    active_directions_.erase(
        std::unique(active_directions_.begin(), active_directions_.end()),
        active_directions_.end());

    for (int idx : active_directions_) {
      if (idx < 0 || idx >= theta_dim_) {
        throw std::out_of_range("active direction out of range.");
      }
    }

    for (const auto &entry : pattern_) {
      if (entry.first < 0 || entry.first >= random_dim_ || entry.second < 0 ||
          entry.second >= random_dim_) {
        throw std::out_of_range("random Hessian pattern entry out of range.");
      }
    }
  }

  std::vector<Eigen::SparseMatrix<double>>
  compute_all_sparse(const Eigen::VectorXd &theta,
                     const Eigen::VectorXd &uhat) const {
    if (theta.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }

    had::ADGraph graph;
    had::g_ADGraph = &graph;

    std::vector<had::AReal> x;
    x.reserve(static_cast<size_t>(theta_dim_ + random_dim_));

    // Build variables with zero tangents for replay.
    for (int j = 0; j < theta_dim_; ++j) {
      x.emplace_back(theta[j]);
      x.back().dot = 0.0;
      graph.vertices[x.back().varId].dot = 0.0;
    }

    for (int i = 0; i < random_dim_; ++i) {
      x.emplace_back(uhat[i]);
      x.back().dot = 0.0;
      graph.vertices[x.back().varId].dot = 0.0;
    }

    // Replay objective once.
    had::AReal y = combined_objective_(x);

    std::vector<Eigen::SparseMatrix<double>> out;
    out.resize(static_cast<size_t>(theta_dim_));

    for (int j = 0; j < theta_dim_; ++j) {
      if (!is_active(j)) {
        out[static_cast<size_t>(j)].resize(random_dim_, random_dim_);
        out[static_cast<size_t>(j)].makeCompressed();
        continue;
      }

      reset_had_quadra_directional_reverse_state(graph);
      seed_had_quadra_direction(x, graph, theta_dim_, random_dim_, j);
      retangent_had_quadra_graph(graph);

      had::SetAdjoint(y, had::Real(1.0));
      had::PropagateAdjointDirectional();

      out[static_cast<size_t>(j)] = extract_sparse_hdot(x);
    }

    had::g_ADGraph = nullptr;
    return out;
  }

  Eigen::SparseMatrix<double> sparse(const Eigen::VectorXd &theta,
                                     const Eigen::VectorXd &uhat,
                                     int theta_index) const {
    if (theta_index < 0 || theta_index >= theta_dim_) {
      throw std::out_of_range("theta_index out of range.");
    }

    const auto all = compute_all_sparse(theta, uhat);
    return all[static_cast<size_t>(theta_index)];
  }

  Eigen::MatrixXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat,
                             int theta_index) const {
    return Eigen::MatrixXd(sparse(theta, uhat, theta_index));
  }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

private:
  CombinedObjectiveFn combined_objective_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  double drop_tol_;

  bool is_active(int theta_index) const {
    return std::binary_search(active_directions_.begin(),
                              active_directions_.end(), theta_index);
  }

  Eigen::SparseMatrix<double>
  extract_sparse_hdot(const std::vector<had::AReal> &x) const {
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(pattern_.size() * 2);

    for (const auto &entry : pattern_) {
      const int a = entry.first;
      const int b = entry.second;

      const int ia = theta_dim_ + a;
      const int ib = theta_dim_ + b;

      double value = had::GetAdjointDot(x[static_cast<size_t>(ia)],
                                        x[static_cast<size_t>(ib)]);

      if (std::abs(value) <= drop_tol_) {
        continue;
      }

      triplets.emplace_back(a, b, value);
      if (a != b) {
        triplets.emplace_back(b, a, value);
      }
    }

    Eigen::SparseMatrix<double> Hdot(random_dim_, random_dim_);
    Hdot.setFromTriplets(triplets.begin(), triplets.end());
    Hdot.makeCompressed();
    return Hdot;
  }
};

template <class CombinedObjectiveFn>
auto make_had_quadra_replay_reuse_sparse_hdot_provider(
    CombinedObjectiveFn combined_objective, int theta_dim, int random_dim,
    RandomHessianPattern pattern, std::vector<int> active_directions,
    double drop_tol = 0.0) {
  return HadQuadraReplayReuseSparseHdotProvider<CombinedObjectiveFn>(
      std::move(combined_objective), theta_dim, random_dim, std::move(pattern),
      std::move(active_directions), drop_tol);
}

// Compute exact Laplace gradient using all replay-reused Hdot_j objects.
// This avoids calling provider.sparse(...) repeatedly, which would rebuild the
// graph each time through the one-direction-compatible API.
template <class HessianUUFn, class ReplayReuseProvider>
Eigen::VectorXd full_exact_laplace_gradient_replay_reuse_hdot(
    const Eigen::VectorXd &grad_joint_envelope,
    const HessianUUFn &hessian_uu_at_fixed_uhat,
    const ReplayReuseProvider &provider, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat) {
  if (grad_joint_envelope.size() != theta.size()) {
    throw std::invalid_argument(
        "grad_joint_envelope and theta must have same length.");
  }

  const Eigen::MatrixXd H = hessian_uu_at_fixed_uhat(theta, uhat);
  Eigen::LDLT<Eigen::MatrixXd> ldlt(H);
  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("LDLT factorization failed.");
  }

  const auto Hdots = provider.compute_all_sparse(theta, uhat);

  if (static_cast<int>(Hdots.size()) != theta.size()) {
    throw std::runtime_error(
        "provider returned wrong number of Hdot matrices.");
  }

  Eigen::VectorXd grad = grad_joint_envelope;

  for (int j = 0; j < theta.size(); ++j) {
    if (Hdots[static_cast<size_t>(j)].nonZeros() == 0) {
      continue;
    }

    const Eigen::MatrixXd rhs = Eigen::MatrixXd(Hdots[static_cast<size_t>(j)]);
    const double trace = ldlt.solve(rhs).trace();
    grad[j] += 0.5 * trace;
  }

  return grad;
}

// Same as full_exact_laplace_gradient_replay_reuse_hdot(...), but computes
// trace(H^{-1} Hdot_j) using cached selected inverse columns instead of dense
// RHS solves.
template <class HessianUUFn, class ReplayReuseProvider>
Eigen::VectorXd full_exact_laplace_gradient_replay_reuse_cached_trace(
    const Eigen::VectorXd &grad_joint_envelope,
    const HessianUUFn &hessian_uu_at_fixed_uhat,
    const ReplayReuseProvider &provider, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat) {
  if (grad_joint_envelope.size() != theta.size()) {
    throw std::invalid_argument(
        "grad_joint_envelope and theta must have same length.");
  }

  const Eigen::MatrixXd H = hessian_uu_at_fixed_uhat(theta, uhat);

  Eigen::LDLT<Eigen::MatrixXd> ldlt(H);
  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("LDLT factorization failed.");
  }

  const auto Hdots = provider.compute_all_sparse(theta, uhat);

  if (static_cast<int>(Hdots.size()) != theta.size()) {
    throw std::runtime_error(
        "provider returned wrong number of Hdot matrices.");
  }

  const auto needed_columns = needed_columns_from_sparse_matrices(Hdots);
  SelectedInverseColumnTraceCache trace_cache(ldlt, static_cast<int>(H.rows()),
                                              needed_columns);

  Eigen::VectorXd grad = grad_joint_envelope;

  for (int j = 0; j < theta.size(); ++j) {
    const auto &Hdot = Hdots[static_cast<size_t>(j)];
    if (Hdot.nonZeros() == 0) {
      continue;
    }

    const double trace = trace_cache.trace(Hdot);
    grad[j] += 0.5 * trace;
  }

  return grad;
}

} // namespace laplace
} // namespace quadra
