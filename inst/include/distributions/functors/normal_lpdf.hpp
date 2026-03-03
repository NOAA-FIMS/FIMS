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

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

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
  /**
   * @brief The natural log of the standard deviation of the distribution. The
   * argument can be a vector or scalar, where the latter is referenced for
   * each instance through the use of
   * \ref fims::Vector::get_force_scalar(size_t) "get_force_scalar()".
   */
  fims::Vector<Type> log_sd;

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
    // set vector size based on input type (prior, process, or data)
    size_t n_x = this->get_n_x();
    // get expected value vector size
    size_t n_expected = this->get_n_expected();
    // setup vector for recording the log probability density function values
    this->lpdf_vec.resize(n_x);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    this->lpdf = static_cast<Type>(0);

    // Dimension checks
    if (n_x != n_expected) {
      if (n_expected == 1) {
        n_expected = n_x;
      } else if (n_x > n_expected) {
        n_x = n_expected;
      }
    }

    if (this->log_sd.size() > 1 && n_x != this->log_sd.size()) {
      throw std::invalid_argument(
          "NormalLPDF::Vector index out of bounds. The size of observed data "
          "does not equal the size of the log_sd vector. The observed data "
          "vector is of size " +
          fims::to_string(n_x) + " and the log_sd vector is of size " +
          fims::to_string(this->log_sd.size()));
    }

    for (size_t i = 0; i < n_x; i++) {
#ifdef TMB_MODEL
      if (this->input_type == "data") {
        // if data, check if there are any NA values and skip lpdf calculation
        // if there are
        if (this->get_observed(i) != this->data_observed_values->na_value) {
          this->lpdf_vec[i] =
              dnorm(this->get_observed(i), this->get_expected(i),
                    fims_math::exp(log_sd.get_force_scalar(i)), true);
        } else {
          this->lpdf_vec[i] = 0;
        }
        // if not data (i.e. prior or process), use x vector instead of
        // observed_values
      } else {
        this->lpdf_vec[i] =
            dnorm(this->get_observed(i), this->get_expected(i),
                  fims_math::exp(log_sd.get_force_scalar(i)), true);
      }
      this->lpdf += this->lpdf_vec[i];
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {
          if (this->input_type == "data") {
            this->data_observed_values->at(i) =
                rnorm(this->get_expected(i),
                      fims_math::exp(log_sd.get_force_scalar(i)));
          }
          if (this->input_type == "random_effects") {
            (*this->re)[i] = rnorm(this->get_expected(i),
                                   fims_math::exp(log_sd.get_force_scalar(i)));
          }
          if (this->input_type == "prior") {
            (*(this->priors[i]))[0] =
                rnorm(this->get_expected(i),
                      fims_math::exp(log_sd.get_force_scalar(i)));
          }
        }
      }
#endif
      /* osa not working yet
        if(osa_flag){//data observation type implements osa residuals
            //code for osa cdf method
            this->lpdf_vec[i] = this->keep.cdf_lower[i] * log( pnorm(this->observed_values[i],
        this->get_expected(i), sd[i]) ); this->lpdf_vec[i] =
        this->keep.cdf_upper[i] * log( 1.0 - pnorm(this->observed_values[i],
        this->get_expected(i), sd[i]) );
        } */
    }
#ifdef TMB_MODEL
    vector<Type> normal_observed_values = this->observed_values.to_tmb();
#endif
    return (this->lpdf);
  }
};

}  // namespace fims_distributions
#endif
