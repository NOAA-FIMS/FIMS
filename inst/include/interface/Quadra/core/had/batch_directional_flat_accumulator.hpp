#ifndef QUADRA_HAD_BATCH_DIRECTIONAL_FLAT_ACCUMULATOR_HPP
#define QUADRA_HAD_BATCH_DIRECTIONAL_FLAT_ACCUMULATOR_HPP

#include <Eigen/Dense>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace had {

// A compact K x n_slots directional accumulator.
//
// Intended use:
//   - slot ids are precomputed from the known sparse pattern
//   - each direction k has contiguous storage
//   - clear is a single fill over K*n_slots
//
// This avoids tree query/insert/balance costs in batched Hdot propagation.
class BatchDirectionalFlatAccumulator {
public:
  BatchDirectionalFlatAccumulator() = default;

  BatchDirectionalFlatAccumulator(std::size_t n_directions,
                                  std::size_t n_slots) {
    Resize(n_directions, n_slots);
  }

  void Resize(std::size_t n_directions, std::size_t n_slots) {
    n_directions_ = n_directions;
    n_slots_ = n_slots;
    values_.assign(n_directions_ * n_slots_, 0.0);
  }

  void Clear() { std::fill(values_.begin(), values_.end(), 0.0); }

  void EnsureSlotsPreserve(std::size_t n_slots) {
    if (n_slots <= n_slots_) {
      return;
    }

    std::vector<double> next(n_directions_ * n_slots, 0.0);

    for (std::size_t k = 0; k < n_directions_; ++k) {
      for (std::size_t slot = 0; slot < n_slots_; ++slot) {
        next[k * n_slots + slot] = values_[k * n_slots_ + slot];
      }
    }

    values_.swap(next);
    n_slots_ = n_slots;
  }

  std::size_t directions() const { return n_directions_; }
  std::size_t slots() const { return n_slots_; }

  double &operator()(std::size_t direction, std::size_t slot) {
    return values_[Index(direction, slot)];
  }

  double operator()(std::size_t direction, std::size_t slot) const {
    return values_[Index(direction, slot)];
  }

  void Add(std::size_t direction, std::size_t slot, double value) {
    values_[Index(direction, slot)] += value;
  }

  Eigen::Map<Eigen::VectorXd> DirectionView(std::size_t direction) {
    if (direction >= n_directions_) {
      throw std::out_of_range("DirectionView direction out of range");
    }

    return Eigen::Map<Eigen::VectorXd>(values_.data() + direction * n_slots_,
                                       static_cast<Eigen::Index>(n_slots_));
  }

  Eigen::Map<const Eigen::VectorXd> DirectionView(std::size_t direction) const {
    if (direction >= n_directions_) {
      throw std::out_of_range("DirectionView direction out of range");
    }

    return Eigen::Map<const Eigen::VectorXd>(
        values_.data() + direction * n_slots_,
        static_cast<Eigen::Index>(n_slots_));
  }

private:
  std::size_t Index(std::size_t direction, std::size_t slot) const {
    if (direction >= n_directions_ || slot >= n_slots_) {
      throw std::out_of_range(
          "BatchDirectionalFlatAccumulator index out of range");
    }
    return direction * n_slots_ + slot;
  }

  std::size_t n_directions_ = 0;
  std::size_t n_slots_ = 0;
  std::vector<double> values_;
};

} // namespace had

#endif // QUADRA_HAD_BATCH_DIRECTIONAL_FLAT_ACCUMULATOR_HPP
