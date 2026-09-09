#pragma once

#include "../memory/arena_pool.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace quadra {

class HadQuadraWorkspace {
public:
  explicit HadQuadraWorkspace(std::size_t block_bytes = 1u << 20)
      : arena_m(block_bytes) {}

  HadQuadraWorkspace(const HadQuadraWorkspace &) = delete;
  HadQuadraWorkspace &operator=(const HadQuadraWorkspace &) = delete;

  void reset() noexcept { arena_m.reset(); }
  void release() { arena_m.release(); }

  ArenaPool &arena() noexcept { return arena_m; }
  const ArenaPool &arena() const noexcept { return arena_m; }

  std::size_t bytes_used() const noexcept { return arena_m.bytes_used(); }
  std::size_t bytes_reserved() const noexcept {
    return arena_m.bytes_reserved();
  }
  std::size_t block_count() const noexcept { return arena_m.block_count(); }

  template <class T> T *allocate(std::size_t n = 1) {
    return arena_m.template allocate<T>(n);
  }

  template <class T, class... Args> T *create(Args &&...args) {
    return arena_m.template create<T>(std::forward<Args>(args)...);
  }

  template <class T> using Vector = std::vector<T, ArenaAllocator<T>>;

  template <class T> Vector<T> make_vector() {
    return Vector<T>{ArenaAllocator<T>(&arena_m)};
  }

  template <class T> Vector<T> make_vector_with_reserve(std::size_t n) {
    Vector<T> out{ArenaAllocator<T>(&arena_m)};
    out.reserve(n);
    return out;
  }

private:
  ArenaPool arena_m;
};

} // namespace quadra
