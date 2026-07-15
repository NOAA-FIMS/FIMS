#ifndef OBJECTIVE_HPP
#define OBJECTIVE_HPP
#pragma once

#include <vector>

#include "../core/autodiff.hpp"
#include "../core/evaluation.hpp"
#include "parameter.hpp"
namespace quadra {

//==============================
// Evaluation wrapper
//==============================
template <typename Model>
had::AReal evaluate_model(Model &model, const ParameterVector &params,
                          const std::vector<had::AReal> &x) {
  // Unconstrained values
  // std::vector<double> x = parameters.values();

  // // AD-compatible lambda
  // auto f = [&](const std::vector<AD>& x_ad) {

  //     // Apply transforms inside AD
  //     std::vector<AD> transformed(x_ad.size());

  //     for (size_t i = 0; i < x_ad.size(); ++i) {
  //         transformed[i] =
  //             apply_transform(x_ad[i],
  //                             parameters.params[i].transform);
  //     }

  //     // Call user model
  //     return model(transformed);
  // };

  // auto x_ad = to_ad(x);
  // AD y = quadra::evaluate(f, x_ad);

  // ADResult res;
  // res.value = y.val;   // HAD uses `.val`, not `.value()`
  // return res;
  // auto x_ad = to_ad(x);
  for (int i = 0; i < x.size(); i++)
    std::cout << "model sees x_ad[" << i << "] = " << x[i].val << "\n";
  AD y = evaluate(model, x);

  // ADResult res;
  // res.value = y.val;

  return y;
}

} // namespace quadra

#endif // OBJECTIVE_HPP