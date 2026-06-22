/**
 * @file invgamma_likelihood.hpp
 * @brief Negative log-likelihood component for inverse-gamma errors.
 */
#ifndef FIMS_INVGAMMA_LIKELIHOOD_HPP
#define FIMS_INVGAMMA_LIKELIHOOD_HPP

#include "likelihood_component_base.hpp"

namespace fims_likelihood {

template <typename Type>
struct InvGammaLikelihood : public LikelihoodComponentBase<Type> {
  fims::Vector<Type> log_shape;
  fims::Vector<Type> log_scale;

  std::string Name() const override { return "InvGammaLikelihood"; }

  Type GetLogShape(size_t i) { return log_shape.get_force_scalar(i); }
  Type GetLogScale(size_t i) { return log_scale.get_force_scalar(i); }

  void CheckParameterSizes(size_t n_input) const {
    if (log_shape.size() > 1 && log_shape.size() != n_input) {
      throw std::invalid_argument(
          "InvGammaLikelihood::Evaluate: input and log_shape must "
          "have the same size unless log_shape is scalar.");
    }
    if (log_scale.size() > 1 && log_scale.size() != n_input) {
      throw std::invalid_argument(
          "InvGammaLikelihood::Evaluate: input and log_scale must "
          "have the same size unless log_scale is scalar.");
    }
  }

  Type Evaluate() override {
    size_t n = this->GetInputSize();
    CheckParameterSizes(n);
    this->PrepareComponents(n);

    for (size_t i = 0; i < n; i++) {
      Type input = this->GetInput(i);
      if (input == this->na_value) {
        continue;
      }
      Type shape = fims_math::exp(GetLogShape(i));
      Type scale = fims_math::exp(GetLogScale(i));
      Type log_density = -shape * fims_math::log(scale) -
                         fims_math::lgamma(shape) -
                         (shape + static_cast<Type>(1.0)) *
                             fims_math::log(input) -
                         static_cast<Type>(1.0) / (scale * input);
      this->nll_components[i] = -log_density;
      this->nll += this->nll_components[i];
    }
    return this->nll;
  }
};

}  // namespace fims_likelihood

#endif /* FIMS_INVGAMMA_LIKELIHOOD_HPP */
