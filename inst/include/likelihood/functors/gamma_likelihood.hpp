/**
 * @file gamma_likelihood.hpp
 * @brief Negative log-likelihood component for gamma errors.
 */
#ifndef FIMS_GAMMA_LIKELIHOOD_HPP
#define FIMS_GAMMA_LIKELIHOOD_HPP

#include "normal_likelihood.hpp"

namespace fims_likelihood {

template <typename Type>
struct GammaLikelihood : public NormalLikelihood<Type> {
  std::string Name() const override { return "GammaLikelihood"; }

  Type Evaluate() override {
    size_t n = this->GetInputSize();
    this->CheckExpectedSize(n, this->Name());
    this->CheckLogSdSize(n);
    this->PrepareComponents(n);

    for (size_t i = 0; i < n; i++) {
      Type input = this->GetInput(i);
      if (input == this->na_value) {
        continue;
      }
      Type mean = this->GetExpected(i);
      Type sd = fims_math::exp(this->GetLogSd(i));
      Type shape = (mean / sd) * (mean / sd);
      Type scale = (sd * sd) / mean;
      Type log_density = (shape - static_cast<Type>(1.0)) *
                             fims_math::log(input) -
                         input / scale -
                         fims_math::lgamma(shape) -
                         shape * fims_math::log(scale);
      this->nll_components[i] = -log_density;
      this->nll += this->nll_components[i];
#ifdef TMB_MODEL
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {
          this->GetInput(i) = rgamma(shape, scale);
        }
      }
#endif
    }
    return this->nll;
  }
};

}  // namespace fims_likelihood

#endif /* FIMS_GAMMA_LIKELIHOOD_HPP */
