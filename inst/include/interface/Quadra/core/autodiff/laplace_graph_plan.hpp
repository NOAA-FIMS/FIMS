#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../had_quadra.hpp"

namespace quadra {

// Cached dependency plan for Laplace derivatives. It records which objective
// ancestors depend on fixed effects, random effects, or both, allowing replay
// and reverse sweeps to skip fixed-only/report-only graph regions.
class LaplaceGraphPlan {
 public:
  void Build(const had::ADGraph& graph,
             const std::vector<had::VertexId>& fixed_inputs,
             const std::vector<had::VertexId>& random_inputs,
             had::VertexId objective) {
    if (objective >= graph.vertices.size())
      throw std::out_of_range("LaplaceGraphPlan objective is out of range");
    const size_t n = graph.vertices.size();
    fixed_dependent_.assign(n, false);
    random_dependent_.assign(n, false);
    objective_ancestor_.assign(n, false);
    for (auto id : fixed_inputs) {
      if (id >= n) throw std::out_of_range("fixed input is out of range");
      fixed_dependent_[id] = true;
    }
    for (auto id : random_inputs) {
      if (id >= n) throw std::out_of_range("random input is out of range");
      random_dependent_[id] = true;
    }

    // Vertices are recorded in topological order, so dependency tags flow
    // forward directly from each operation's parent edges.
    for (had::VertexId id = 1; id < static_cast<had::VertexId>(n); ++id) {
      const auto& vertex = graph.vertices[id];
      if (vertex.e1.to != id) {
        fixed_dependent_[id] =
            fixed_dependent_[id] || fixed_dependent_[vertex.e1.to];
        random_dependent_[id] =
            random_dependent_[id] || random_dependent_[vertex.e1.to];
      }
      if (vertex.e2.to != id) {
        fixed_dependent_[id] =
            fixed_dependent_[id] || fixed_dependent_[vertex.e2.to];
        random_dependent_[id] =
            random_dependent_[id] || random_dependent_[vertex.e2.to];
      }
    }

    // Mark only vertices that can contribute to the selected objective.
    objective_ancestor_[objective] = true;
    for (had::VertexId id = objective; id > 0; --id) {
      if (!objective_ancestor_[id]) continue;
      const auto& vertex = graph.vertices[id];
      if (vertex.e1.to != id) objective_ancestor_[vertex.e1.to] = true;
      if (vertex.e2.to != id) objective_ancestor_[vertex.e2.to] = true;
    }

    random_reverse_order_.clear();
    laplace_reverse_order_.clear();
    for (had::VertexId id = static_cast<had::VertexId>(n - 1); id > 0; --id) {
      if (!objective_ancestor_[id]) continue;
      if (random_dependent_[id]) random_reverse_order_.push_back(id);
      if (random_dependent_[id] || fixed_dependent_[id])
        laplace_reverse_order_.push_back(id);
    }
    objective_ = objective;
    vertex_count_ = n;
  }

  const std::vector<had::VertexId>& random_reverse_order() const {
    return random_reverse_order_;
  }
  const std::vector<had::VertexId>& laplace_reverse_order() const {
    return laplace_reverse_order_;
  }
  size_t vertex_count() const { return vertex_count_; }
  size_t random_active_count() const { return random_reverse_order_.size(); }
  size_t laplace_active_count() const { return laplace_reverse_order_.size(); }
  had::VertexId objective() const { return objective_; }
  bool random_active(had::VertexId id) const {
    return id < random_dependent_.size() && random_dependent_[id] &&
           objective_ancestor_[id];
  }
  bool laplace_active(had::VertexId id) const {
    return id < random_dependent_.size() && objective_ancestor_[id] &&
           (random_dependent_[id] || fixed_dependent_[id]);
  }

