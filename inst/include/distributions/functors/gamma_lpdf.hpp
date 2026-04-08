/**
 * @file gamma_lpdf.hpp
 * @brief Gamma Log Probability Density Function (LPDF) module file defines
 * the Gamma LPDF class and its fields and returns the log probability density
 * function.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef GAMMA_LPDF
#define GAMMA_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions {
/**
 * @copybrief gamma_lpdf.hpp
 *
 * @details This implementation relies on [TMB's R-style `dgamma()` utility](
 * https://kaskr.github.io/adcomp/group__R__style__distribution.html) for
 * Gamma log-density calculations. Specifically, when evaluating the Gamma
 * likelihood, observations are passed to `dgamma(..., give_log = true)` to
 * obtain log-density values.
 *
 * For `data` input, values equal to `na_value` are skipped and contribute zero
 * to the objective. Per-observation contributions are stored in `lpdf_vec`;
 * the summed total is returned by `evaluate()` and stored in `lpdf`.
 */
template <typename Type>
struct GammaLPDF : public DensityComponentBase<Type> {
  /**
   * @brief The natural log of the standard deviation of the distribution. The
   * argument can be a vector or scalar, where the latter is referenced for
   * each instance through the use of
   * \ref fims::Vector::get_force_scalar(size_t) "get_force_scalar()".
   */
  fims::Vector<Type> log_sd;

  /** @brief Constructor.
   */
  GammaLPDF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~GammaLPDF() {}

 /**
   * @brief Evaluates the Gamma log probability density function.
   * @details The following equation is Gamma probability density function:
   * \f[
   * f(x) =
   * \frac{1}{\Gamma(\alpha)\theta^{\alpha}}y^{\alpha-1}\mathrm{exp}
   * \Bigg(-\frac{y}{\theta}\Bigg), \f] where \f$\alpha\theta\f$ is the 
   * mean of the distribution and \f$\alpha\theta^{2}\f$ is the variance.
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
    if (n_expected > 1 && n_expected != n_x) {
      throw std::invalid_argument(
        "GammaLPDF::Vector index out of bounds. The size of observed "
            "data does not equal the expected size. The observed data vector "
            "is of size " +
            fims::to_string(n_x) +
            " and the expected size is " + fims::to_string(n_expected));
    }

    if (this->log_sd.size() > 1 && n_x != this->log_sd.size()) {
      throw std::invalid_argument(
          "GammaLPDF::Vector index out of bounds. The size of observed data "
          "does not equal the size of the log_sd vector. The observed data "
          "vector is of size " +
          fims::to_string(n_x) + " and the log_sd vector is of size " +
          fims::to_string(this->log_sd.size()));
    }

    for (size_t i = 0; i < n_x; i++) {
#ifdef TMB_MODEL
      // Calculate shape and scale parameters from mean (expected value) and standard deviation
      // shape = (mean/sd)^2
      // scale = sd^2/mean
      Type mean_val = this->get_expected(i);
      Type sd_val = fims_math::exp(log_sd.get_force_scalar(i));
      Type shape = (mean_val / sd_val) * (mean_val / sd_val);
      Type scale = (sd_val * sd_val) / mean_val;

      if (this->input_type == "data") {
        // if data, check if there are any NA values and skip lpdf calculation
        // if there are
        if (this->get_observed(i) != this->data_observed_values->na_value) {
          this->lpdf_vec[i] =
              dgamma(this->get_observed(i), shape, scale, true);
        } else {
          this->lpdf_vec[i] = 0;
        }
        // if not data (i.e. prior or process), use x vector instead of
        // observed_values
      } else {
        // TODO: hard coded for now but need to address NA values when observed value is derived from data
        if(this->get_observed(i) != -999){
          this->lpdf_vec[i] =
              dgamma(this->get_observed(i), shape, scale, true);
        }
      }
      this->lpdf += this->lpdf_vec[i];
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {
          if (this->input_type == "data") {
            this->data_observed_values->at(i) =
                rgamma(shape, scale);
          }
          if (this->input_type == "random_effects") {
            (*this->re)[i] = rgamma(shape, scale);
          }
          if (this->input_type == "prior") {
            (*(this->priors[i]))[0] =
                rgamma(shape, scale);
          }
        }
      }
#endif
    }
#ifdef TMB_MODEL
    vector<Type> gamma_observed_values = this->observed_values.to_tmb();
#endif
    return (this->lpdf);
  }
};

}  // namespace fims_distributions
#endif
