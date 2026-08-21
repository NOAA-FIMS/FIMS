#pragma once

#include <string>

#include "report.hpp"
#include "scalar_value.hpp"

namespace quadra {

class ModelReportContext {
public:
  template <typename Type>
  void report(const std::string &name, const Type &value) {
    reports_m.report(name, scalar_value(value));
  }

  template <typename Type>
  void adreport(const std::string &name, const Type &value) {
    reports_m.adreport(name, scalar_value(value));
  }

  const ReportStore &reports() const { return reports_m; }

  void clear() { reports_m.clear(); }

private:
  ReportStore reports_m;
};

} // namespace quadra
