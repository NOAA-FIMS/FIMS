#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace quadra {

enum class ParameterTransform { Identity, Log, Logit, Square };

inline std::string parameter_transform_name(ParameterTransform transform) {
  switch (transform) {
  case ParameterTransform::Identity:
    return "identity";
  case ParameterTransform::Log:
    return "log";
  case ParameterTransform::Logit:
    return "logit";
  case ParameterTransform::Square:
    return "square";
  default:
    return "unknown";
  }
}

inline std::string transform_name(ParameterTransform transform) {
  return parameter_transform_name(transform);
}

template <typename Type> inline Type logistic(const Type &x) {
  return Type(1.0) / (Type(1.0) + exp(-x));
}

template <typename Type>
inline Type apply_transform(const Type &x, ParameterTransform transform) {
  switch (transform) {
  case ParameterTransform::Identity:
    return x;

  case ParameterTransform::Log:
    return exp(x);

  case ParameterTransform::Logit:
    return logistic(x);

  case ParameterTransform::Square:
    return x * x;

  default:
    throw std::invalid_argument("Unknown parameter transform.");
  }
}

template <typename Type>
inline std::vector<Type>
apply_transforms(const std::vector<Type> &unconstrained,
                 const std::vector<ParameterTransform> &transforms) {
  if (unconstrained.size() != transforms.size()) {
    throw std::invalid_argument("apply_transforms: unconstrained and "
                                "transforms must have the same length.");
  }

  std::vector<Type> constrained;
  constrained.reserve(unconstrained.size());

  for (size_t i = 0; i < unconstrained.size(); ++i) {
    constrained.push_back(apply_transform(unconstrained[i], transforms[i]));
  }

  return constrained;
}

} // namespace quadra
