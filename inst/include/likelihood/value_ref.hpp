/**
 * @file value_ref.hpp
 * @brief Lightweight references to scalar or vector likelihood inputs.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_LIKELIHOOD_VALUE_REF_HPP
#define FIMS_LIKELIHOOD_VALUE_REF_HPP

#include <functional>
#include <stdexcept>

#include "parameter.hpp"

namespace fims_likelihood {

/**
 * @brief Read-only scalar/vector value accessor for likelihood terms.
 *
 * @details ValueRef is intentionally small: it stores a getter and a size. This
 * lets a likelihood term read constants, data vectors, model outputs, or
 * transformed parameters through the same interface without owning them.
 */
template <typename Type>
struct ValueRef {
  std::function<Type(size_t)> get;
  size_t size_m = 0;

  ValueRef() {}

  ValueRef(std::function<Type(size_t)> get, size_t size)
      : get(get), size_m(size) {}

  Type operator[](size_t i) const {
    if (!this->get) {
      throw std::runtime_error("ValueRef has no getter.");
    }
    return this->get(i);
  }

  size_t size() const { return this->size_m; }
};

/**
 * @brief Create a scalar ValueRef from a constant.
 */
template <typename Type>
inline ValueRef<Type> constant_ref(const Type& value) {
  return ValueRef<Type>([value](size_t) { return value; }, 1);
}

/**
 * @brief Create a ValueRef from a fims::Vector-like object.
 */
template <typename VectorType>
inline auto vector_ref(VectorType& values)
    -> ValueRef<typename VectorType::value_type> {
  typedef typename VectorType::value_type Type;
  return ValueRef<Type>(
      [&values](size_t i) -> Type {
        size_t index = values.size() == 1 ? 0 : i;
        return values[index];
      },
      values.size());
}

/**
 * @brief Create a ValueRef from a transformed Parameter.
 */
template <typename ParameterType>
inline auto parameter_ref(ParameterType& parameter)
    -> ValueRef<typename ParameterType::value_type> {
  typedef typename ParameterType::value_type Type;
  return ValueRef<Type>(
      [&parameter](size_t i) -> Type { return parameter.value(i); },
      parameter.size());
}

}  // namespace fims_likelihood

#endif
