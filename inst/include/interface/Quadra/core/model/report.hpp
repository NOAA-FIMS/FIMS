#pragma once

#include <string>
#include <vector>

namespace quadra {

struct ReportValue {
  std::string name_m;
  double value_m = 0.0;
  bool requires_se_m = false;
};

class ReportStore {
public:
  void report(const std::string &name, double value) {
    values_m.push_back({name, value, false});
  }

  void adreport(const std::string &name, double value) {
    values_m.push_back({name, value, true});
  }

  const std::vector<ReportValue> &values() const { return values_m; }

  void clear() { values_m.clear(); }

private:
  std::vector<ReportValue> values_m;
};

} // namespace quadra
