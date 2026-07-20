#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "had_quadra.hpp"

namespace quadra {

// Owns and reuses a HAD graph.
//
// This is the production-facing seam for graph reuse. It intentionally keeps
// the first API small and explicit:
//
//   HadGraphWorkspace ws;
//   auto y = ws.Build([&]() { return model(theta, u); });
//   ws.PropagateAdjoint(y);
//
// Later layers can build exact-gradient/Laplace workspaces on top of this
// without directly managing had::g_ADGraph.
class HadGraphWorkspace {
public:
  HadGraphWorkspace() = default;

  ~HadGraphWorkspace() {
    if (had::g_ADGraph == &graph_) had::g_ADGraph = nullptr;
  }

  HadGraphWorkspace(const HadGraphWorkspace &) = delete;
  HadGraphWorkspace &operator=(const HadGraphWorkspace &) = delete;

  // Moving would change graph_'s address while the thread-local active graph
  // pointer may still refer to the source object.
  HadGraphWorkspace(HadGraphWorkspace &&) = delete;
  HadGraphWorkspace &operator=(HadGraphWorkspace &&) = delete;

  had::ADGraph &Graph() { return graph_; }
  const had::ADGraph &Graph() const { return graph_; }

  void Activate() { had::g_ADGraph = &graph_; }

  bool IsActive() const { return had::g_ADGraph == &graph_; }

  std::size_t VertexCount() const { return graph_.vertices.size(); }

  void Clear() {
    graph_.Clear();
    Activate();
    built_ = false;
    output_var_id_ = 0;
  }

  template <class Builder> had::AReal Build(Builder &&builder) {
    Clear();
    Activate();

    had::AReal output = std::forward<Builder>(builder)();

    built_ = true;
    output_var_id_ = output.varId;

    return output;
  }

  void ResetAdjoints() {
    Activate();

    std::fill(graph_.vertexWDotBatch.begin(), graph_.vertexWDotBatch.end(),
              had::Real(0.0));
    std::fill(graph_.vertexSoWDotBatch.begin(),
              graph_.vertexSoWDotBatch.end(), had::Real(0.0));
    std::fill(graph_.vertexDotBatch.begin(), graph_.vertexDotBatch.end(),
              had::Real(0.0));
    std::fill(graph_.edge1DwBatch.begin(), graph_.edge1DwBatch.end(),
              had::Real(0.0));
    std::fill(graph_.edge2DwBatch.begin(), graph_.edge2DwBatch.end(),
              had::Real(0.0));

    for (auto &vertex : graph_.vertices) {
      vertex.w = had::Real(0.0);
      vertex.wDot = had::Real(0.0);
      vertex.soWDot = had::Real(0.0);
      vertex.dot = had::Real(0.0);

    }

    if (graph_.soEdges.size() < graph_.vertices.size()) {
      graph_.soEdges.resize(graph_.vertices.size());
    } else {
      for (auto &tree : graph_.soEdges) {
        tree.Clear();
      }
    }

    if (graph_.soEdgesDot.size() < graph_.vertices.size()) {
      graph_.soEdgesDot.resize(graph_.vertices.size());
    } else {
      for (auto &tree : graph_.soEdgesDot) {
        tree.Clear();
      }
    }

    graph_.selfSoEdges.assign(graph_.vertices.size(), had::Real(0.0));
    graph_.selfSoEdgesDot.assign(graph_.vertices.size(), had::Real(0.0));
  }

  void PropagateAdjoint() {
    if (!built_) {
      throw std::logic_error(
          "HadGraphWorkspace::PropagateAdjoint called before Build.");
    }

    PropagateAdjoint(output_var_id_);
  }

  void PropagateAdjoint(had::VertexId output_var_id) {
    Activate();
    ResetAdjoints();

    if (output_var_id >= graph_.vertices.size()) {
      throw std::out_of_range(
          "HadGraphWorkspace::PropagateAdjoint output_var_id out of range.");
    }

    graph_.vertices[output_var_id].w = had::Real(1.0);
    had::PropagateAdjoint();
  }

  void ResizeDirectionalBatch(std::size_t n_directions) {
    Activate();
    had::ResizeDirectionalBatch(static_cast<int>(n_directions));
  }

  void PropagateAdjointDirectionalBatch() {
    if (!built_) {
      throw std::logic_error(
          "HadGraphWorkspace::PropagateAdjointDirectionalBatch called before "
          "Build.");
    }

    Activate();

    if (output_var_id_ >= graph_.vertices.size()) {
      throw std::out_of_range(
          "HadGraphWorkspace::PropagateAdjointDirectionalBatch output_var_id "
          "out of range.");
    }

    // PropagateAdjoint() consumes/clears reverse adjoints while building
    // the base Hessian. The directional reverse sweep needs the output
    // adjoint seed restored.
    graph_.vertices[output_var_id_].w = had::Real(1.0);

    had::PropagateAdjointDirectionalBatch();
  }

  had::VertexId OutputVarId() const {
    if (!built_) {
      throw std::logic_error(
          "HadGraphWorkspace::OutputVarId called before Build.");
    }
    return output_var_id_;
  }

private:
  had::ADGraph graph_;
  bool built_ = false;
  had::VertexId output_var_id_ = 0;
};

} // namespace quadra
