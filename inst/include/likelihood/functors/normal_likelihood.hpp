/**
 * @file normal_likelihood.hpp
 * @brief Negative log-likelihood component for normal errors.
 */
#ifndef FIMS_NORMAL_LIKELIHOOD_HPP
#define FIMS_NORMAL_LIKELIHOOD_HPP

#include "likelihood_component_base.hpp"

namespace fims_likelihood {

template <typename Type>
struct NormalLikelihood : public LikelihoodComponentBase<Type> {
  fims::Vector<Type> log_sd;

  std::string Name() const override { return "NormalLikelihood"; }

  Type GetLogSd(size_t i) { return log_sd.get_force_scalar(i); }

  void CheckLogSdSize(size_t n_input) const {
    if (log_sd.size() > 1 && log_sd.size() != n_input) {
      throw std::invalid_argument(
          "NormalLikelihood::Evaluate: input and log_sd must have the same "
          "size unless log_sd is scalar. input size is " +
          fims::to_string(n_input) + " and log_sd size is " +
          fims::to_string(log_sd.size()));
    }
  }

  Type Evaluate() override {
    size_t n = this->GetInputSize();
    this->CheckExpectedSize(n, Name());
    CheckLogSdSize(n);
    this->PrepareComponents(n);

    const Type log_two_pi = static_cast<Type>(1.8378770664093453);
    for (size_t i = 0; i < n; i++) {
      Type input = this->GetInput(i);
      if (input == this->na_value) {
        continue;
      }
      Type log_sigma = GetLogSd(i);
      Type sigma = fims_math::exp(log_sigma);
      Type residual = input - this->GetExpected(i);
      this->nll_components[i] =
          static_cast<Type>(0.5) *
          (log_two_pi + static_cast<Type>(2.0) * log_sigma +
           (residual * residual) / (sigma * sigma));
      this->nll += this->nll_components[i];
#ifdef TMB_MODEL
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {
          this->GetInput(i) = rnorm(this->GetExpected(i), sigma);
        }
      }
#endif
    }
    return this->nll;
  }
};

}  // namespace fims_likelihood

#endif /* FIMS_NORMAL_LIKELIHOOD_HPP */