 private:
  std::vector<bool> fixed_dependent_;
  std::vector<bool> random_dependent_;
  std::vector<bool> objective_ancestor_;
  std::vector<had::VertexId> random_reverse_order_;
  std::vector<had::VertexId> laplace_reverse_order_;
  size_t vertex_count_ = 0;
  had::VertexId objective_ = 0;
};

inline void PropagateFirstOrderRestricted(
    had::ADGraph& graph, had::VertexId objective,
    const std::vector<had::VertexId>& reverse_order) {
  had::ZeroAdjoints(graph);
  graph.vertices[objective].w = 1.0;
  for (had::VertexId id : reverse_order) {
    had::ADVertex& vertex = graph.vertices[id];
    const double adjoint = vertex.w;
    if (vertex.e1.to != id)
      graph.vertices[vertex.e1.to].w += adjoint * vertex.e1.w;
    if (vertex.e2.to != id)
      graph.vertices[vertex.e2.to].w += adjoint * vertex.e2.w;
  }
}

template <class ActivePredicate>
inline void PropagateHessianRestricted(
    had::ADGraph& graph, const LaplaceGraphPlan& plan,
    const std::vector<had::VertexId>& reverse_order,
    ActivePredicate active) {
  had::g_ADGraph = &graph;
  had::ZeroAdjoints(graph);
  if (graph.soEdges.size() < graph.vertices.size())
    graph.soEdges.resize(graph.vertices.size());
  else
    for (auto& tree : graph.soEdges) tree.Clear();
  graph.selfSoEdges.assign(graph.vertices.size(), had::Real(0.0));
  graph.vertices[plan.objective()].w = had::Real(1.0);

  for (had::VertexId id : reverse_order) {
    had::ADVertex& vertex = graph.vertices[id];
    had::ADEdge& e1 = vertex.e1;
    had::ADEdge& e2 = vertex.e2;
    if (e1.to == id) continue;
    const bool e1_active = active(e1.to);
    const bool e2_active = e2.to != id && active(e2.to);

    auto& tree = graph.soEdges[id];
    for (const auto& node : tree.nodes) {
      if (!active(node.key)) continue;
      had::ADEdge second(node.key, node.val);
      if (e1_active) had::PushEdge(e1, second);
      if (e2_active) had::PushEdge(e2, second);
    }

    const had::Real diagonal = graph.selfSoEdges[id];
    if (diagonal != had::Real(0.0)) {
      if (e1_active)
        graph.selfSoEdges[e1.to] += e1.w * e1.w * diagonal;
      if (e2_active) {
        graph.selfSoEdges[e2.to] += e2.w * e2.w * diagonal;
        if (e1_active) {
          if (e1.to == e2.to)
            graph.selfSoEdges[e1.to] +=
                had::Real(2.0) * e1.w * e2.w * diagonal;
          else
            graph.soEdges[std::max(e1.to, e2.to)].Insert(
                std::min(e1.to, e2.to), e1.w * e2.w * diagonal);
        }
      }
    }

    const had::Real adjoint = vertex.w;
    if (adjoint != had::Real(0.0)) {
      if (vertex.soW != had::Real(0.0)) {
        if (e2.to == id) {
          if (e1_active)
            graph.selfSoEdges[e1.to] += adjoint * vertex.soW;
        } else if (e1.to == e2.to) {
          if (e1_active)
            graph.selfSoEdges[e1.to] +=
                had::Real(2.0) * adjoint * vertex.soW;
        } else if (e1_active && e2_active) {
          graph.soEdges[std::max(e1.to, e2.to)].Insert(
              std::min(e1.to, e2.to), adjoint * vertex.soW);
        }
      }
      vertex.w = had::Real(0.0);
      if (e1_active) graph.vertices[e1.to].w += adjoint * e1.w;
      if (e2_active) graph.vertices[e2.to].w += adjoint * e2.w;
    }
  }
}

inline void PropagateRandomHessianRestricted(had::ADGraph& graph,
                                             const LaplaceGraphPlan& plan) {
  PropagateHessianRestricted(
      graph, plan, plan.random_reverse_order(),
      [&plan](had::VertexId id) { return plan.random_active(id); });
}

// Exact second-order propagation over objective ancestors that depend on
// either partition. This retains the mixed random/fixed edges required for
// H_{u,theta}, while pruning constants and report-only graph regions.
inline void PropagateLaplaceHessianRestricted(had::ADGraph& graph,
                                              const LaplaceGraphPlan& plan) {
  PropagateHessianRestricted(
      graph, plan, plan.laplace_reverse_order(),
      [&plan](had::VertexId id) { return plan.laplace_active(id); });
}

}  // namespace quadra
