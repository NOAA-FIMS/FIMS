#ifndef QUADRA_EVALUATION_HPP
#define QUADRA_EVALUATION_HPP

#pragma once

#include <iostream>
#include <vector>

#include "../core/autodiff.hpp"
#include "../math/transforms.hpp"
#include "../model/parameter.hpp"

namespace quadra {

struct ModelContext {
  std::vector<double> Y;
};

//====================================================
// 1. Build full parameter vector
//====================================================
template <typename Scalar>
inline void build_parameters(const ParameterVector &params,
                             const std::vector<Scalar> &u,
                             std::vector<Scalar> &x) {
  size_t k = 0;

  for (size_t i = 0; i < params.size(); ++i) {
    if (params.params[i].is_random)
      x[i] = u[k++];
    else
      x[i] = Scalar(params.params[i].value);
  }
}

//====================================================
// 2. Transform + Jacobian
//====================================================
template <typename Scalar>
inline Scalar apply_transforms(const ParameterVector &params,
                               const std::vector<Scalar> &x,
                               std::vector<Scalar> &t) {
  Scalar log_jacobian = Scalar(0.0);

  for (size_t i = 0; i < x.size(); ++i) {
    t[i] = apply_transform(x[i], params.params[i].transform);

    log_jacobian += apply_log_jacobian(x[i], params.params[i].transform);
  }

  return log_jacobian;
}

//====================================================
// 3. SINGLE source of truth for NLL
//====================================================
template <typename Scalar, typename Model>
Scalar evaluate_nll(Model &model, const ParameterVector &params,
                    const std::vector<Scalar> &u, bool include_jacobian = true,
                    bool verbose = false) {
  std::vector<Scalar> x(params.size());
  std::vector<Scalar> t(params.size());

  if (verbose) {
    std::cout << "u = ";
    for (const auto &ui : u)
      std::cout << value_of_arithmetic_or_ad(ui) << " ";
    std::cout << "\n";
  }

  build_parameters(params, u, x);

  Scalar log_jacobian = apply_transforms(params, x, t);
  Scalar nll = model(t);

  if (include_jacobian)
    nll -= log_jacobian;

  return nll;
}

// Backward-compatible overload for existing AReal callers.
template <typename Model>
AD evaluate_nll(Model &model, const ParameterVector &params,
                const std::vector<AD> &u, bool include_jacobian = true) {
  return evaluate_nll<AD>(model, params, u, include_jacobian, false);
}

template <typename F, typename Scalar>
Scalar evaluate(F &f, const std::vector<Scalar> &x) {
  return f(x);
}

} // namespace quadra

#endif // QUADRA_EVALUATION_HPP
