#ifndef QUADRA_HAD_INTERMEDIATE_EDGE_SLOT_REGISTRY_HPP
#define QUADRA_HAD_INTERMEDIATE_EDGE_SLOT_REGISTRY_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace had {

class IntermediateEdgeSlotRegistry {
public:
  using VertexId = std::uint32_t;

  struct Edge {
    VertexId outer = 0;
    VertexId inner = 0;
    std::size_t slot = 0;
  };

  void Clear() {
    slots_.clear();
    edges_.clear();
  }

  std::size_t size() const { return edges_.size(); }

  // The unordered-map node allocation is implementation-defined. This
  // estimate covers its bucket array and stored key/value payload, plus the
  // exact reserved edge-vector payload.
  std::size_t EstimatedReservedBytes() const {
    return slots_.bucket_count() * sizeof(void *) +
           slots_.size() * sizeof(std::pair<const std::uint64_t, std::size_t>) +
           edges_.capacity() * sizeof(Edge);
  }

  const std::vector<Edge> &edges() const { return edges_; }

  std::size_t GetOrCreate(VertexId a, VertexId b) {
    const VertexId outer = std::max(a, b);
    const VertexId inner = std::min(a, b);
    const std::uint64_t key = Pack(outer, inner);

    const auto found = slots_.find(key);
    if (found != slots_.end()) {
      return found->second;
    }

    const std::size_t slot = edges_.size();
    slots_.emplace(key, slot);
    edges_.push_back(Edge{outer, inner, slot});
    return slot;
  }

  bool TryGet(VertexId a, VertexId b, std::size_t &slot_out) const {
    const VertexId outer = std::max(a, b);
    const VertexId inner = std::min(a, b);
    const std::uint64_t key = Pack(outer, inner);

    const auto found = slots_.find(key);
    if (found == slots_.end()) {
      return false;
    }

    slot_out = found->second;
    return true;
  }

private:
  static std::uint64_t Pack(VertexId outer, VertexId inner) {
    return (static_cast<std::uint64_t>(outer) << 32) |
           static_cast<std::uint64_t>(inner);
  }

  std::unordered_map<std::uint64_t, std::size_t> slots_;
  std::vector<Edge> edges_;
};

} // namespace had

#endif // QUADRA_HAD_INTERMEDIATE_EDGE_SLOT_REGISTRY_HPP
