#pragma once

#include "model_health.hpp"

#include <sstream>
#include <string>

namespace quadra {
namespace diagnostics {

inline std::string uncertainty_structure_label(const std::string &avg_degree,
                                               const std::string &max_degree,
                                               const std::string &diameter,
                                               const std::string &nodes) {
  const double avg = to_double_or_nan(avg_degree);
  const double maxd = to_double_or_nan(max_degree);
  const double dia = to_double_or_nan(diameter);
  const double n = to_double_or_nan(nodes);

  if (!std::isfinite(avg) || !std::isfinite(maxd))
    return "UNKNOWN";

  if (avg <= 2.0 && maxd <= 3.0)
    return "LOCAL";

  if (std::isfinite(n) && std::isfinite(dia) && n > 0.0 && avg <= 0.25 * n &&
      dia >= 0.25 * n)
    return "MODERATE";

  if (std::isfinite(n) && n > 0.0 && avg > 0.5 * n)
    return "GLOBAL";

  return "MIXED";
}

inline std::string compression_label(const std::string &structural_nonzeros,
                                     const std::string &entries_required) {
  const double nz = to_double_or_nan(structural_nonzeros);
  const double keep = to_double_or_nan(entries_required);
  if (!std::isfinite(nz) || !std::isfinite(keep) || keep <= 0.0)
    return "";

  std::ostringstream os;
  os << (nz / keep);
  return os.str();
}

} // namespace diagnostics
} // namespace quadra
