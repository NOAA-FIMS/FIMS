#pragma once

#include "report.hpp"
#include <string>
#include <vector>

namespace quadra {

struct ModelResult {
  double objective_value_m = 0.0;
  double gradient_norm_m = 0.0;
  int iterations_m = 0;
  bool converged_m = false;
  std::string message_m;

  std::vector<double> parameters_m;
  std::vector<ReportValue> reports_m;
};

} // namespace quadra
