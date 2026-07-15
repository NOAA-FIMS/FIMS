#pragma once

#include "../model/model_result.hpp"
#include "../model/quadra_model.hpp"

namespace quadra {

class ModelRunner {
public:
  explicit ModelRunner(QuadraModelBase &model) : model_m(model) {}

  ModelResult evaluate_once(const std::vector<double> &parameters) {
    ModelReportContext ctx;
    model_m.initialize(ctx);

    double value = model_m.evaluate(parameters, ctx);

    ModelResult result;
    result.objective_value_m = value;
    result.parameters_m = parameters;
    result.converged_m = true;
    result.message_m = "Evaluated model once.";
    result.reports_m = ctx.reports().values();

    return result;
  }

private:
  QuadraModelBase &model_m;
};

template <class Model>
ModelResult evaluate_once(Model &model, const std::vector<double> &parameters) {
  ModelReportContext ctx;
  model.initialize(ctx);

  double value = model.template evaluate<double>(parameters, ctx);

  ModelResult result;
  result.objective_value_m = value;
  result.parameters_m = parameters;
  result.converged_m = true;
  result.message_m = "Evaluated scalar-generic model once.";
  result.reports_m = ctx.reports().values();

  return result;
}

} // namespace quadra
