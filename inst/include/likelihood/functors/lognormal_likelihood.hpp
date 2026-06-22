/**
 * @file lognormal_likelihood.hpp
 * @brief Negative log-likelihood component for lognormal errors.
 */
#ifndef FIMS_LOGNORMAL_LIKELIHOOD_HPP
#define FIMS_LOGNORMAL_LIKELIHOOD_HPP

#include "normal_likelihood.hpp"

namespace fims_likelihood {

template <typename Type>
struct LognormalLikelihood : public NormalLikelihood<Type> {
  std::string Name() const override { return "LognormalLikelihood"; }

  Type Evaluate() override {
    size_t n = this->GetInputSize();
    this->CheckExpectedSize(n, this->Name());
    this->CheckLogSdSize(n);
    this->PrepareComponents(n);

    const Type log_two_pi = static_cast<Type>(1.8378770664093453);
    for (size_t i = 0; i < n; i++) {
      Type input = this->GetInput(i);
      if (input == this->na_value) {
        continue;
      }
      Type log_observed = fims_math::log(input);
      Type log_sigma = this->GetLogSd(i);
      Type sigma = fims_math::exp(log_sigma);
      Type residual = log_observed - this->GetExpected(i);
      this->nll_components[i] =
          static_cast<Type>(0.5) *
              (log_two_pi + static_cast<Type>(2.0) * log_sigma +
               (residual * residual) / (sigma * sigma)) +
          log_observed;
      this->nll += this->nll_components[i];
#ifdef TMB_MODEL
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {
          this->GetInput(i) =
              fims_math::exp(rnorm(this->GetExpected(i), sigma));
        }
      }
#endif
    }
    return this->nll;
  }
};

}  // namespace fims_likelihood

#endif /* FIMS_LOGNORMAL_LIKELIHOOD_HPP */
