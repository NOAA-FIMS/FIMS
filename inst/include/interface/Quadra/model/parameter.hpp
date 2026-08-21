#ifndef PARAMETER_HPP
#define PARAMETER_HPP
#pragma once

#include "../core/model/parameter_transform.hpp"

#include "../math/transforms.hpp"
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace quadra {

//==============================
// Transform types
//==============================
// enum class ParameterTransform {
//     Identity,
//     Log,
//     Logit
// };

// //==============================
// // Transform functions
// //==============================
// template <typename T>
// T apply_transform(T x, Transform t) {
//     switch (t) {
//         case ParameterTransform::Identity:
//             return x;

//         case Transform::Log:
//             return exp(x);  // positive

//         case Transform::Logit:
//             return T(1) / (T(1) + exp(-x));  // (0,1)

//         default:
//             throw std::runtime_error("Unknown transform");
//     }
// }

//==============================
// Parameter definition
//==============================
struct Parameter {
  std::string name;
  double value;
  ParameterTransform transform;
  bool is_random;
  int block_id;

  Parameter(std::string name, double value,
            ParameterTransform transform = ParameterTransform::Identity,
            bool is_random = false, int block_id = -1)
      : name(name), value(value), transform(transform), is_random(is_random),
        block_id(block_id) {}
};

class ParameterVector {
public:
  std::vector<Parameter> params;

  void add(const Parameter &p) { params.push_back(p); }

  size_t size() const { return params.size(); }

  //====================================
  // INDICES
  //====================================
  std::vector<size_t> fixed_indices() const {
    std::vector<size_t> idx;
    for (size_t i = 0; i < params.size(); ++i)
      if (!params[i].is_random)
        idx.push_back(i);
    return idx;
  }

  std::vector<size_t> random_indices() const {
    std::vector<size_t> idx;
    for (size_t i = 0; i < params.size(); ++i)
      if (params[i].is_random)
        idx.push_back(i);
    return idx;
  }

  //====================================
  // BLOCK GROUPING
  //====================================
  std::map<int, std::vector<size_t>> random_blocks() const {

    std::map<int, std::vector<size_t>> blocks;

    for (size_t i = 0; i < params.size(); ++i) {

      if (!params[i].is_random)
        continue;

      int bid = params[i].block_id;

      // fallback: each param its own block
      if (bid < 0)
        bid = static_cast<int>(i);

      blocks[bid].push_back(i);
    }

    return blocks;
  }

  //====================================
  // BLOCK SIZES (useful for debugging/perf)
  //====================================
  std::vector<size_t> block_sizes() const {
    std::vector<size_t> sizes;

    auto blocks = random_blocks();

    for (const auto &kv : blocks)
      sizes.push_back(kv.second.size());

    return sizes;
  }

  //====================================
  // VALUES
  //====================================
  std::vector<double> values() const {
    std::vector<double> x(params.size());
    for (size_t i = 0; i < params.size(); ++i)
      x[i] = params[i].value;
    return x;
  }
};

} // namespace quadra
#endif // PARAMETER_HPP