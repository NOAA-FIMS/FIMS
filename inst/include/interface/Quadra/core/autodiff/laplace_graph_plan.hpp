#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../had_quadra.hpp"

namespace quadra {

inline void AddOffDiagonalHessian(had::ADGraph &graph, had::VertexId a,
                                  had::VertexId b, had::Real value) {
  const had::VertexId outer = std::max(a, b);
  const had::VertexId inner = std::min(a, b);
  graph.InsertSoEdge(outer, inner, value);
}

template <class Consumer>
inline void ForEachOffDiagonalHessian(had::ADGraph &graph, had::VertexId outer,
                                      Consumer consumer) {
  graph.ForEachSoEdge(outer, consumer);
}

inline had::Real GetOffDiagonalHessian(const had::ADGraph &graph,
                                       had::VertexId a, had::VertexId b) {
  return graph.QuerySoEdge(std::max(a, b), std::min(a, b));
}

// Reset an ordinary Hessian sweep without releasing the per-vertex BTree
// buffers. The graph sparsity is stable across parameter replay, so retaining
// node capacity avoids rebuilding thousands of small allocations.
inline void ResetHessianSweepPreserveStorage(
    had::ADGraph &graph,
    const std::vector<had::VertexId> &active_reverse_order) {
  graph.EnsureSoTreeCount(graph.vertices.size());
  if (graph.selfSoEdges.size() < graph.vertices.size())
    graph.selfSoEdges.resize(graph.vertices.size(), had::Real(0.0));
  // Vertex zero may be a parameter input. Reverse-order plans omit it because
  // an input has no parents to propagate into, but its accumulated adjoint and
  // Hessian slots still have to be reset between evaluations.
  if (!graph.vertices.empty()) {
    graph.vertices[0].w = had::Real(0.0);
    if (graph.HasScalarDirectionalStorage())
      graph.scalarDirectional[0].wDot = had::Real(0.0);
    graph.ZeroSoEdgeValues(0);
    graph.selfSoEdges[0] = had::Real(0.0);
  }
  for (had::VertexId id : active_reverse_order) {
    graph.vertices[id].w = had::Real(0.0);
    if (graph.HasScalarDirectionalStorage())
      graph.scalarDirectional[id].wDot = had::Real(0.0);
    graph.ZeroSoEdgeValues(id);
    graph.selfSoEdges[id] = had::Real(0.0);
  }
}

// Cached dependency plan for Laplace derivatives. It records which objective
// ancestors depend on fixed effects, random effects, or both, allowing replay
// and reverse sweeps to skip fixed-only/report-only graph regions.
class LaplaceGraphPlan {
public:
  void Build(const had::ADGraph &graph,
             const std::vector<had::VertexId> &fixed_inputs,
             const std::vector<had::VertexId> &random_inputs,
             had::VertexId objective) {
    if (objective >= graph.vertices.size())
      throw std::out_of_range("LaplaceGraphPlan objective is out of range");
    const size_t n = graph.vertices.size();
    fixed_dependent_.assign(n, false);
    random_dependent_.assign(n, false);
    objective_ancestor_.assign(n, false);
    for (auto id : fixed_inputs) {
      if (id >= n)
        throw std::out_of_range("fixed input is out of range");
      fixed_dependent_[id] = true;
    }
    for (auto id : random_inputs) {
      if (id >= n)
        throw std::out_of_range("random input is out of range");
      random_dependent_[id] = true;
    }

    // Vertices are recorded in topological order, so dependency tags flow
    // forward directly from each operation's parent edges.
    for (had::VertexId id = 1; id < static_cast<had::VertexId>(n); ++id) {
      const auto &vertex = graph.vertices[id];
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
      if (!objective_ancestor_[id])
        continue;
      const auto &vertex = graph.vertices[id];
      if (vertex.e1.to != id)
        objective_ancestor_[vertex.e1.to] = true;
      if (vertex.e2.to != id)
        objective_ancestor_[vertex.e2.to] = true;
    }

    random_reverse_order_.clear();
    laplace_reverse_order_.clear();
    for (had::VertexId id = static_cast<had::VertexId>(n - 1); id > 0; --id) {
      if (!objective_ancestor_[id])
        continue;
      if (random_dependent_[id])
        random_reverse_order_.push_back(id);
      if (random_dependent_[id] || fixed_dependent_[id])
        laplace_reverse_order_.push_back(id);
    }

    objective_ = objective;
    vertex_count_ = n;
  }

