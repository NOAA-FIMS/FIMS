/**
 * @file tmb_derived_quantity_uncertainty.hpp
 * @brief TMB-facing adapter for derived quantity uncertainty calculations.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_TMB_DERIVED_QUANTITY_UNCERTAINTY_HPP
#define FIMS_INTERFACE_TMB_DERIVED_QUANTITY_UNCERTAINTY_HPP

#include "../../common/derived_quantity_uncertainty.hpp"

namespace fims_tmb {

/**
 * @brief Fixed-effect ADREPORT information extracted from TMB.
 *
 * @details The Jacobian is expected in the same row-major layout returned by
 * TMBad and used internally by TMB before conversion to an R matrix.
 */
struct FixedEffectADReport {
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_fixed_effects = 0;
};

/**
 * @brief Random-effect ADREPORT information after TMB Laplace reduction.
 *
 * @details The adjusted fixed-effect Jacobian includes the reverse-sweep
 * correction used by TMB's sdreport. The random-effect covariance is the
 * inverse random-effect Hessian. All matrices are stored row-major.
 */
struct LaplaceADReport {
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> adjusted_fixed_jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<double> random_jacobian;
  fims::Vector<double> random_effect_covariance;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_fixed_effects = 0;
  size_t n_random_effects = 0;
};

/**
 * @brief Structured ADREPORT payload extracted from a TMB model.
 *
 * @details This type is the backend boundary for TMB-derived uncertainty. Rcpp
 * can translate an R list into this payload today, and a future TMB API
 * extractor can populate the same payload directly from C++.
 */
struct ADReportPayload {
  std::string method;
  fims_report::DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> jacobian;
  fims::Vector<double> fixed_jacobian;
  fims::Vector<double> adjusted_fixed_jacobian;
  fims::Vector<double> fixed_effect_covariance;
  fims::Vector<double> random_jacobian;
  fims::Vector<double> random_effect_covariance;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_fixed_effects = 0;
  size_t n_random_effects = 0;
};

/**
 * @brief Adapter from TMB fixed-effect ADREPORT output to FIMS uncertainty.
 */
class FixedEffectADReportUncertaintyAdapter {
 public:
  typedef std::vector<FixedEffectADReport> report_vector;
  typedef std::vector<fims_report::DerivedQuantityEstimate> estimate_vector;

  /**
   * @brief Calculate SEs from one TMB fixed-effect ADREPORT payload.
   *
   * @param report ADREPORT values, Jacobian, and fixed-effect covariance.
   * @return fims_report::DerivedQuantityEstimate Estimate and SEs.
   */
  fims_report::DerivedQuantityEstimate Calculate(
      const FixedEffectADReport& report) const {
    fims_report::DeltaMethodInput input;
    input.request = report.request;
    input.estimate = report.estimate;
    input.jacobian = report.jacobian;
    input.covariance = report.fixed_effect_covariance;
    input.dims = report.dims;
    input.dim_names = report.dim_names;
    input.n_parameters = report.n_fixed_effects;

    return this->calculator_m.Calculate(input);
  }

  /**
   * @brief Calculate SEs from multiple TMB fixed-effect ADREPORT payloads.
   *
   * @param reports ADREPORT payloads.
   * @return estimate_vector Estimate and SEs for each payload.
   */
  estimate_vector Calculate(const report_vector& reports) const {
    estimate_vector estimates;
    estimates.reserve(reports.size());
    for (size_t i = 0; i < reports.size(); i++) {
      estimates.push_back(this->Calculate(reports[i]));
    }
    return estimates;
  }

 private:
  fims_report::DeltaMethodUncertaintyCalculator calculator_m;
};

/**
 * @brief Adapter from TMB Laplace ADREPORT output to FIMS uncertainty.
 */
class LaplaceADReportUncertaintyAdapter {
 public:
  typedef std::vector<LaplaceADReport> report_vector;
  typedef std::vector<fims_report::DerivedQuantityEstimate> estimate_vector;

