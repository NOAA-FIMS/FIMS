/**
 * @file derived_quantity_report.hpp
 * @brief Backend-neutral request and result types for derived quantity reports.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_DERIVED_QUANTITY_REPORT_HPP
#define FIMS_DERIVED_QUANTITY_REPORT_HPP

#include <cstdint>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "fims_vector.hpp"

namespace fims_report {

/**
 * @brief Identifies the model component that owns a derived quantity.
 */
enum class DerivedQuantityComponentType { model, population, fleet, unknown };

/**
 * @brief Convert a derived quantity component type to a stable report label.
 *
 * @param component_type Component type.
 * @return std::string Stable label used in default report names.
 */
inline std::string ToString(DerivedQuantityComponentType component_type) {
  switch (component_type) {
    case DerivedQuantityComponentType::model:
      return "model";
    case DerivedQuantityComponentType::population:
      return "population";
    case DerivedQuantityComponentType::fleet:
      return "fleet";
    case DerivedQuantityComponentType::unknown:
    default:
      return "unknown";
  }
}

/**
 * @brief A request to report a derived quantity and optionally calculate its
 * SE.
 */
struct DerivedQuantityReportRequest {
  uint32_t model_id = 0;
  uint32_t component_id = 0;
  DerivedQuantityComponentType component_type =
      DerivedQuantityComponentType::unknown;
  std::string quantity_name;
  std::string report_name;
  bool report_value = true;
  bool report_se = false;

  /**
   * @brief Build a stable default report name.
   *
   * @return std::string Default report name.
   */
  std::string BuildDefaultReportName() const {
    std::ostringstream ss;
    ss << "model." << model_id << "." << ToString(component_type);
    if (component_type != DerivedQuantityComponentType::model) {
      ss << "." << component_id;
    }
    ss << "." << quantity_name;
    return ss.str();
  }
};

/**
 * @brief A backend-neutral estimate and uncertainty result.
 */
struct DerivedQuantityEstimate {
  DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> se;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
};

/**
 * @brief Stores derived quantity reporting requests for a model.
 */
class DerivedQuantityReportRegistry {
 public:
  typedef std::vector<DerivedQuantityReportRequest> request_vector;

 private:
  request_vector requests_m;
  std::map<std::string, size_t> report_name_index_m;

 public:
  /**
   * @brief Register a derived quantity reporting request.
   *
   * @param request Request to add.
   * @return const DerivedQuantityReportRequest& Stored request.
   */
  const DerivedQuantityReportRequest& Add(
      DerivedQuantityReportRequest request) {
    if (request.quantity_name.empty()) {
      throw std::invalid_argument(
          "DerivedQuantityReportRegistry::Add: quantity_name cannot be empty");
    }
    if (request.report_name.empty()) {
      request.report_name = request.BuildDefaultReportName();
    }
    if (report_name_index_m.find(request.report_name) !=
        report_name_index_m.end()) {
      std::ostringstream ss;
      ss << "DerivedQuantityReportRegistry::Add: report_name '"
         << request.report_name << "' already exists";
      throw std::invalid_argument(ss.str());
    }

    report_name_index_m[request.report_name] = requests_m.size();
    requests_m.push_back(request);
    return requests_m.back();
  }

  /**
   * @brief Remove all registered requests.
   */
  void Clear() {
    requests_m.clear();
    report_name_index_m.clear();
  }

  /**
   * @brief Get registered requests.
   *
   * @return const request_vector& Registered requests.
   */
  const request_vector& GetRequests() const { return requests_m; }

  /**
   * @brief Get number of registered requests.
   *
   * @return size_t Number of requests.
   */
  size_t size() const { return requests_m.size(); }

  /**
   * @brief Check whether there are no registered requests.
   *
   * @return bool True if empty.
   */
  bool empty() const { return requests_m.empty(); }
};

}  // namespace fims_report

#endif
