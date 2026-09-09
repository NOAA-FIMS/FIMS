#pragma once

#include <Eigen/Dense>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <vector>

#include "../autodiff/laplace_graph_plan.hpp"
#include "../had_quadra.hpp"
#include "had_quadra_sparse_exact_hdot_provider.hpp"

namespace quadra {
namespace laplace {

struct ReverseHessianTraceResult {
  Eigen::VectorXd input_gradient;
  std::size_t operation_count = 0;
  std::size_t operation_bytes = 0;
};

namespace detail {

enum class TraceReverseRecordKind : std::uint8_t {
  HessianTreeE1,
  HessianTreeE2,
  HessianDiagonalE1,
  HessianDiagonalE2,
  HessianDiagonalCross,
  HessianSecondUnary,
  HessianSecondSame,
  HessianSecondBinary,
  FirstAdjointE1,
  FirstAdjointE2
};

struct TraceReverseRecord {
  double payload = 0.0;
  had::VertexId vertex = 0;
  std::uint32_t packed_auxiliary_kind = 0;

  static constexpr std::uint32_t kAuxiliaryBits = 28U;
  static constexpr std::uint32_t kAuxiliaryMask =
      (std::uint32_t(1) << kAuxiliaryBits) - 1U;

  void set(TraceReverseRecordKind kind, had::VertexId auxiliary) {
    if (auxiliary > kAuxiliaryMask)
      throw std::length_error(
          "reverse Hessian trace auxiliary vertex exceeds 28-bit limit");
    packed_auxiliary_kind =
        (static_cast<std::uint32_t>(kind) << kAuxiliaryBits) |
        static_cast<std::uint32_t>(auxiliary);
  }

  TraceReverseRecordKind kind() const {
    return static_cast<TraceReverseRecordKind>(packed_auxiliary_kind >>
                                               kAuxiliaryBits);
  }

