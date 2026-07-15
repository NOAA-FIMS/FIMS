#ifndef TRANSFORMS_HPP
#define TRANSFORMS_HPP
#include "../core/autodiff.hpp"
namespace quadra {

//==============================
// Transform types
//==============================
enum class Transform { Identity, Log, Logit };

template <typename T> T apply_transform(T x, Transform t) {
  switch (t) {
  case Transform::Log:
    return exp(x);
  case Transform::Logit:
    return exp(x) / (1.0 + exp(x));
  case Transform::Identity:
  default:
    return x;
  }
}

template <typename T> T apply_log_jacobian(const T &x, Transform t) {
  switch (t) {
  case Transform::Log:
    return x;

  case Transform::Logit: {
    T ex = exp(x);
    T denom = 1.0 + ex;
    return log(ex) - 2.0 * log(denom);
  }

  case Transform::Identity:
  default:
    return T(0.0);
  }
}

// //----------------------------------------
// // Identity
// //----------------------------------------
// inline TransformResult identity_transform(const AD& x) {
//     return {x, AD(0.0)};
// }

// //----------------------------------------
// // Log transform: theta = exp(x)
// //----------------------------------------
// inline TransformResult log_transform(const AD& x) {
//     TransformResult out;
//     out.value = exp(x);
//     out.log_jacobian = x;
//     return out;
// }

// //----------------------------------------
// // Logit transform: (0,1)
// //----------------------------------------
// inline TransformResult logit_transform(const AD& x) {

//     AD ex = exp(x);
//     AD denom = 1.0 + ex;

//     TransformResult out;
//     out.value = ex / denom;
//     out.log_jacobian = log(ex) - 2.0 * log(denom);

//     return out;
// }

// //----------------------------------------
// // Dispatcher
// //----------------------------------------
// inline TransformResult apply_transform_full(
//     const AD& x,
//     Transform t
// ) {
//     switch (t) {
//         case Transform::Log:
//             return log_transform(x);
//         case Transform::Logit:
//             return logit_transform(x);
//         case Transform::Identity:
//         default:
//             return identity_transform(x);
//     }
// }

} // namespace quadra
#endif // TRANSFORMS_HPP