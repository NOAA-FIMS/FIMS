/**
 * @file fims_vector_impl.hpp
 * @brief Implementation details for `fims::Vector` free functions.
 */
#ifndef FIMS_VECTOR_IMPL_HPP
#define FIMS_VECTOR_IMPL_HPP

#include <ostream>

namespace fims {

/** @brief Comparison operator. */
template <class T>
bool operator==(const fims::Vector<T> &lhs, const fims::Vector<T> &rhs) {
  return lhs.vec_m == rhs.vec_m;
}

}  // namespace fims

/** @brief Output for std::ostream& for a vector. */
template <typename Type>
std::ostream &operator<<(std::ostream &out, const fims::Vector<Type> &v) {
  out << std::fixed << std::setprecision(10);
  out << "[";

  if (v.size() == 0) {
    out << "]";
    return out;
  }
  for (size_t i = 0; i < v.size() - 1; i++) {
    if (v[i] != v[i]) {
      out << "-999" << ",";
    } else {
      out << v[i] << ",";
    }
  }
  if (v[v.size() - 1] != v[v.size() - 1]) {
    out << "-999]";
  } else {
    out << v[v.size() - 1] << "]";
  }
  return out;
}

#endif /* FIMS_VECTOR_IMPL_HPP */