  const std::vector<had::VertexId> &random_reverse_order() const {
    return random_reverse_order_;
  }
  const std::vector<had::VertexId> &laplace_reverse_order() const {
    return laplace_reverse_order_;
  }
  // Compatibility surface for callers recorded against the earlier
  // selective-forward API. ADGraph now deliberately performs a full replay.
  const std::vector<had::VertexId> &laplace_forward_order() const {
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
  // Byte masks avoid the proxy reads and read-modify-write operations of
  // vector<bool> in the inner restricted-Hessian propagation loops.
  std::vector<std::uint8_t> fixed_dependent_;
  std::vector<std::uint8_t> random_dependent_;
  std::vector<std::uint8_t> objective_ancestor_;
  std::vector<had::VertexId> random_reverse_order_;
  std::vector<had::VertexId> laplace_reverse_order_;
  size_t vertex_count_ = 0;
  had::VertexId objective_ = 0;
};

inline void
PropagateFirstOrderRestricted(had::ADGraph &graph, had::VertexId objective,
                              const std::vector<had::VertexId> &reverse_order) {
  had::ZeroAdjoints(graph);
  graph.vertices[objective].w = 1.0;
  for (had::VertexId id : reverse_order) {
    had::ADVertex &vertex = graph.vertices[id];
    const double adjoint = vertex.w;
    if (vertex.e1.to != id)
      graph.vertices[vertex.e1.to].w += adjoint * vertex.e1.w;
    if (vertex.e2.to != id)
      graph.vertices[vertex.e2.to].w += adjoint * vertex.e2.w;
  }
}

template <class ActivePredicate>
inline void
PropagateHessianRestricted(had::ADGraph &graph, const LaplaceGraphPlan &plan,
                           const std::vector<had::VertexId> &reverse_order,
                           ActivePredicate active,
                           std::size_t *operation_count = nullptr) {
  had::g_ADGraph = &graph;
  ResetHessianSweepPreserveStorage(graph, reverse_order);
  graph.vertices[plan.objective()].w = had::Real(1.0);

  for (had::VertexId id : reverse_order) {
    had::ADVertex &vertex = graph.vertices[id];
    had::ADEdge &e1 = vertex.e1;
    had::ADEdge &e2 = vertex.e2;
    if (e1.to == id)
      continue;
    const bool e1_active = active(e1.to);
    const bool e2_active = e2.to != id && active(e2.to);

    ForEachOffDiagonalHessian(
        graph, id, [&](had::VertexId key, had::Real value) {
          if (!active(key))
            return;
          had::ADEdge second(key, value);
          if (e1_active) {
            if (e1.to == second.to)
              graph.selfSoEdges[e1.to] += had::Real(2.0) * e1.w * second.w;
            else
              AddOffDiagonalHessian(graph, e1.to, second.to, e1.w * second.w);
            if (operation_count)
              ++*operation_count;
          }
          if (e2_active) {
            if (e2.to == second.to)
              graph.selfSoEdges[e2.to] += had::Real(2.0) * e2.w * second.w;
            else
              AddOffDiagonalHessian(graph, e2.to, second.to, e2.w * second.w);
            if (operation_count)
              ++*operation_count;
          }
        });

    const had::Real diagonal = graph.selfSoEdges[id];
    if (diagonal != had::Real(0.0)) {
      if (e1_active) {
        graph.selfSoEdges[e1.to] += e1.w * e1.w * diagonal;
        if (operation_count)
          ++*operation_count;
      }
      if (e2_active) {
        graph.selfSoEdges[e2.to] += e2.w * e2.w * diagonal;
        if (operation_count)
          ++*operation_count;
        if (e1_active) {
          if (e1.to == e2.to)
            graph.selfSoEdges[e1.to] += had::Real(2.0) * e1.w * e2.w * diagonal;
          else
            AddOffDiagonalHessian(graph, e1.to, e2.to, e1.w * e2.w * diagonal);
          if (operation_count)
            ++*operation_count;
        }
      }
    }

    const had::Real adjoint = vertex.w;
    if (adjoint != had::Real(0.0)) {
      if (vertex.soW != had::Real(0.0)) {
        if (e2.to == id) {
          if (e1_active) {
            graph.selfSoEdges[e1.to] += adjoint * vertex.soW;
            if (operation_count)
              ++*operation_count;
          }
        } else if (e1.to == e2.to) {
          if (e1_active) {
            graph.selfSoEdges[e1.to] += had::Real(2.0) * adjoint * vertex.soW;
            if (operation_count)
              ++*operation_count;
          }
        } else if (e1_active && e2_active) {
          AddOffDiagonalHessian(graph, e1.to, e2.to, adjoint * vertex.soW);
          if (operation_count)
            ++*operation_count;
        }
      }
      vertex.w = had::Real(0.0);
      if (e1_active)
        graph.vertices[e1.to].w += adjoint * e1.w;
      if (e2_active)
        graph.vertices[e2.to].w += adjoint * e2.w;
    }
  }
}

inline bool PropagateRandomHessianFrozen(had::ADGraph &graph,
                                         const LaplaceGraphPlan &plan,
                                         std::size_t *operation_count) {
  if (!graph.sharedSoTopology)
    return false;
  constexpr std::uint32_t no_destination =
      std::numeric_limits<std::uint32_t>::max();
  constexpr std::uint32_t diagonal_destination = no_destination - 1U;
  auto topology = graph.sharedSoTopology;
  try {
    std::call_once(topology->random_destination_once, [&]() {
      topology->random_destination_e1.assign(topology->nodes.size(),
                                              no_destination);
      topology->random_destination_e2.assign(topology->nodes.size(),
                                              no_destination);
      const auto set_destination = [&](std::vector<std::uint32_t> &destinations,
                                       std::size_t source_slot,
                                       had::VertexId target,
                                       had::VertexId key) {
        if (!plan.random_active(target) || !plan.random_active(key))
          return;
        if (target == key) {
          destinations[source_slot] = diagonal_destination;
          return;
        }
        const std::size_t destination = graph.FindSharedSoSlot(
            std::max(target, key), std::min(target, key));
        if (destination >= diagonal_destination)
          throw std::out_of_range("frozen random-Hessian destination absent");
        destinations[source_slot] = static_cast<std::uint32_t>(destination);
      };
      for (had::VertexId id : plan.random_reverse_order()) {
        const had::ADVertex &vertex = graph.vertices[id];
        if (vertex.e1.to == id)
          continue;
        const std::size_t begin = topology->offsets[id];
        const std::size_t end = topology->offsets[id + 1];
        for (std::size_t slot = begin; slot < end; ++slot) {
          const had::VertexId key = topology->nodes[slot].key;
          set_destination(topology->random_destination_e1, slot,
                          vertex.e1.to, key);
          if (vertex.e2.to != id)
            set_destination(topology->random_destination_e2, slot,
                            vertex.e2.to, key);
        }
      }
    });
  } catch (const std::out_of_range &) {
    graph.ThawSharedHessianTopology();
    return false;
  }

  had::g_ADGraph = &graph;
  ResetHessianSweepPreserveStorage(graph, plan.random_reverse_order());
  graph.vertices[plan.objective()].w = had::Real(1.0);
  const auto accumulate = [&](std::uint32_t destination,
                              had::VertexId diagonal_vertex,
                              had::Real value) {
    if (destination == diagonal_destination)
      graph.selfSoEdges[diagonal_vertex] += value;
    else if (destination != no_destination)
      graph.sharedSoValues[destination] += value;
  };

  for (had::VertexId id : plan.random_reverse_order()) {
    had::ADVertex &vertex = graph.vertices[id];
    had::ADEdge &e1 = vertex.e1;
    had::ADEdge &e2 = vertex.e2;
    if (e1.to == id)
      continue;
    const bool e1_active = plan.random_active(e1.to);
    const bool e2_active = e2.to != id && plan.random_active(e2.to);
    const std::size_t begin = topology->offsets[id];
    const std::size_t end = topology->offsets[id + 1];
    for (std::size_t slot = begin; slot < end; ++slot) {
      const had::Real value = graph.sharedSoValues[slot];
      if (value == had::Real(0.0))
        continue;
      if (e1_active) {
        const auto destination = topology->random_destination_e1[slot];
        accumulate(destination, e1.to,
                   (destination == diagonal_destination ? had::Real(2.0)
                                                        : had::Real(1.0)) *
                       e1.w * value);
        if (operation_count)
          ++*operation_count;
      }
      if (e2_active) {
        const auto destination = topology->random_destination_e2[slot];
        accumulate(destination, e2.to,
                   (destination == diagonal_destination ? had::Real(2.0)
                                                        : had::Real(1.0)) *
                       e2.w * value);
        if (operation_count)
          ++*operation_count;
      }
    }

    const had::Real diagonal = graph.selfSoEdges[id];
    if (diagonal != had::Real(0.0)) {
      if (e1_active)
        graph.selfSoEdges[e1.to] += e1.w * e1.w * diagonal;
      if (e2_active) {
        graph.selfSoEdges[e2.to] += e2.w * e2.w * diagonal;
        if (e1_active) {
          if (e1.to == e2.to)
            graph.selfSoEdges[e1.to] += had::Real(2.0) * e1.w * e2.w * diagonal;
          else
            AddOffDiagonalHessian(graph, e1.to, e2.to,
                                  e1.w * e2.w * diagonal);
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
          AddOffDiagonalHessian(graph, e1.to, e2.to,
                                adjoint * vertex.soW);
        }
      }
      vertex.w = had::Real(0.0);
      if (e1_active)
        graph.vertices[e1.to].w += adjoint * e1.w;
      if (e2_active)
        graph.vertices[e2.to].w += adjoint * e2.w;
    }
  }
  return true;
}

inline void PropagateRandomHessianRestricted(
    had::ADGraph &graph, const LaplaceGraphPlan &plan,
    std::size_t *operation_count = nullptr) {
  if (PropagateRandomHessianFrozen(graph, plan, operation_count))
    return;
  PropagateHessianRestricted(
      graph, plan, plan.random_reverse_order(),
      [&plan](had::VertexId id) { return plan.random_active(id); },
      operation_count);
}

inline void
PropagateRandomHessianDirectionalRestricted(had::ADGraph &graph,
                                            const LaplaceGraphPlan &plan) {
  had::g_ADGraph = &graph;
  // The scalar directional fallback mutates two independent tree families.
  // Keep its established implementation by detaching this graph from shared
  // ordinary topology if the bounded direct trace ever falls back.
  graph.ThawSharedHessianTopology();
  graph.EnsureScalarDirectionalStorage();
  if (graph.soEdges.size() < graph.vertices.size()) {
    graph.soEdges.resize(graph.vertices.size());
  }
  if (graph.soEdgesDot.size() < graph.vertices.size()) {
    graph.soEdgesDot.resize(graph.vertices.size());
  }
  if (graph.selfSoEdges.size() < graph.vertices.size()) {
    graph.selfSoEdges.resize(graph.vertices.size(), had::Real(0.0));
  }
  if (graph.selfSoEdgesDot.size() < graph.vertices.size()) {
    graph.selfSoEdgesDot.resize(graph.vertices.size(), had::Real(0.0));
  }
  for (had::VertexId id : plan.random_reverse_order()) {
    graph.soEdges[id].Clear();
    graph.soEdgesDot[id].Clear();
    graph.selfSoEdges[id] = had::Real(0.0);
    graph.selfSoEdgesDot[id] = had::Real(0.0);
  }

  const auto active = [&plan](had::VertexId id) {
    return plan.random_active(id);
  };
  for (had::VertexId id : plan.random_reverse_order()) {
    had::ADVertex &vertex = graph.vertices[id];
    had::ADEdge &e1 = vertex.e1;
    had::ADEdge &e2 = vertex.e2;
    had::ADScalarDirectionalVertex &directional = graph.scalarDirectional[id];
    if (e1.to == id)
      continue;
    const bool e1_active = active(e1.to);
    const bool e2_active = e2.to != id && active(e2.to);

    auto &tree = graph.soEdges[id];
    auto &tree_dot = graph.soEdgesDot[id];
    for (const auto &node : tree.nodes) {
      if (!active(node.key))
        continue;
      const had::ADEdge second(node.key, node.val);
      const had::Real second_dot = tree_dot.Query(node.key);
      if (e1_active) {
        had::PushEdge(e1, second);
        had::PushEdgeDot(e1, directional.e1Dw, second, second_dot);
      }
      if (e2_active) {
        had::PushEdge(e2, second);
        had::PushEdgeDot(e2, directional.e2Dw, second, second_dot);
      }
    }

    const had::Real diagonal = graph.selfSoEdges[id];
    const had::Real diagonal_dot = graph.selfSoEdgesDot[id];
    if (diagonal != had::Real(0.0) || diagonal_dot != had::Real(0.0)) {
      if (e1_active) {
        graph.selfSoEdges[e1.to] += e1.w * e1.w * diagonal;
        graph.selfSoEdgesDot[e1.to] +=
            had::Real(2.0) * e1.w * directional.e1Dw * diagonal +
            e1.w * e1.w * diagonal_dot;
      }
      if (e2_active) {
        graph.selfSoEdges[e2.to] += e2.w * e2.w * diagonal;
        graph.selfSoEdgesDot[e2.to] +=
            had::Real(2.0) * e2.w * directional.e2Dw * diagonal +
            e2.w * e2.w * diagonal_dot;
        if (e1_active) {
          const had::Real cross = e1.w * e2.w * diagonal;
          const had::Real cross_dot =
              (directional.e1Dw * e2.w + e1.w * directional.e2Dw) * diagonal +
              e1.w * e2.w * diagonal_dot;
          if (e1.to == e2.to) {
            graph.selfSoEdges[e1.to] += had::Real(2.0) * cross;
            graph.selfSoEdgesDot[e1.to] += had::Real(2.0) * cross_dot;
          } else {
            graph.soEdges[std::max(e1.to, e2.to)].Insert(std::min(e1.to, e2.to),
                                                         cross);
            graph.soEdgesDot[std::max(e1.to, e2.to)].Insert(
                std::min(e1.to, e2.to), cross_dot);
          }
        }
      }
    }

    const had::Real adjoint = vertex.w;
    const had::Real adjoint_dot = directional.wDot;
    if ((adjoint != had::Real(0.0) || adjoint_dot != had::Real(0.0)) &&
        (vertex.soW != had::Real(0.0) ||
         directional.soWDot != had::Real(0.0))) {
      const had::Real create = adjoint * vertex.soW;
      const had::Real create_dot =
          adjoint_dot * vertex.soW + adjoint * directional.soWDot;
      if (e2.to == id) {
        if (e1_active) {
          graph.selfSoEdges[e1.to] += create;
          graph.selfSoEdgesDot[e1.to] += create_dot;
        }
      } else if (e1.to == e2.to) {
        if (e1_active) {
          graph.selfSoEdges[e1.to] += had::Real(2.0) * create;
          graph.selfSoEdgesDot[e1.to] += had::Real(2.0) * create_dot;
        }
      } else if (e1_active && e2_active) {
        graph.soEdges[std::max(e1.to, e2.to)].Insert(std::min(e1.to, e2.to),
                                                     create);
        graph.soEdgesDot[std::max(e1.to, e2.to)].Insert(std::min(e1.to, e2.to),
                                                        create_dot);
      }
    }

    if (adjoint != had::Real(0.0) || adjoint_dot != had::Real(0.0)) {
      vertex.w = had::Real(0.0);
      directional.wDot = had::Real(0.0);
      if (e1_active) {
        graph.vertices[e1.to].w += adjoint * e1.w;
        graph.scalarDirectional[e1.to].wDot +=
            adjoint_dot * e1.w + adjoint * directional.e1Dw;
      }
      if (e2_active) {
        graph.vertices[e2.to].w += adjoint * e2.w;
        graph.scalarDirectional[e2.to].wDot +=
            adjoint_dot * e2.w + adjoint * directional.e2Dw;
      }
    }
  }
}

// Exact second-order propagation over objective ancestors that depend on
// either partition. This retains the mixed random/fixed edges required for
// H_{u,theta}, while pruning constants and report-only graph regions.
inline void PropagateLaplaceHessianRestricted(had::ADGraph &graph,
                                              const LaplaceGraphPlan &plan) {
  PropagateHessianRestricted(
      graph, plan, plan.laplace_reverse_order(),
      [&plan](had::VertexId id) { return plan.laplace_active(id); });
}

inline bool PropagateMixedHessianFrozen(had::ADGraph &graph,
                                        const LaplaceGraphPlan &plan) {
  if (!graph.sharedSoTopology)
    return false;
  constexpr std::uint32_t no_destination =
      std::numeric_limits<std::uint32_t>::max();
  constexpr std::uint32_t diagonal_destination = no_destination - 1U;
  auto topology = graph.sharedSoTopology;
  try {
    std::call_once(topology->mixed_destination_once, [&]() {
      topology->mixed_destination_e1.assign(topology->nodes.size(),
                                            no_destination);
      topology->mixed_destination_e2.assign(topology->nodes.size(),
                                            no_destination);
      const auto keep_edge = [&plan](had::VertexId a, had::VertexId b) {
        return plan.laplace_active(a) && plan.laplace_active(b) &&
               (plan.random_active(a) || plan.random_active(b));
      };
      const auto set_destination = [&](std::vector<std::uint32_t> &destinations,
                                       std::size_t source_slot,
                                       had::VertexId target,
                                       had::VertexId key) {
        if (!plan.laplace_active(target) || !keep_edge(target, key))
          return;
        if (target == key) {
          destinations[source_slot] = diagonal_destination;
          return;
        }
        const std::size_t destination = graph.FindSharedSoSlot(
            std::max(target, key), std::min(target, key));
        if (destination >= diagonal_destination)
          throw std::out_of_range("frozen mixed-Hessian destination is absent");
        destinations[source_slot] = static_cast<std::uint32_t>(destination);
      };

      for (had::VertexId id : plan.laplace_reverse_order()) {
        const had::ADVertex &vertex = graph.vertices[id];
        if (vertex.e1.to == id)
          continue;
        const std::size_t begin = topology->offsets[id];
        const std::size_t end = topology->offsets[id + 1];
        for (std::size_t slot = begin; slot < end; ++slot) {
          const had::VertexId key = topology->nodes[slot].key;
          set_destination(topology->mixed_destination_e1, slot, vertex.e1.to,
                          key);
          if (vertex.e2.to != id)
            set_destination(topology->mixed_destination_e2, slot, vertex.e2.to,
                            key);
        }
      }
    });
  } catch (const std::out_of_range &) {
    // Preserve the general copy-on-write behavior if an unforeseen
    // parameter-dependent edge was not present when topology was frozen.
    graph.ThawSharedHessianTopology();
    return false;
  }

  had::g_ADGraph = &graph;
  ResetHessianSweepPreserveStorage(graph, plan.laplace_reverse_order());
  graph.vertices[plan.objective()].w = had::Real(1.0);

  const auto active = [&plan](had::VertexId id) {
    return plan.laplace_active(id);
  };
  const auto keep_edge = [&plan](had::VertexId a, had::VertexId b) {
    return plan.laplace_active(a) && plan.laplace_active(b) &&
           (plan.random_active(a) || plan.random_active(b));
  };
  const auto accumulate_destination = [&](std::uint32_t destination,
                                          had::VertexId diagonal_vertex,
                                          had::Real value) {
    if (destination == diagonal_destination)
      graph.selfSoEdges[diagonal_vertex] += value;
    else if (destination != no_destination)
      graph.sharedSoValues[destination] += value;
  };

  for (had::VertexId id : plan.laplace_reverse_order()) {
    had::ADVertex &vertex = graph.vertices[id];
    had::ADEdge &e1 = vertex.e1;
    had::ADEdge &e2 = vertex.e2;
    if (e1.to == id)
      continue;
    const bool e1_active = active(e1.to);
    const bool e2_active = e2.to != id && active(e2.to);
    const std::size_t begin = topology->offsets[id];
    const std::size_t end = topology->offsets[id + 1];
    for (std::size_t slot = begin; slot < end; ++slot) {
      const had::Real value = graph.sharedSoValues[slot];
      if (value == had::Real(0.0))
        continue;
      if (e1_active)
        accumulate_destination(
            topology->mixed_destination_e1[slot], e1.to,
            (topology->mixed_destination_e1[slot] == diagonal_destination
                 ? had::Real(2.0)
                 : had::Real(1.0)) *
                e1.w * value);
      if (e2_active)
        accumulate_destination(
            topology->mixed_destination_e2[slot], e2.to,
            (topology->mixed_destination_e2[slot] == diagonal_destination
                 ? had::Real(2.0)
                 : had::Real(1.0)) *
                e2.w * value);
    }

    const had::Real diagonal = graph.selfSoEdges[id];
    if (diagonal != had::Real(0.0)) {
      if (e1_active && plan.random_active(e1.to))
        graph.selfSoEdges[e1.to] += e1.w * e1.w * diagonal;
      if (e2_active) {
        if (plan.random_active(e2.to))
          graph.selfSoEdges[e2.to] += e2.w * e2.w * diagonal;
        if (e1_active && keep_edge(e1.to, e2.to)) {
          if (e1.to == e2.to)
            graph.selfSoEdges[e1.to] += had::Real(2.0) * e1.w * e2.w * diagonal;
          else
            AddOffDiagonalHessian(graph, e1.to, e2.to, e1.w * e2.w * diagonal);
        }
      }
    }

    const had::Real adjoint = vertex.w;
    if (adjoint != had::Real(0.0)) {
      if (vertex.soW != had::Real(0.0)) {
        const had::Real create = adjoint * vertex.soW;
        if (e2.to == id) {
          if (e1_active && plan.random_active(e1.to))
            graph.selfSoEdges[e1.to] += create;
        } else if (e1.to == e2.to) {
          if (e1_active && plan.random_active(e1.to))
            graph.selfSoEdges[e1.to] += had::Real(2.0) * create;
        } else if (e1_active && e2_active && keep_edge(e1.to, e2.to)) {
          AddOffDiagonalHessian(graph, e1.to, e2.to, create);
        }
      }
      vertex.w = had::Real(0.0);
      if (e1_active)
        graph.vertices[e1.to].w += adjoint * e1.w;
      if (e2_active)
        graph.vertices[e2.to].w += adjoint * e2.w;
    }
  }
  return true;
}

// Exact propagation for H_{u,theta}. Fixed-fixed curvature can never
// contribute to a mixed entry, so discard it while retaining random-random
// edges needed while intermediates are pushed toward the inputs.
inline void PropagateMixedHessianRestricted(had::ADGraph &graph,
                                            const LaplaceGraphPlan &plan) {
  if (PropagateMixedHessianFrozen(graph, plan))
    return;
  had::g_ADGraph = &graph;
  ResetHessianSweepPreserveStorage(graph, plan.laplace_reverse_order());
  graph.vertices[plan.objective()].w = had::Real(1.0);

  const auto active = [&plan](had::VertexId id) {
    return plan.laplace_active(id);
  };
  const auto keep_edge = [&plan](had::VertexId a, had::VertexId b) {
    return plan.laplace_active(a) && plan.laplace_active(b) &&
           (plan.random_active(a) || plan.random_active(b));
  };

  for (had::VertexId id : plan.laplace_reverse_order()) {
    had::ADVertex &vertex = graph.vertices[id];
    had::ADEdge &e1 = vertex.e1;
    had::ADEdge &e2 = vertex.e2;
    if (e1.to == id)
      continue;
    const bool e1_active = active(e1.to);
    const bool e2_active = e2.to != id && active(e2.to);

    ForEachOffDiagonalHessian(
        graph, id, [&](had::VertexId key, had::Real value) {
          const had::ADEdge second(key, value);
          if (e1_active && keep_edge(e1.to, key)) {
            if (e1.to == key)
              graph.selfSoEdges[e1.to] += had::Real(2.0) * e1.w * value;
            else
              AddOffDiagonalHessian(graph, e1.to, key, e1.w * value);
          }
          if (e2_active && keep_edge(e2.to, key)) {
            if (e2.to == key)
              graph.selfSoEdges[e2.to] += had::Real(2.0) * e2.w * value;
            else
              AddOffDiagonalHessian(graph, e2.to, key, e2.w * value);
          }
        });

    const had::Real diagonal = graph.selfSoEdges[id];
    if (diagonal != had::Real(0.0)) {
      if (e1_active && plan.random_active(e1.to))
        graph.selfSoEdges[e1.to] += e1.w * e1.w * diagonal;
      if (e2_active) {
        if (plan.random_active(e2.to))
          graph.selfSoEdges[e2.to] += e2.w * e2.w * diagonal;
        if (e1_active && keep_edge(e1.to, e2.to)) {
          if (e1.to == e2.to)
            graph.selfSoEdges[e1.to] += had::Real(2.0) * e1.w * e2.w * diagonal;
          else
            AddOffDiagonalHessian(graph, e1.to, e2.to, e1.w * e2.w * diagonal);
        }
      }
    }

    const had::Real adjoint = vertex.w;
    if (adjoint != had::Real(0.0)) {
      if (vertex.soW != had::Real(0.0)) {
        const had::Real create = adjoint * vertex.soW;
        if (e2.to == id) {
          if (e1_active && plan.random_active(e1.to))
            graph.selfSoEdges[e1.to] += create;
        } else if (e1.to == e2.to) {
          if (e1_active && plan.random_active(e1.to))
            graph.selfSoEdges[e1.to] += had::Real(2.0) * create;
        } else if (e1_active && e2_active && keep_edge(e1.to, e2.to)) {
          AddOffDiagonalHessian(graph, e1.to, e2.to, create);
        }
      }
      vertex.w = had::Real(0.0);
      if (e1_active)
        graph.vertices[e1.to].w += adjoint * e1.w;
      if (e2_active)
        graph.vertices[e2.to].w += adjoint * e2.w;
    }
  }
}

} // namespace quadra