  had::VertexId auxiliary() const {
    return static_cast<had::VertexId>(packed_auxiliary_kind & kAuxiliaryMask);
  }
};

static_assert(sizeof(TraceReverseRecord) == 16,
              "trace reverse record unexpectedly grew");

inline void add_hessian_value(had::ADGraph &graph, had::VertexId a,
                              had::VertexId b, double value) {
  if (a == b)
    graph.selfSoEdges[a] += value;
  else
    graph.InsertSoEdge(std::max(a, b), std::min(a, b), value);
}

inline double hessian_value(const had::ADGraph &graph, had::VertexId a,
                            had::VertexId b) {
  return a == b ? graph.selfSoEdges[a]
                : graph.QuerySoEdge(std::max(a, b), std::min(a, b));
}

inline void zero_hessian_values_preserve_topology(had::ADGraph &graph) {
  graph.ZeroAllSoEdgeValues();
  std::fill(graph.selfSoEdges.begin(), graph.selfSoEdges.end(), 0.0);
}

inline void log_trace_record(std::vector<TraceReverseRecord> &records,
                             TraceReverseRecordKind kind, had::VertexId vertex,
                             double payload, had::VertexId auxiliary = 0) {
  TraceReverseRecord record;
  record.vertex = vertex;
  record.payload = payload;
  record.set(kind, auxiliary);
  records.push_back(record);
}

} // namespace detail

// Reverse differentiate the random-restricted edge-pushing Hessian sweep.
//
// selected_inverse(row, col) returns H_uu^{-1} in random-effect coordinates.
// The returned input gradient is ordered [fixed, random].
template <class SelectedInverseAccessor>
ReverseHessianTraceResult reverse_hessian_trace_contraction(
    had::ADGraph &graph, const LaplaceGraphPlan &plan,
    const std::vector<had::AReal> &fixed, const std::vector<had::AReal> &random,
    const RandomHessianPattern &pattern,
    SelectedInverseAccessor selected_inverse,
    std::size_t memory_cap_bytes = 64U * 1024U * 1024U) {
  had::g_ADGraph = &graph;
  ResetHessianSweepPreserveStorage(graph, plan.random_reverse_order());
  graph.vertices[plan.objective()].w = had::Real(1.0);

  std::vector<detail::TraceReverseRecord> records;
  std::size_t record_upper_bound = 0;
  for (had::VertexId id : plan.random_reverse_order()) {
    const had::ADVertex &vertex = graph.vertices[id];
    if (vertex.e1.to != id)
      record_upper_bound += 2U * graph.SoEdgeCount(id) + 6U;
  }
  const std::size_t record_capacity =
      std::min(memory_cap_bytes / sizeof(detail::TraceReverseRecord),
               record_upper_bound);
  records.reserve(record_capacity);
  const auto check_cap = [&]() {
    if (records.size() * sizeof(detail::TraceReverseRecord) > memory_cap_bytes)
      throw std::length_error(
          "reverse Hessian trace operation log exceeded memory cap");
  };
  const auto active = [&plan](had::VertexId id) {
    return plan.random_active(id);
  };
  constexpr std::uint32_t no_destination =
      std::numeric_limits<std::uint32_t>::max();
  constexpr std::uint32_t diagonal_destination = no_destination - 1U;
  auto topology = graph.sharedSoTopology;
  bool frozen_direct = static_cast<bool>(topology);
  if (frozen_direct) {
    try {
      std::call_once(topology->trace_destination_once, [&]() {
        topology->trace_destination_e1.assign(topology->nodes.size(),
                                              no_destination);
        topology->trace_destination_e2.assign(topology->nodes.size(),
                                              no_destination);
        const auto set_destination =
            [&](std::vector<std::uint32_t> &destinations,
                std::size_t source_slot, had::VertexId target,
                had::VertexId key) {
              if (!active(target) || !active(key))
                return;
              if (target == key) {
                destinations[source_slot] = diagonal_destination;
                return;
              }
              const std::size_t destination = graph.FindSharedSoSlot(
                  std::max(target, key), std::min(target, key));
              if (destination >= diagonal_destination)
                throw std::out_of_range(
                    "frozen trace-Hessian destination is absent");
              destinations[source_slot] =
                  static_cast<std::uint32_t>(destination);
            };
        for (had::VertexId id : plan.random_reverse_order()) {
          const had::ADVertex &vertex = graph.vertices[id];
          if (vertex.e1.to == id)
            continue;
          const std::size_t begin = topology->offsets[id];
          const std::size_t end = topology->offsets[id + 1];
          for (std::size_t slot = begin; slot < end; ++slot) {
            const had::VertexId key = topology->nodes[slot].key;
            set_destination(topology->trace_destination_e1, slot, vertex.e1.to,
                            key);
            if (vertex.e2.to != id)
              set_destination(topology->trace_destination_e2, slot,
                              vertex.e2.to, key);
          }
        }
      });
    } catch (const std::out_of_range &) {
      graph.ThawSharedHessianTopology();
      topology.reset();
      frozen_direct = false;
    }
  }
  const auto accumulate_destination = [&](std::uint32_t destination,
                                          had::VertexId diagonal_vertex,
                                          double value) {
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
    const bool e1_active = active(e1.to);
    const bool e2_active = e2.to != id && active(e2.to);

    if (frozen_direct) {
      const std::size_t begin = topology->offsets[id];
      const std::size_t end = topology->offsets[id + 1];
      for (std::size_t slot = begin; slot < end; ++slot) {
        const had::Real source = graph.sharedSoValues[slot];
        if (e1_active) {
          const std::uint32_t destination =
              topology->trace_destination_e1[slot];
          const double symmetry =
              destination == diagonal_destination ? 2.0 : 1.0;
          accumulate_destination(destination, e1.to, symmetry * e1.w * source);
          detail::log_trace_record(
              records, detail::TraceReverseRecordKind::HessianTreeE1, id,
              source, static_cast<had::VertexId>(slot));
        }
        if (e2_active) {
          const std::uint32_t destination =
              topology->trace_destination_e2[slot];
          const double symmetry =
              destination == diagonal_destination ? 2.0 : 1.0;
          accumulate_destination(destination, e2.to, symmetry * e2.w * source);
          detail::log_trace_record(
              records, detail::TraceReverseRecordKind::HessianTreeE2, id,
              source, static_cast<had::VertexId>(slot));
        }
      }
    } else {
      graph.ForEachSoEdge(id, [&](had::VertexId key, had::Real source) {
        if (e1_active) {
          const double symmetry = e1.to == key ? 2.0 : 1.0;
          detail::add_hessian_value(graph, e1.to, key,
                                    symmetry * e1.w * source);
          detail::log_trace_record(
              records, detail::TraceReverseRecordKind::HessianTreeE1, id,
              source, key);
        }
        if (e2_active) {
          const double symmetry = e2.to == key ? 2.0 : 1.0;
          detail::add_hessian_value(graph, e2.to, key,
                                    symmetry * e2.w * source);
          detail::log_trace_record(
              records, detail::TraceReverseRecordKind::HessianTreeE2, id,
              source, key);
        }
      });
    }

    const double diagonal = graph.selfSoEdges[id];
    if (diagonal != 0.0) {
      if (e1_active) {
        detail::add_hessian_value(graph, e1.to, e1.to, e1.w * e1.w * diagonal);
        detail::log_trace_record(
            records, detail::TraceReverseRecordKind::HessianDiagonalE1, id,
            diagonal);
      }
      if (e2_active) {
        detail::add_hessian_value(graph, e2.to, e2.to, e2.w * e2.w * diagonal);
        detail::log_trace_record(
            records, detail::TraceReverseRecordKind::HessianDiagonalE2, id,
            diagonal);
      }
      if (e1_active && e2_active) {
        const double symmetry = e1.to == e2.to ? 2.0 : 1.0;
        detail::add_hessian_value(graph, e1.to, e2.to,
                                  symmetry * e1.w * e2.w * diagonal);
        detail::log_trace_record(
            records, detail::TraceReverseRecordKind::HessianDiagonalCross, id,
            diagonal);
      }
    }

    const double adjoint = vertex.w;
    if (adjoint != 0.0 && vertex.soW != 0.0) {
      if (e2.to == id && e1_active) {
        detail::add_hessian_value(graph, e1.to, e1.to, adjoint * vertex.soW);
        detail::log_trace_record(
            records, detail::TraceReverseRecordKind::HessianSecondUnary, id,
            adjoint);
      } else if (e1.to == e2.to && e1_active) {
        detail::add_hessian_value(graph, e1.to, e1.to,
                                  2.0 * adjoint * vertex.soW);
        detail::log_trace_record(
            records, detail::TraceReverseRecordKind::HessianSecondSame, id,
            adjoint);
      } else if (e1_active && e2_active) {
        detail::add_hessian_value(graph, e1.to, e2.to, adjoint * vertex.soW);
        detail::log_trace_record(
            records, detail::TraceReverseRecordKind::HessianSecondBinary, id,
            adjoint);
      }
    }

    if (adjoint != 0.0) {
      vertex.w = 0.0;
      if (e1_active) {
        graph.vertices[e1.to].w += adjoint * e1.w;
        detail::log_trace_record(records,
                                 detail::TraceReverseRecordKind::FirstAdjointE1,
                                 id, adjoint);
      }
      if (e2_active) {
        graph.vertices[e2.to].w += adjoint * e2.w;
        detail::log_trace_record(records,
                                 detail::TraceReverseRecordKind::FirstAdjointE2,
                                 id, adjoint);
      }
    }
    check_cap();
  }

  // The completed forward sweep no longer needs its Hessian values. Reuse the
  // graph's stable sparse slots for reverse Hessian bars instead of allocating
  // a second hash table keyed by the same vertex pairs.
  detail::zero_hessian_values_preserve_topology(graph);
  for (const auto &entry : pattern) {
    const int a = entry.first;
    const int b = entry.second;
    const double inverse = selected_inverse(a, b);
    const double seed = a == b ? 0.5 * inverse : inverse;
    detail::add_hessian_value(graph, random[static_cast<size_t>(a)].varId,
                              random[static_cast<size_t>(b)].varId, seed);
  }

  std::vector<double> adjoint_bar(graph.vertices.size(), 0.0);
  std::vector<double> e1_bar(graph.vertices.size(), 0.0);
  std::vector<double> e2_bar(graph.vertices.size(), 0.0);
  std::vector<double> second_bar(graph.vertices.size(), 0.0);

  for (auto it = records.rbegin(); it != records.rend(); ++it) {
    const auto &record = *it;
    const had::VertexId id = record.vertex;
    const had::ADVertex &vertex = graph.vertices[id];
    const had::ADEdge &e1 = vertex.e1;
    const had::ADEdge &e2 = vertex.e2;
    double bar = 0.0;
    switch (record.kind()) {
    case detail::TraceReverseRecordKind::HessianTreeE1: {
      if (frozen_direct) {
        const std::size_t source_slot = record.auxiliary();
        const std::uint32_t destination =
            topology->trace_destination_e1[source_slot];
        const double symmetry = destination == diagonal_destination ? 2.0 : 1.0;
        if (destination == diagonal_destination)
          bar = graph.selfSoEdges[e1.to];
        else if (destination != no_destination)
          bar = graph.sharedSoValues[destination];
        if (bar != 0.0) {
          graph.sharedSoValues[source_slot] += symmetry * e1.w * bar;
          e1_bar[static_cast<size_t>(id)] += symmetry * record.payload * bar;
        }
      } else {
        const had::VertexId key = record.auxiliary();
        const double symmetry = e1.to == key ? 2.0 : 1.0;
        bar = detail::hessian_value(graph, e1.to, key);
        if (bar != 0.0) {
          detail::add_hessian_value(graph, id, key, symmetry * e1.w * bar);
          e1_bar[static_cast<size_t>(id)] += symmetry * record.payload * bar;
        }
      }
      break;
    }
    case detail::TraceReverseRecordKind::HessianTreeE2: {
      if (frozen_direct) {
        const std::size_t source_slot = record.auxiliary();
        const std::uint32_t destination =
            topology->trace_destination_e2[source_slot];
        const double symmetry = destination == diagonal_destination ? 2.0 : 1.0;
        if (destination == diagonal_destination)
          bar = graph.selfSoEdges[e2.to];
        else if (destination != no_destination)
          bar = graph.sharedSoValues[destination];
        if (bar != 0.0) {
          graph.sharedSoValues[source_slot] += symmetry * e2.w * bar;
          e2_bar[static_cast<size_t>(id)] += symmetry * record.payload * bar;
        }
      } else {
        const had::VertexId key = record.auxiliary();
        const double symmetry = e2.to == key ? 2.0 : 1.0;
        bar = detail::hessian_value(graph, e2.to, key);
        if (bar != 0.0) {
          detail::add_hessian_value(graph, id, key, symmetry * e2.w * bar);
          e2_bar[static_cast<size_t>(id)] += symmetry * record.payload * bar;
        }
      }
      break;
    }
    case detail::TraceReverseRecordKind::HessianDiagonalE1:
      bar = detail::hessian_value(graph, e1.to, e1.to);
      if (bar != 0.0) {
        detail::add_hessian_value(graph, id, id, e1.w * e1.w * bar);
        e1_bar[static_cast<size_t>(id)] += 2.0 * e1.w * record.payload * bar;
      }
      break;
    case detail::TraceReverseRecordKind::HessianDiagonalE2:
      bar = detail::hessian_value(graph, e2.to, e2.to);
      if (bar != 0.0) {
        detail::add_hessian_value(graph, id, id, e2.w * e2.w * bar);
        e2_bar[static_cast<size_t>(id)] += 2.0 * e2.w * record.payload * bar;
      }
      break;
    case detail::TraceReverseRecordKind::HessianDiagonalCross: {
      const double symmetry = e1.to == e2.to ? 2.0 : 1.0;
      bar = detail::hessian_value(graph, e1.to, e2.to);
      if (bar != 0.0) {
        detail::add_hessian_value(graph, id, id, symmetry * e1.w * e2.w * bar);
        e1_bar[static_cast<size_t>(id)] +=
            symmetry * e2.w * record.payload * bar;
        e2_bar[static_cast<size_t>(id)] +=
            symmetry * e1.w * record.payload * bar;
      }
      break;
    }
    case detail::TraceReverseRecordKind::HessianSecondUnary:
      bar = detail::hessian_value(graph, e1.to, e1.to);
      if (bar != 0.0) {
        adjoint_bar[static_cast<size_t>(id)] += vertex.soW * bar;
        second_bar[static_cast<size_t>(id)] += record.payload * bar;
      }
      break;
    case detail::TraceReverseRecordKind::HessianSecondSame:
      bar = detail::hessian_value(graph, e1.to, e1.to);
      if (bar != 0.0) {
        adjoint_bar[static_cast<size_t>(id)] += 2.0 * vertex.soW * bar;
        second_bar[static_cast<size_t>(id)] += 2.0 * record.payload * bar;
      }
      break;
    case detail::TraceReverseRecordKind::HessianSecondBinary:
      bar = detail::hessian_value(graph, e1.to, e2.to);
      if (bar != 0.0) {
        adjoint_bar[static_cast<size_t>(id)] += vertex.soW * bar;
        second_bar[static_cast<size_t>(id)] += record.payload * bar;
      }
      break;
    case detail::TraceReverseRecordKind::FirstAdjointE1:
      bar = adjoint_bar[static_cast<size_t>(e1.to)];
      adjoint_bar[static_cast<size_t>(id)] += e1.w * bar;
      e1_bar[static_cast<size_t>(id)] += record.payload * bar;
      break;
    case detail::TraceReverseRecordKind::FirstAdjointE2:
      bar = adjoint_bar[static_cast<size_t>(e2.to)];
      adjoint_bar[static_cast<size_t>(id)] += e2.w * bar;
      e2_bar[static_cast<size_t>(id)] += record.payload * bar;
      break;
    }
  }

  std::vector<double> primal_bar(graph.vertices.size(), 0.0);
  for (had::VertexId id = 1;
       id < static_cast<had::VertexId>(graph.vertices.size()); ++id) {
    const auto &vertex = graph.vertices[id];
    const auto &e1 = vertex.e1;
    const auto &e2 = vertex.e2;
    if (e1.to == id)
      continue;
    if (e2.to == id) {
      primal_bar[static_cast<size_t>(e1.to)] +=
          e1_bar[static_cast<size_t>(id)] * vertex.soW +
          second_bar[static_cast<size_t>(id)] * vertex.toW;
    } else if (vertex.op == had::OpCode::Multiply) {
      primal_bar[static_cast<size_t>(e2.to)] += e1_bar[static_cast<size_t>(id)];
      primal_bar[static_cast<size_t>(e1.to)] += e2_bar[static_cast<size_t>(id)];
    }
  }

  for (had::VertexId id = static_cast<had::VertexId>(graph.vertices.size() - 1);
       id > 0; --id) {
    const double bar = primal_bar[static_cast<size_t>(id)];
    if (bar == 0.0)
      continue;
    const auto &vertex = graph.vertices[id];
    if (vertex.e1.to != id)
      primal_bar[static_cast<size_t>(vertex.e1.to)] += bar * vertex.e1.w;
    if (vertex.e2.to != id)
      primal_bar[static_cast<size_t>(vertex.e2.to)] += bar * vertex.e2.w;
  }

  ReverseHessianTraceResult result;
  result.input_gradient.resize(
      static_cast<Eigen::Index>(fixed.size() + random.size()));
  for (size_t j = 0; j < fixed.size(); ++j)
    result.input_gradient[static_cast<Eigen::Index>(j)] =
        primal_bar[static_cast<size_t>(fixed[j].varId)];
  for (size_t i = 0; i < random.size(); ++i)
    result.input_gradient[static_cast<Eigen::Index>(fixed.size() + i)] =
        primal_bar[static_cast<size_t>(random[i].varId)];
  result.operation_count = records.size();
  result.operation_bytes = records.size() * sizeof(detail::TraceReverseRecord);
  return result;
}

} // namespace laplace
} // namespace quadra
