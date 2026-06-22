/**
 * @file multinomial_likelihood.hpp
 * @brief Negative log-likelihood component for multinomial errors.
 */
#ifndef FIMS_MULTINOMIAL_LIKELIHOOD_HPP
#define FIMS_MULTINOMIAL_LIKELIHOOD_HPP

#include "likelihood_component_base.hpp"

namespace fims_likelihood {

template <typename Type>
struct MultinomialLikelihood : public LikelihoodComponentBase<Type> {
  fims::Vector<size_t> dims;

  std::string Name() const override { return "MultinomialLikelihood"; }

  Type Evaluate() override {
    if (dims.size() != 2) {
      throw std::invalid_argument(
          "MultinomialLikelihood::Evaluate: dims must have two elements.");
    }

    size_t n_rows = dims[0];
    size_t n_cols = dims[1];
    size_t n = n_rows * n_cols;
    if (this->GetInputSize() != n) {
      throw std::invalid_argument(
          "MultinomialLikelihood::Evaluate: input size does not "
          "match dims.");
    }
    size_t expected_size = this->expected_values.size();
    if (this->expected_values_input != nullptr) {
      expected_size = this->expected_values_input->size();
    }
    if (expected_size != n) {
      throw std::invalid_argument(
          "MultinomialLikelihood::Evaluate: expected_values size does not "
          "match dims.");
    }

    this->PrepareComponents(n_rows);

    for (size_t i = 0; i < n_rows; i++) {
      bool skip_row = false;
      Type row_total = static_cast<Type>(0.0);
      Type log_density = static_cast<Type>(0.0);

      for (size_t j = 0; j < n_cols; j++) {
        size_t idx = i * n_cols + j;
        if (this->GetInput(idx) == this->na_value) {
          skip_row = true;
          break;
        }
        row_total += this->GetInput(idx);
      }
      if (skip_row) {
        continue;
      }

      log_density += fims_math::lgamma(row_total + static_cast<Type>(1.0));
      for (size_t j = 0; j < n_cols; j++) {
        size_t idx = i * n_cols + j;
        Type observed = this->GetInput(idx);
        Type probability = this->GetExpected(idx);
        log_density -= fims_math::lgamma(observed + static_cast<Type>(1.0));
        log_density += observed * fims_math::log(probability);
      }

      this->nll_components[i] = -log_density;
      this->nll += this->nll_components[i];
    }

    return this->nll;
  }
};

}  // namespace fims_likelihood

#endif /* FIMS_MULTINOMIAL_LIKELIHOOD_HPP */