  /**
   * @brief Calculate SEs from one TMB Laplace ADREPORT payload.
   *
   * @param report ADREPORT values and Laplace delta-method ingredients.
   * @return fims_report::DerivedQuantityEstimate Estimate and SEs.
   */
  fims_report::DerivedQuantityEstimate Calculate(
      const LaplaceADReport& report) const {
    this->Validate(report);

    fims_report::DerivedQuantityEstimate output;
    output.request = report.request;
    output.estimate = report.estimate;
    output.dims = report.dims;
    output.dim_names = report.dim_names;
    output.se.resize(report.estimate.size());

    for (size_t i = 0; i < report.estimate.size(); i++) {
      double variance = 0.0;
      for (size_t j = 0; j < report.n_random_effects; j++) {
        const double d_i_j =
            report.random_jacobian[(i * report.n_random_effects) + j];
        for (size_t k = 0; k < report.n_random_effects; k++) {
          variance += d_i_j *
                      report.random_effect_covariance
                          [(j * report.n_random_effects) + k] *
                      report.random_jacobian
                          [(i * report.n_random_effects) + k];
        }
      }

      for (size_t j = 0; j < report.n_fixed_effects; j++) {
        const double d_i_j =
            report.adjusted_fixed_jacobian
                [(i * report.n_fixed_effects) + j];
        for (size_t k = 0; k < report.n_fixed_effects; k++) {
          variance += d_i_j *
                      report.fixed_effect_covariance
                          [(j * report.n_fixed_effects) + k] *
                      report.adjusted_fixed_jacobian
                          [(i * report.n_fixed_effects) + k];
        }
      }

      if (variance < 0.0 && variance > -1.0e-12) {
        variance = 0.0;
      }
      if (variance < 0.0) {
        std::ostringstream ss;
        ss << "LaplaceADReportUncertaintyAdapter::Calculate: negative "
           << "variance for element " << i << ": " << variance;
        throw std::runtime_error(ss.str());
      }
      output.se[i] = std::sqrt(variance);
    }

    return output;
  }

  /**
   * @brief Calculate SEs from multiple TMB Laplace ADREPORT payloads.
   *
   * @param reports ADREPORT payloads.
   * @return estimate_vector Estimate and SEs for each payload.
   */
  estimate_vector Calculate(const report_vector& reports) const {
    estimate_vector estimates;
    estimates.reserve(reports.size());
    for (size_t i = 0; i < reports.size(); i++) {
      estimates.push_back(this->Calculate(reports[i]));
    }
    return estimates;
  }

 private:
  /**
   * @brief Validate TMB Laplace ADREPORT inputs.
   *
   * @param report ADREPORT payload.
   */
  void Validate(const LaplaceADReport& report) const {
    if (report.estimate.size() == 0) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: estimate cannot be "
          "empty");
    }
    if (report.n_fixed_effects == 0) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: n_fixed_effects must "
          "be greater than zero");
    }
    if (report.n_random_effects == 0) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: n_random_effects must "
          "be greater than zero");
    }
    if (report.adjusted_fixed_jacobian.size() !=
        report.estimate.size() * report.n_fixed_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: adjusted fixed "
          "Jacobian size is invalid");
    }
    if (report.fixed_effect_covariance.size() !=
        report.n_fixed_effects * report.n_fixed_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: fixed covariance size "
          "is invalid");
    }
    if (report.random_jacobian.size() !=
        report.estimate.size() * report.n_random_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: random Jacobian size "
          "is invalid");
    }
    if (report.random_effect_covariance.size() !=
        report.n_random_effects * report.n_random_effects) {
      throw std::invalid_argument(
          "LaplaceADReportUncertaintyAdapter::Validate: random covariance "
          "size is invalid");
    }
  }
};

/**
 * @brief Dispatch ADREPORT payloads to the correct backend uncertainty adapter.
 */
class ADReportPayloadUncertaintyCalculator {
 public:
  /**
   * @brief Calculate standard errors from a structured ADREPORT payload.
   *
   * @param payload ADREPORT payload.
   * @return fims_report::DerivedQuantityEstimate Estimate and SEs.
   */
  fims_report::DerivedQuantityEstimate Calculate(
      const ADReportPayload& payload) const {
    if (payload.method == "laplace") {
      LaplaceADReport report;
      report.request = payload.request;
      report.estimate = payload.estimate;
      report.adjusted_fixed_jacobian = payload.adjusted_fixed_jacobian;
      report.fixed_effect_covariance = payload.fixed_effect_covariance;
      report.random_jacobian = payload.random_jacobian;
      report.random_effect_covariance = payload.random_effect_covariance;
      report.dims = payload.dims;
      report.dim_names = payload.dim_names;
      report.n_fixed_effects = payload.n_fixed_effects;
      report.n_random_effects = payload.n_random_effects;

      LaplaceADReportUncertaintyAdapter adapter;
      return adapter.Calculate(report);
    }

    FixedEffectADReport report;
    report.request = payload.request;
    report.estimate = payload.estimate;
    if (payload.method == "fixed_after_laplace") {
      report.jacobian = payload.fixed_jacobian;
    } else if (payload.method == "fixed") {
      report.jacobian = payload.jacobian;
    } else {
      std::ostringstream ss;
      ss << "ADReportPayloadUncertaintyCalculator::Calculate: unsupported "
         << "method '" << payload.method << "'";
      throw std::invalid_argument(ss.str());
    }
    report.fixed_effect_covariance = payload.fixed_effect_covariance;
    report.dims = payload.dims;
    report.dim_names = payload.dim_names;
    report.n_fixed_effects = payload.n_fixed_effects;

    FixedEffectADReportUncertaintyAdapter adapter;
    return adapter.Calculate(report);
  }
};

}  // namespace fims_tmb

#endif
