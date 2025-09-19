/**
 * @file multinomial_lpmf.hpp
 * @brief Multinomial Log Probability Mass Function (LPMF) module file defines
 * the Multinomial LPMF class and its fields and returns the log probability
 * mass function.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef MULTINOMIAL_LPMF
#define MULTINOMIAL_LPMF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions {
/**
 * Multinomial Log Probability Mass Function
 */
template <typename Type>
struct MultinomialLPMF : public DensityComponentBase<Type> {
  Type lpdf = static_cast<Type>(0.0); /**< total negative log-likelihood
                                         contribution of the distribution */
  fims::Vector<size_t> dims; /**< Dimensions of the number of rows and columns
                                of the multivariate dataset */

  /** @brief Constructor.
   */
  MultinomialLPMF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~MultinomialLPMF() {}

  /**
   * @brief Evaluates the multinomial probability mass function.
   * 
   * For \f$k\f$ categories and a sample size of \f$n\f$,
   * \f[f(\underline{y}) = \frac{n!}{y_{1}!...
   * y_{k}!}p^{y_{1}}_{1}...p^{y_{k}}_{k}\f]
   * where \f$\sum^{k}_{i=1}y_{i} = n\f$, \f$p_{i} > 0\f$, and
   * \f$\sum^{k}_{i=1}p_{i} = 1\f$.
   * The mean and variance of \f$y_{i}\f$ are respectively, \f$\mu_{i} =
   * np_{i}\f$ and \f$\sigma^{2}_{i} = np_{i}(1-p_{i})\f$. The implementation
   * of this function is through TMB and its `dmultinom` function.
   */
  virtual const Type evaluate() {
    // set dims using observed_values if no user input
    if (dims.size() != 2) {
      dims.resize(2);
      dims[0] = this->observed_values->get_imax();
      dims[1] = this->observed_values->get_jmax();
    }

    // setup vector for recording the log probability density function values
    Type lpdf = static_cast<Type>(0.0); /**< total log probability mass
                                           contribution of the distribution */
    this->lpdf_vec.resize(dims[0]);
    this->report_lpdf_vec.clear();
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), 0);

    // Dimension checks
    if (this->input_type == "data") {
      if (dims[0] * dims[1] != this->expected_values.size()) {
        throw std::invalid_argument(
            "MultinomialLPDF: Vector index out of bounds. The dimension of the "
            "number of rows times the number of columns is of size " +
            fims::to_string(dims[0] * dims[1]) +
            " and the expected vector is of size " +
            fims::to_string(this->expected_values.size()));
      }
    } else {
      if (dims[0] * dims[1] != this->x.size()) {
        throw std::invalid_argument(
            "MultinomialLPDF: Vector index out of bounds. The dimension of the "
            "number of  rows times the number of columns is of size " +
            fims::to_string(dims[0] * dims[1]) +
            " and the observed vector is of size " +
            fims::to_string(this->x.size()));
      }
      if (this->x.size() != this->expected_values.size()) {
        throw std::invalid_argument(
            "MultinomialLPDF: Vector index out of bounds. The dimension of the "
            "observed vector of size " +
            fims::to_string(this->x.size()) +
            " and the expected vector is of size " +
            fims::to_string(this->expected_values.size()));
      }
    }

    for (size_t i = 0; i < dims[0]; i++) {
      // for each row, create new x and prob vectors
      fims::Vector<Type> x_vector;
      fims::Vector<Type> prob_vector;
      x_vector.resize(dims[1]);
      prob_vector.resize(dims[1]);

      bool containsNA = false; /**< skips the entire row if any values are NA */

#ifdef TMB_MODEL
      for (size_t j = 0; j < dims[1]; j++) {
        if (this->input_type == "data") {
          // if data, check if there are any NA values and skip lpdf calculation
          // for entire row if there are
          if (this->get_observed(i, j) == this->observed_values->na_value) {
            containsNA = true;
            break;
          }
          if (!containsNA) {
            size_t idx = (i * dims[1]) + j;
            x_vector[j] = this->get_observed(i, j);
            prob_vector[j] = this->get_expected(idx);
          }
        } else {
          // if not data (i.e. prior or process), use x vector instead of
          // observed_values
          size_t idx = (i * dims[1]) + j;
          x_vector[j] = this->get_observed(idx);
          prob_vector[j] = this->get_expected(idx);
        }
      }

      if (!containsNA) {
        this->lpdf_vec[i] =
            dmultinom((vector<Type>)x_vector, (vector<Type>)prob_vector, true);

      } else {
        this->lpdf_vec[i] = 0;
      }
      // track the values for output, e.g., report_lpdf_vec
      this->report_lpdf_vec.insert(this->report_lpdf_vec.end(), dims[1],
                                   this->lpdf_vec[i]);
      lpdf += this->lpdf_vec[i];
/*
if (this->simulate_flag)
{
    FIMS_SIMULATE_F(this->of)
    {
        fims::Vector<Type> sim_observed;
        sim_observed.resize(dims[1]);
        sim_observed = rmultinom(prob_vector);
        sim_observed.resize(this->x);
        for (size_t j = 0; j < dims[1]; j++)
        {
            idx = (i * dims[1]) + j;
            this->x[idx] = sim_observed[j];
        }
    }
}
*/
#endif
    }

#ifdef TMB_MODEL
#endif
    this->lpdf = lpdf;
    return (lpdf);
  }
};
}  // namespace fims_distributions
#endif
