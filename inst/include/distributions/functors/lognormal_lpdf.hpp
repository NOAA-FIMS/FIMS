/**
 * @file lognormal_lpdf.hpp
 * @brief Implements the LogNormalLPDF distribution functor used by FIMS to
 * evaluate observation-level and total log-likelihood contributions under a
 * lognormal error model for data, priors, and random effects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef LOGNORMAL_LPDF
#define LOGNORMAL_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"

namespace fims_distributions {
/**
 * @copybrief lognormal_lpdf.hpp
 *
 * @details This implementation relies on [TMB's R-style `dnorm()` utility](
 * https://kaskr.github.io/adcomp/group__R__style__distribution.html) for
 * normal log-density calculations on log-transformed values. Specifically,
 * when evaluating the lognormal likelihood, observations are transformed with
 * `log(x)` and passed to `dnorm(..., give_log = true)` to obtain log-density
 * values. For data inputs, the Jacobian adjustment `-log(x)` is applied where
 * appropriate to convert from normal density on the log scale to the lognormal
 * density on the original scale.
 *
 * For `data` input, values equal to `na_value` are skipped and contribute zero
 * to the objective. Per-observation contributions are stored in `lpdf_vec`;
 * the summed total is returned by `evaluate()` and stored in `lpdf`.
 */
template <typename Type>
struct LogNormalLPDF : public DensityComponentBase<Type> {
  /**
   * @brief Natural log of the standard deviation of the distribution on the
   * log scale. The argument can be a vector or scalar, where the latter is
   * referenced for each instance through the use of
   * \ref fims::Vector::get_force_scalar(size_t) "get_force_scalar()".
   */

  /** @brief Constructor.
   */
  LogNormalLPDF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~LogNormalLPDF() {}

  /**
   * @brief Evaluates the lognormal log probability density function.
   * @details The following equation is the lognormal probability density
   * function, and thus, the log of it is evaluated:
   * \f[
   * f(x) = \frac{1.0}{ x\sigma\sqrt{2\pi}
   * }\mathrm{exp}\Bigg(-\frac{(\mathrm{ln}(x) - \mu)^{2}}{2\sigma^{2}}\Bigg),
   * \f]
   * where \f$\mu\f$ is the mean of the distribution of \f$\mathrm{ln(x)}\f$
   * and \f$\sigma^2\f$ is the variance of \f$\mathrm{ln}(x)\f$.
   */
  virtual const Type evaluate() {
    
    this->distribution_type = "lognormal";

    if (this->dims != NULL) {
      throw std::invalid_argument(
          "Dimensions specified for lognormal distribution, 
          but lognormal distribution is not multivariate. 
          Remove dimensions to evaluate.");
    }
    
    // extract evaluation vector size based on inputs 
    size_t n_evals = this->check_input_sizes();

    // setup vector for recording the log probability density function values
    this->lpdf_vec.resize(n_evals);

    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    
    this->lpdf = static_cast<Type>(0);


    for (size_t i = 0; i < n_evals; i++) {
#ifdef TMB_MODEL
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) { // preprocessor definition in interface.hpp
                                    // this simulates data that is mean biased
          (*this->observed_pointer).set_observed(i,
          fims_math::exp(rnorm(this->get_expected(i), 
          fims_math::exp(this->get_uncertainty(i)))));
          
        }
      }else{
        // Skip any evaluations with missing values.
        if (this->get_observed(i) != this->na_value &&
              this->get_expected(i) != this->na_value &&
              this->get_uncertainty(i) != this->na_value) {
                // See Deroba and Miller, 2016
                // (https://doi.org/10.1016/j.fishres.2015.12.002) for the use of
                // lognormal constant for bias correction.
                this->lpdf_vec[i] = this->get_lambda(i) * (
                dnorm(log(this->get_observed(i)), this->get_expected(i),
                      fims_math::exp(this->get_uncertainty(i)), true) -
                log(this->get_observed(i)));
        } else {
            this->lpdf_vec[i] = 0;
        }
        this->lpdf += this->lpdf_vec[i];
      }
#endif
    }
#ifdef TMB_MODEL
    vector<Type> lognormal_observed_values = (*this->observed_pointer).to_tmb();
    //  FIMS_REPORT_F(lognormal_observed_values, this->of);
#endif
    return (this->lpdf);
  }
};
}  // namespace fims_distributions
#endif
