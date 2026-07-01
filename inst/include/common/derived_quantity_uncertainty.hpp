/**
 * @file derived_quantity_uncertainty.hpp
 * @brief Backend-neutral uncertainty calculations for derived quantities.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_DERIVED_QUANTITY_UNCERTAINTY_HPP
#define FIMS_DERIVED_QUANTITY_UNCERTAINTY_HPP

#include <cmath>
#include <sstream>
#include <stdexcept>

#include "derived_quantity_report.hpp"

namespace fims_report {

/**
 * @brief Inputs needed for a delta-method uncertainty calculation.
 */
struct DeltaMethodInput {
  DerivedQuantityReportRequest request;
  fims::Vector<double> estimate;
  fims::Vector<double> jacobian;
  fims::Vector<double> covariance;
  fims::Vector<int> dims;
  fims::Vector<std::string> dim_names;
  size_t n_parameters = 0;
};

/**
 * @brief Backend-neutral delta-method calculator for derived quantities.
 *
 * @details The Jacobian is stored row-major with one row per derived quantity
 * element and one column per parameter. The covariance matrix is stored
 * row-major with dimensions n_parameters by n_parameters.
 */
class DeltaMethodUncertaintyCalculator {
 public:
  typedef std::vector<DeltaMethodInput> input_vector;
  typedef std::vector<DerivedQuantityEstimate> estimate_vector;

  /**
   * @brief Calculate derived quantity standard errors.
   *
   * @param input Delta-method inputs.
   * @return DerivedQuantityEstimate Estimate and standard errors.
   */
  DerivedQuantityEstimate Calculate(const DeltaMethodInput& input) const {
    this->Validate(input);

    DerivedQuantityEstimate output;
    output.request = input.request;
    output.estimate = input.estimate;
    output.dims = input.dims;
    output.dim_names = input.dim_names;
    output.se.resize(input.estimate.size());

    for (size_t i = 0; i < input.estimate.size(); i++) {
      double variance = 0.0;
      for (size_t j = 0; j < input.n_parameters; j++) {
        const double d_i_j =
            input.jacobian[(i * input.n_parameters) + j];
        for (size_t k = 0; k < input.n_parameters; k++) {
          variance +=
              d_i_j * input.covariance[(j * input.n_parameters) + k] *
              input.jacobian[(i * input.n_parameters) + k];
        }
      }

      if (variance < 0.0 && variance > -1.0e-12) {
        variance = 0.0;
      }
      if (variance < 0.0) {
        std::ostringstream ss;
        ss << "DeltaMethodUncertaintyCalculator::Calculate: negative "
           << "variance for element " << i << ": " << variance;
        throw std::runtime_error(ss.str());
      }
      output.se[i] = std::sqrt(variance);
    }

    return output;
  }

  /**
   * @brief Calculate standard errors for multiple derived quantities.
   *
   * @param inputs Delta-method inputs.
   * @return estimate_vector Estimate and standard errors for each input.
   */
  estimate_vector Calculate(const input_vector& inputs) const {
    estimate_vector outputs;
    outputs.reserve(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) {
      outputs.push_back(this->Calculate(inputs[i]));
    }
    return outputs;
  }

 private:
  /**
   * @brief Validate delta-method inputs.
   *
   * @param input Delta-method inputs.
   */
  void Validate(const DeltaMethodInput& input) const {
    if (input.n_parameters == 0) {
      throw std::invalid_argument(
          "DeltaMethodUncertaintyCalculator::Validate: n_parameters must be "
          "greater than zero");
    }
    if (input.estimate.size() == 0) {
      throw std::invalid_argument(
          "DeltaMethodUncertaintyCalculator::Validate: estimate cannot be "
          "empty");
    }
    if (input.jacobian.size() != input.estimate.size() * input.n_parameters) {
      std::ostringstream ss;
      ss << "DeltaMethodUncertaintyCalculator::Validate: jacobian size "
         << input.jacobian.size() << " does not match estimate size "
         << input.estimate.size() << " times n_parameters "
         << input.n_parameters;
      throw std::invalid_argument(ss.str());
    }
    if (input.covariance.size() != input.n_parameters * input.n_parameters) {
      std::ostringstream ss;
      ss << "DeltaMethodUncertaintyCalculator::Validate: covariance size "
         << input.covariance.size() << " does not match n_parameters squared "
         << input.n_parameters * input.n_parameters;
      throw std::invalid_argument(ss.str());
    }
  }
};

}  // namespace fims_report

#endif
