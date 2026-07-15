#pragma once

#include <cmath>
#include <limits>
#include <string>

namespace quadra {
namespace diagnostics {

struct ModelHealthStatus {
  std::string optimization = "UNKNOWN";
  std::string gradient = "UNKNOWN";
  std::string curvature = "UNKNOWN";
  std::string conditioning = "UNKNOWN";
  std::string overall = "UNKNOWN";
  std::string confidence = "UNKNOWN";
  std::string optimization_quality = "UNKNOWN";
};

inline double to_double_or_nan(const std::string &value) {
  try {
    return std::stod(value);
  } catch (...) {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

inline std::string health_pass_fail(const std::string &value,
                                    const std::string &pass_value = "yes") {
  return value == pass_value ? "PASS" : "CHECK";
}

inline std::string health_gradient_label(const std::string &value) {
  const double x = to_double_or_nan(value);
  if (!std::isfinite(x))
    return "UNKNOWN";
  if (x < 1.0e-2)
    return "PASS";
  if (x < 1.0e-1)
    return "CAUTION";
  return "CHECK";
}

inline std::string
health_label_from_condition_number(const std::string &value) {
  const double x = to_double_or_nan(value);
  if (!std::isfinite(x))
    return "UNKNOWN";
  if (x < 100.0)
    return "EXCELLENT";
  if (x < 1000.0)
    return "GOOD";
  if (x < 10000.0)
    return "CAUTION";
  return "HIGH RISK";
}

inline std::string optimization_quality_label(const std::string &converged,
                                              const std::string &grad_norm,
                                              const std::string &pd,
                                              const std::string &condition) {
  const double g = to_double_or_nan(grad_norm);
  const double k = to_double_or_nan(condition);

  if (converged == "yes" && pd == "yes" && std::isfinite(g) &&
      std::isfinite(k) && g < 1.0e-2 && k < 100.0) {
    return "EXCELLENT";
  }

  if (converged == "yes" && pd == "yes" && std::isfinite(g) &&
      std::isfinite(k) && g < 1.0e-1 && k < 1000.0) {
    return "GOOD";
  }

  if (converged == "yes")
    return "REVIEW";

  return "CHECK";
}

inline ModelHealthStatus evaluate_model_health(const std::string &converged,
                                               const std::string &grad_norm,
                                               const std::string &pd,
                                               const std::string &condition) {
  ModelHealthStatus out;
  out.optimization = health_pass_fail(converged);
  out.gradient = health_gradient_label(grad_norm);
  out.curvature = health_pass_fail(pd);
  out.conditioning = health_label_from_condition_number(condition);
  out.optimization_quality =
      optimization_quality_label(converged, grad_norm, pd, condition);

  const bool healthy =
      out.optimization == "PASS" && out.gradient == "PASS" &&
      out.curvature == "PASS" &&
      (out.conditioning == "EXCELLENT" || out.conditioning == "GOOD");

  const bool high_confidence =
      out.optimization == "PASS" && out.gradient == "PASS" &&
      out.curvature == "PASS" && out.conditioning == "EXCELLENT";

  out.overall = healthy ? "HEALTHY" : "REVIEW";
  out.confidence = high_confidence ? "HIGH" : (healthy ? "MODERATE" : "LOW");

  return out;
}

} // namespace diagnostics
} // namespace quadra
