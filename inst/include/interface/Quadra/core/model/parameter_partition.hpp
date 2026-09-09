#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "parameter.hpp"

namespace quadra {

struct ParameterPartition {
  std::vector<size_t> fixed_indices_m;
  std::vector<size_t> random_indices_m;

  size_t n_fixed() const { return fixed_indices_m.size(); }

  size_t n_random() const { return random_indices_m.size(); }

  bool has_random_effects() const { return !random_indices_m.empty(); }
};

template <typename Type> struct PartitionedVector {
  std::vector<Type> fixed_m;
  std::vector<Type> random_m;
};

inline ParameterPartition partition_parameters(const ParameterSet &parameters) {
  ParameterPartition out;

  const auto &values = parameters.values();

  for (size_t i = 0; i < values.size(); ++i) {
    if (values[i].is_random_m) {
      out.random_indices_m.push_back(i);
    } else {
      out.fixed_indices_m.push_back(i);
    }
  }

  return out;
}

inline std::vector<size_t>
fixed_effect_indices(const ParameterSet &parameters) {
  return partition_parameters(parameters).fixed_indices_m;
}

inline std::vector<size_t>
random_effect_indices(const ParameterSet &parameters) {
  return partition_parameters(parameters).random_indices_m;
}

template <typename Type>
inline std::vector<Type>
extract_by_indices(const std::vector<Type> &values,
                   const std::vector<size_t> &indices) {
  std::vector<Type> out;
  out.reserve(indices.size());

  for (size_t idx : indices) {
    if (idx >= values.size()) {
      throw std::out_of_range(
          "extract_by_indices: index exceeds vector length");
    }

    out.push_back(values[idx]);
  }

  return out;
}

template <typename Type>
inline PartitionedVector<Type>
split_parameters(const std::vector<Type> &values,
                 const ParameterPartition &partition) {
  PartitionedVector<Type> out;
  out.fixed_m = extract_by_indices(values, partition.fixed_indices_m);
  out.random_m = extract_by_indices(values, partition.random_indices_m);
  return out;
}

template <typename Type>
inline PartitionedVector<Type>
split_parameters(const std::vector<Type> &values,
                 const ParameterSet &parameters) {
  return split_parameters(values, partition_parameters(parameters));
}

template <typename Type>
inline std::vector<Type> merge_parameters(const std::vector<Type> &fixed,
                                          const std::vector<Type> &random,
                                          const ParameterPartition &partition) {
  if (fixed.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument(
        "merge_parameters: fixed vector has incorrect length");
  }

  if (random.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument(
        "merge_parameters: random vector has incorrect length");
  }

  size_t n_total =
      partition.fixed_indices_m.size() + partition.random_indices_m.size();
  std::vector<Type> out(n_total);

  for (size_t i = 0; i < partition.fixed_indices_m.size(); ++i) {
    const size_t idx = partition.fixed_indices_m[i];

    if (idx >= n_total) {
      throw std::out_of_range(
          "merge_parameters: fixed index exceeds total length");
    }

    out[idx] = fixed[i];
  }

  for (size_t i = 0; i < partition.random_indices_m.size(); ++i) {
    const size_t idx = partition.random_indices_m[i];

    if (idx >= n_total) {
      throw std::out_of_range(
          "merge_parameters: random index exceeds total length");
    }

    out[idx] = random[i];
  }

  return out;
}

template <typename Type>
inline std::vector<Type>
merge_parameters(const PartitionedVector<Type> &partitioned,
                 const ParameterPartition &partition) {
  return merge_parameters(partitioned.fixed_m, partitioned.random_m, partition);
}

template <typename Type>
inline std::vector<Type>
merge_parameters(const PartitionedVector<Type> &partitioned,
                 const ParameterSet &parameters) {
  return merge_parameters(partitioned, partition_parameters(parameters));
}

inline std::vector<std::string>
parameter_names_by_indices(const ParameterSet &parameters,
                           const std::vector<size_t> &indices) {
  const auto names = parameters.names();
  return extract_by_indices(names, indices);
}

inline std::vector<std::string>
fixed_effect_names(const ParameterSet &parameters) {
  return parameter_names_by_indices(parameters,
                                    fixed_effect_indices(parameters));
}

inline std::vector<std::string>
random_effect_names(const ParameterSet &parameters) {
  return parameter_names_by_indices(parameters,
                                    random_effect_indices(parameters));
}

} // namespace quadra
