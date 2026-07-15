#pragma once

#include <stdexcept>
#include <vector>

#include "../autodiff.hpp"
#include "../model/model_context.hpp"
#include "../model/model_result.hpp"
#include "../model/parameter_partition.hpp"

namespace quadra {

struct RandomEffectObjectiveResult {
  double objective_value_m = 0.0;
  double gradient_norm_m = 0.0;

  std::vector<double> fixed_m;
  std::vector<double> random_m;
  std::vector<double> full_m;
  std::vector<double> gradient_random_m;

  std::vector<ReportValue> reports_m;
};

inline double random_effect_gradient_norm(const std::vector<double> &gradient) {
  double out = 0.0;

  for (double gi : gradient) {
    out += gi * gi;
  }

  return std::sqrt(out);
}

// Evaluate f(theta, u) by merging fixed and random vectors into the
// original full parameter order described by `partition`.
template <class Model, typename Type>
inline Type evaluate_fixed_random(Model &model, const std::vector<Type> &fixed,
                                  const std::vector<Type> &random,
                                  const ParameterPartition &partition,
                                  ModelReportContext &ctx) {
  std::vector<Type> full = merge_parameters(fixed, random, partition);
  return model.template evaluate<Type>(full, ctx);
}

// Evaluate f(theta, u) with double scalars.
template <class Model>
inline double evaluate_random_effect_objective_value(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random, const ParameterPartition &partition) {
  ModelReportContext ctx;
  model.initialize(ctx);

  return evaluate_fixed_random<Model, double>(model, fixed, random, partition,
                                              ctx);
}

// Evaluate f(theta, u) and gradient wrt u only.
//
// This is the first Laplace-facing wrapper: it keeps theta fixed, treats u
// as the active AD variables, merges (theta, u) back into full parameter
// order, and returns the random-effect gradient.
template <class Model>
inline RandomEffectObjectiveResult evaluate_random_effect_objective_gradient(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random, const ParameterPartition &partition) {
  if (random.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument("evaluate_random_effect_objective_gradient: "
                                "random vector has incorrect length");
  }

  if (fixed.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument("evaluate_random_effect_objective_gradient: "
                                "fixed vector has incorrect length");
  }

  TapeContext tape;
  ADScope scope(tape.graph);

  ModelReportContext ctx;
  model.initialize(ctx);

  std::vector<AD> fixed_ad;
  fixed_ad.reserve(fixed.size());
  for (double theta_i : fixed) {
    fixed_ad.push_back(AD(theta_i));
  }

  std::vector<AD> random_ad = to_ad(random);

  AD objective = evaluate_fixed_random<Model, AD>(model, fixed_ad, random_ad,
                                                  partition, ctx);

  scope.backward(objective);
  Eigen::VectorXd g = extract_gradient(random_ad);

  RandomEffectObjectiveResult result;
  result.objective_value_m = value_of(objective);
  result.fixed_m = fixed;
  result.random_m = random;
  result.full_m = merge_parameters(fixed, random, partition);
  result.reports_m = ctx.reports().values();
  result.gradient_random_m.resize(static_cast<size_t>(g.size()));

  for (int i = 0; i < g.size(); ++i) {
    result.gradient_random_m[static_cast<size_t>(i)] = g[i];
  }

  result.gradient_norm_m =
      random_effect_gradient_norm(result.gradient_random_m);

  return result;
}

// Convenience overload using a ParameterSet.
template <class Model>
inline RandomEffectObjectiveResult evaluate_random_effect_objective_gradient(
    Model &model, const std::vector<double> &fixed,
    const std::vector<double> &random, const ParameterSet &parameters) {
  return evaluate_random_effect_objective_gradient(
      model, fixed, random, partition_parameters(parameters));
}

} // namespace quadra
