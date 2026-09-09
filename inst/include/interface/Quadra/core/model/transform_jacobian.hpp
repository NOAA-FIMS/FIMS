#pragma once

#include <cmath>
#include <stdexcept>
#include <vector>

#include "parameter_transform.hpp"

namespace quadra {

template <typename Type>
inline Type transform_log_jacobian(const Type &x,
                                   ParameterTransform transform) {
  switch (transform) {
  case ParameterTransform::Identity:
    return Type(0.0);

  case ParameterTransform::Log:
    return x;

  case ParameterTransform::Logit: {
    Type y = logistic(x);
    return log(y) + log(Type(1.0) - y);
  }

  case ParameterTransform::Square: {
    Type z = Type(2.0) * x;
    return log(sqrt(z * z));
  }

  default:
    throw std::invalid_argument("Unknown parameter transform.");
  }
}

template <typename Type>
inline std::vector<Type>
transform_log_jacobians(const std::vector<Type> &unconstrained,
                        const std::vector<ParameterTransform> &transforms) {
  if (unconstrained.size() != transforms.size()) {
    throw std::invalid_argument("transform_log_jacobians: unconstrained and "
                                "transforms must have the same length.");
  }

  std::vector<Type> out;
  out.reserve(unconstrained.size());

  for (size_t i = 0; i < unconstrained.size(); ++i) {
    out.push_back(transform_log_jacobian(unconstrained[i], transforms[i]));
  }

  return out;
}

template <typename Type>
inline Type
sum_transform_log_jacobian(const std::vector<Type> &unconstrained,
                           const std::vector<ParameterTransform> &transforms) {
  if (unconstrained.size() != transforms.size()) {
    throw std::invalid_argument("sum_transform_log_jacobian: unconstrained and "
                                "transforms must have the same length.");
  }

  Type out = Type(0.0);

  for (size_t i = 0; i < unconstrained.size(); ++i) {
    out += transform_log_jacobian(unconstrained[i], transforms[i]);
  }

  return out;
}

} // namespace quadra
