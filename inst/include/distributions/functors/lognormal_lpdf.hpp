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
#include "../../common/def.hpp"

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
  fims::Vector<Type> log_sd;

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
    // TODO: fix dimension check as expected values no longer used for data
    if (n_x != n_expected) {
      if (n_expected == 1) {
        n_expected = n_x;
      } else if (n_x > n_expected) {
        n_x = n_expected;
      }
    }

    if (this->log_sd.size() > 1 && n_x != this->log_sd.size()) {
      throw std::invalid_argument(
          "LognormalLPDF::Vector index out of bounds. The size of observed "
          "data does not equal the size of the log_sd vector. The observed "
          "data vector is of size " +
          fims::to_string(n_x) + " and the log_sd vector is of size " +
          fims::to_string(this->log_sd.size()));
    }

    for (size_t i = 0; i < n_x; i++) {
#ifdef TMB_MODEL
      if (this->input_type == "data") {
        // if data, check if there are any NA values and skip lpdf calculation
        // if there are See Deroba and Miller, 2016
        // (https://doi.org/10.1016/j.fishres.2015.12.002) for the use of
        // lognormal constant
        if (this->get_observed(i) != this->data_observed_values->na_value) {
          this->lpdf_vec[i] =
              dnorm(log(this->get_observed(i)), this->get_expected(i),
                    fims_math::exp(log_sd.get_force_scalar(i)), true) -
              log(this->get_observed(i));
        } else {
          this->lpdf_vec[i] = 0;
        }
      } else {
        if (this->input_type == "random_effects") {
          // if random effects, no lognormal constant needs to be applied
          this->lpdf_vec[i] =
              dnorm(log(this->get_observed(i)), this->get_expected(i),
                    fims_math::exp(log_sd.get_force_scalar(i)), true);
        } else {
          this->lpdf_vec[i] =
              dnorm(log(this->get_observed(i)), this->get_expected(i),
                    fims_math::exp(log_sd.get_force_scalar(i)), true) -
              log(this->get_observed(i));
        }
      }

      this->lpdf += this->lpdf_vec[i];
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {  // preprocessor definition in interface.hpp
                                     // this simulates data that is mean biased
          if (this->input_type == "data") {
            this->data_observed_values->at(i) = fims_math::exp(
                rnorm(this->get_expected(i),
                      fims_math::exp(log_sd.get_force_scalar(i))));
          }
          if (this->input_type == "random_effects") {
            (*this->re)[i] = fims_math::exp(
                rnorm(this->get_expected(i),
                      fims_math::exp(log_sd.get_force_scalar(i))));
          }
          if (this->input_type == "prior") {
            (*(this->priors[i]))[0] = fims_math::exp(
                rnorm(this->get_expected(i),
                      fims_math::exp(log_sd.get_force_scalar(i))));
          }
        }
      }
#endif
    }
#ifdef TMB_MODEL
    vector<Type> lognormal_observed_values = this->observed_values.to_tmb();
    //  FIMS_REPORT_F(lognormal_observed_values, this->of);
#endif
    return (this->lpdf);
  }
};
}  // namespace fims_distributions
#endif
