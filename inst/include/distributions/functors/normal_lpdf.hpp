/**
 * @file normal_lpdf.hpp
 * @brief Implements the NormalLPDF distribution functor used by FIMS to
 * evaluate observation-level and total log-likelihood contributions under a
 * normal error model for data, priors, and random effects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef NORMAL_LPDF
#define NORMAL_LPDF

#include "../../common/def.hpp"
#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
namespace fims_distributions {
/**
 * @copybrief normal_lpdf.hpp
 *
 * @details This implementation relies on [TMB's R-style `dnorm()` utility](
 * https://kaskr.github.io/adcomp/group__R__style__distribution.html) for
 * normal log-density calculations. Specifically, when evaluating the normal
 * likelihood, observations are passed to `dnorm(..., give_log = true)` to
 * obtain log-density values.
 *
 * For `data` input, values equal to `na_value` are skipped and contribute zero
 * to the objective. Per-observation contributions are stored in `lpdf_vec`;
 * the summed total is returned by `evaluate()` and stored in `lpdf`.
 */
template <typename Type>
struct NormalLPDF : public DensityComponentBase<Type> {

  /** @brief Constructor.
   */
  NormalLPDF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~NormalLPDF() {}

  /**
   * @brief Evaluates the normal log probability density function.
   * @details The following equation is normal probability density function,
   * and thus, the log of it evaluated:
   * \f[
   * f(x) =
   * \frac{1}{\sigma\sqrt{2\pi}}\mathrm{exp}\Bigg(-\frac{(x-\mu)^2}{2\sigma^2}
   * \Bigg), \f] where \f$\mu\f$ is the mean of the distribution and
   * \f$\sigma^2\f$ is the variance.
   */
  virtual const Type evaluate() {
    
    this->distribution_type = "normal";
    
    if (this->dims != NULL) {
      throw std::invalid_argument(
          "Dimensions specified for normal distribution, 
          but normal distribution is not multivariate. 
          Remove dimensions to evaluate.");
    }
    
    // extract evaluation vector size based on inputs 
    size_t n_evals = this->check_input_sizes();

    // setup vector for recording the log probability density function values
    this->lpdf_vec.resize(n_evals);

    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    
    this->lpdf = static_cast<Type>(0);
    
  //We should be able to remove this loop for non-OSA cases to speed up
  //evaluation with vectorized dnorm calls.
for (size_t i = 0; i < n_evals; i++) {
#ifdef TMB_MODEL
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) { // preprocessor definition in interface.hpp
                                    // this simulates data that is mean biased
          (*this->observed_pointer).set_observed(i,rnorm(this->get_expected(i), 
          fims_math::exp(this->get_uncertainty(i))));
          
        }
      }else{
        if (this->get_observed(i) != this->na_value &&
              this->get_expected(i) != this->na_value &&
              this->get_uncertainty(i) != this->na_value) {
                
                this->lpdf_vec[i] = this->get_lambda(i) * (
                dnorm(this->get_observed(i), this->get_expected(i),
                      fims_math::exp(this->get_uncertainty(i)), true));
        } else {
            this->lpdf_vec[i] = 0;
        }
        this->lpdf += this->lpdf_vec[i];
      }
#endif
    }
        /* osa not working yet
        if(osa_flag){//data observation type implements osa residuals
            //code for osa cdf method
            this->lpdf_vec[i] = this->keep.cdf_lower[i] * log(
        pnorm(this->observed_values[i], this->get_expected(i), sd[i]) );
        this->lpdf_vec[i] = this->keep.cdf_upper[i] * log( 1.0 -
        pnorm(this->observed_values[i], this->get_expected(i), sd[i]) );
        } */

#ifdef TMB_MODEL
    vector<Type> normal_observed_values = (*this->observed_pointer).to_tmb();
#endif
    return (this->lpdf);
  }
};

}  // namespace fims_distributions
#endif
