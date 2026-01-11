/**
 * @file normal_lpdf.hpp
 * @brief Normal Log Probability Density Function (LPDF) module file defines
 * the Normal LPDF class and its fields and returns the log probability density
 * function.
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
 * Normal Log Probability Density Function
 */
template <typename Type>
struct NormalLPDF : public DensityComponentBase<Type> {
  fims::Vector<Type> log_sd; /**< the natural log of the standard deviation of
                                the distribution; can be a vector or scalar */
  Type lpdf = static_cast<Type>(0.0); /**< total log probability density
                                         contribution of the distribution */

  /** @brief Constructor.
   */
  NormalLPDF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~NormalLPDF() {}

  /**
   * @brief Evaluates the normal probability density function
   */
  virtual const Type evaluate() {
    // set vector size based on input type (prior, process, or data)
    size_t n_x = this->get_n_x();
    // get expected value vector size
    size_t n_expected = this->get_n_expected();
    // setup vector for recording the log probability density function values
    this->lpdf_vec.resize(n_x);
    this->report_lpdf_vec.resize(n_x);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    std::fill(this->report_lpdf_vec.begin(), this->report_lpdf_vec.end(),
              static_cast<Type>(0));
    lpdf = static_cast<Type>(0);

    // Dimension checks
    if (n_x != n_expected) {
      if (n_expected == 1) {
        n_expected = n_x;
      } else if (n_x > n_expected) {
        n_x = n_expected;
      }
    }
    // if(n_x != n_expected){
    //   throw std::invalid_argument(
    //       "LognormalLPDF::Vector index out of bounds. The size of observed "
    //       "data does not equal the size of expected values. The observed data
    //       " "vector is of size " +
    //         fims::to_string(n_x) + " and the expected vector is of size " +
    //         fims::to_string(n_expected) + ". with input type " +
    //         fims::to_string(this->input_type) + ". Final data value = " +
    //         fims::to_string(this->get_observed(n_x - 1)) +
    //         " and the final expected value = " +
    //         fims::to_string(this->get_expected(n_expected - 1)));
    // }
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
        if (this->get_observed(i) != this->observed_values->na_value) {
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
      this->report_lpdf_vec[i] = this->lpdf_vec[i];
      lpdf += this->lpdf_vec[i];
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {
          if (this->input_type == "data") {
            this->observed_values->at(i) =
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
            this->lpdf_vec[i] = this->keep.cdf_lower[i] * log( pnorm(this->x[i],
        this->get_expected(i), sd[i]) ); this->lpdf_vec[i] =
        this->keep.cdf_upper[i] * log( 1.0 - pnorm(this->x[i],
        this->get_expected(i), sd[i]) );
        } */
    }
#ifdef TMB_MODEL
    vector<Type> normal_x = this->x.to_tmb();
#endif
    return (lpdf);
  }
};

}  // namespace fims_distributions
#endif
