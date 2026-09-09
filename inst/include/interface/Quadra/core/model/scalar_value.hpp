#pragma once

#include <type_traits>
#include <utility>

namespace quadra {

namespace detail {

template <typename T, typename = void>
struct has_val_member : std::false_type {};

template <typename T>
struct has_val_member<T, std::void_t<decltype(std::declval<T>().val)>>
    : std::true_type {};

} // namespace detail

template <typename T> inline double scalar_value(const T &x) {
  if constexpr (std::is_arithmetic_v<T>) {
    return static_cast<double>(x);
  } else if constexpr (detail::has_val_member<T>::value) {
    return static_cast<double>(x.val);
  } else {
    static_assert(std::is_arithmetic_v<T> || detail::has_val_member<T>::value,
                  "quadra::scalar_value requires an arithmetic type or an AD "
                  "scalar with a .val member");
  }
}

} // namespace quadra
