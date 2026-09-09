#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "parameter_transform.hpp"
#include "transform_jacobian.hpp"

namespace quadra {

struct ParameterInfo {
  std::string name_m;
  double initial_value_m = 0.0;
  ParameterTransform transform_m = ParameterTransform::Identity;
  bool is_random_m = false;
};

class ParameterSet {
public:
  void add(const std::string &name, double initial_value,
           ParameterTransform transform = ParameterTransform::Identity,
           bool is_random = false) {
    parameters_m.push_back({name, initial_value, transform, is_random});
  }

  size_t size() const { return parameters_m.size(); }

  const std::vector<ParameterInfo> &values() const { return parameters_m; }

  std::vector<std::string> names() const {
    std::vector<std::string> out;
    out.reserve(parameters_m.size());

    for (const auto &p : parameters_m) {
      out.push_back(p.name_m);
    }

    return out;
  }

  std::vector<double> initials() const {
    std::vector<double> out;
    out.reserve(parameters_m.size());

    for (const auto &p : parameters_m) {
      out.push_back(p.initial_value_m);
    }

    return out;
  }

  std::vector<ParameterTransform> transforms() const {
    std::vector<ParameterTransform> out;
    out.reserve(parameters_m.size());

    for (const auto &p : parameters_m) {
      out.push_back(p.transform_m);
    }

    return out;
  }

  std::vector<size_t> random_effect_indices() const {
    std::vector<size_t> out;

    for (size_t i = 0; i < parameters_m.size(); ++i) {
      if (parameters_m[i].is_random_m) {
        out.push_back(i);
      }
    }

    return out;
  }

private:
  std::vector<ParameterInfo> parameters_m;
};

} // namespace quadra
