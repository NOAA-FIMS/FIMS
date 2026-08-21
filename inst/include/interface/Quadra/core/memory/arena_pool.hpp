#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace quadra {

class ArenaPool {
public:
  explicit ArenaPool(std::size_t initial_block_bytes = 1u << 20)
      : block_bytes_m(initial_block_bytes == 0 ? (1u << 20)
                                               : initial_block_bytes) {
    add_block(block_bytes_m);
  }

  ArenaPool(const ArenaPool &) = delete;
  ArenaPool &operator=(const ArenaPool &) = delete;

  ArenaPool(ArenaPool &&) noexcept = default;
  ArenaPool &operator=(ArenaPool &&) noexcept = default;

  ~ArenaPool() = default;

  void reset() noexcept {
    current_block_m = 0;
    offset_m = 0;
    bytes_used_m = 0;
  }

  void release() {
    blocks_m.clear();
    current_block_m = 0;
    offset_m = 0;
    bytes_used_m = 0;
    add_block(block_bytes_m);
  }

  std::size_t bytes_used() const noexcept { return bytes_used_m; }

  std::size_t bytes_reserved() const noexcept {
    std::size_t out = 0;
    for (const auto &block : blocks_m) {
      out += block.size;
    }
    return out;
  }

  std::size_t block_count() const noexcept { return blocks_m.size(); }

  void *allocate_bytes(std::size_t nbytes,
                       std::size_t alignment = alignof(std::max_align_t)) {
    if (nbytes == 0) {
      nbytes = 1;
    }

    if ((alignment & (alignment - 1)) != 0) {
      throw std::bad_alloc();
    }

    for (;;) {
      Block &block = blocks_m[current_block_m];

      std::uintptr_t base = reinterpret_cast<std::uintptr_t>(block.ptr.get());
      std::uintptr_t raw = base + offset_m;
      std::uintptr_t aligned =
          (raw + alignment - 1) & ~(static_cast<std::uintptr_t>(alignment) - 1);
      std::size_t new_offset =
          static_cast<std::size_t>(aligned - base) + nbytes;

      if (new_offset <= block.size) {
        offset_m = new_offset;
        bytes_used_m += nbytes;
        return reinterpret_cast<void *>(aligned);
      }

      if (current_block_m + 1 < blocks_m.size()) {
        ++current_block_m;
        offset_m = 0;
        continue;
      }

      const std::size_t next_size = std::max(block_bytes_m, nbytes + alignment);
      add_block(next_size);
      ++current_block_m;
      offset_m = 0;
    }
  }

  template <class T> T *allocate(std::size_t n = 1) {
    static_assert(!std::is_void<T>::value, "Cannot allocate void");
    return static_cast<T *>(allocate_bytes(sizeof(T) * n, alignof(T)));
  }

  template <class T, class... Args> T *create(Args &&...args) {
    T *ptr = allocate<T>(1);
    ::new (static_cast<void *>(ptr)) T(std::forward<Args>(args)...);
    return ptr;
  }

private:
  struct Block {
    std::unique_ptr<std::byte[]> ptr;
    std::size_t size = 0;
  };

  void add_block(std::size_t size) {
    Block block;
    block.ptr.reset(new std::byte[size]);
    block.size = size;
    blocks_m.push_back(std::move(block));
  }

  std::vector<Block> blocks_m;
  std::size_t block_bytes_m = 0;
  std::size_t current_block_m = 0;
  std::size_t offset_m = 0;
  std::size_t bytes_used_m = 0;
};

template <class T> class ArenaAllocator {
public:
  using value_type = T;

  explicit ArenaAllocator(ArenaPool *arena = nullptr) noexcept
      : arena_m(arena) {}

  template <class U>
  ArenaAllocator(const ArenaAllocator<U> &other) noexcept
      : arena_m(other.arena()) {}

  T *allocate(std::size_t n) {
    if (!arena_m) {
      throw std::bad_alloc();
    }
    return arena_m->template allocate<T>(n);
  }

  void deallocate(T *, std::size_t) noexcept {
    // No-op. ArenaPool::reset() releases allocations in bulk.
  }

  ArenaPool *arena() const noexcept { return arena_m; }

  template <class U>
  bool operator==(const ArenaAllocator<U> &other) const noexcept {
    return arena_m == other.arena();
  }

  template <class U>
  bool operator!=(const ArenaAllocator<U> &other) const noexcept {
    return !(*this == other);
  }

private:
  template <class U> friend class ArenaAllocator;

  ArenaPool *arena_m = nullptr;
};

} // namespace quadra
