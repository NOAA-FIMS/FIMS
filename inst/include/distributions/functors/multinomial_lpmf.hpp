/**
 * @file multinomial_lpmf.hpp
 * @brief Implements the MultinomialLPMF distribution functor used by FIMS to
 * evaluate the observation-level and total log-likelihood contributions under
 * a multinomial error model for data, priors, and random effects.
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
 * @copybrief multinomial_lpmf.hpp
 *
 * @details This implementation relies on [TMB's R-style `dmultinom()`
 * utility]( https://kaskr.github.io/adcomp/group__R__style__distribution.html)
 * to compute row-wise multinomial log-probability mass contributions from
 * observed counts (`x_vector`) and expected proportions (`prob_vector`).
 * Specifically, when evaluating the multinomial likelihood, observations are
 * passed to `dmultinom(..., give_log = true)`.
 *
 * For `data` input, if any element in a row is equal to `na_value`, the entire
 * row is skipped and contributes zero to the objective. Row-level
 * contributions are stored in `lpdf_vec`; values are expanded into
 * `report_lpdf_vec` for output consistency with the full matrix dimensions.
 * The summed total is returned by `evaluate()` and stored in `lpdf`.
 *
 * Row observations could be counts of each age for a given time step, where
 * additional time steps would be additional rows. Thus, columns are bins.
 */
template <typename Type>
struct MultinomialLPMF : public DensityComponentBase<Type> {
  /**
   * @brief Total log-likelihood contribution of the distribution.
   */
  Type lpdf = static_cast<Type>(0.0);

  /**
   * @brief Dimensions of the number of rows and columns of the multivariate
   * dataset.
   */
  fims::Vector<size_t> dims;

  /** @brief Constructor.
   */
  MultinomialLPMF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~MultinomialLPMF() {}

  /**
   * @brief Evaluates the multinomial log probability mass function.
   * @details The following equation is the multinomial probability mass
   * function, and thus, the log of it is evaluated:
   * \f[
   * f(\underline{y}) = \frac{n!}{y_{1}!...
   * y_{k}!}p^{y_{1}}_{1}...p^{y_{k}}_{k}, \f] where \f$k\f$ is the number of
   * categories, \f$n\f$ is the sample size, \f$\mu_{i}\f$ is the mean of
   * \f$y_{i}\f$ and is equal to \f$np_{i}\f$, and \f$\sigma^{2}_{i}\f$ is the
   * variance of \f$y_{i}\f$ and is equal to \f$np_{i}(1-p_{i})\f$.
   */
  virtual const Type evaluate() {
    // set dims using observed_values if no user input
    if (dims.size() != 2) {
      dims.resize(2);
      dims[0] = this->observed_values->get_imax();
      dims[1] = this->observed_values->get_jmax();
    }

    // setup vector for recording row-level log probability mass values
    // total log probability mass contribution of the distribution
    Type lpdf = static_cast<Type>(0.0);
    this->lpdf_vec.resize(dims[0]);
    this->report_lpdf_vec.clear();
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), 0);

    // Dimension checks
    if (this->input_type == "data") {
      if (this->data_expected_values) {
        if (dims[0] * dims[1] != this->data_expected_values->size()) {
          throw std::invalid_argument(
              "MultinomialLPDF: Vector index out of bounds. The dimension of "
              "the "
              "number of rows times the number of columns is of size " +
              fims::to_string(dims[0] * dims[1]) +
              " and the expected vector is of size " +
              fims::to_string(this->data_expected_values->size()));
        }
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

      // Skips the entire row if any values are NA
      bool containsNA = false;

#ifdef TMB_MODEL
      for (size_t j = 0; j < dims[1]; j++) {
        if (this->input_type == "data") {
          // if data, check if there are any NA values and skip lpdf calculation
          // for entire row if there are
          if (this->get_observed(static_cast<size_t>(i),
                                 static_cast<size_t>(j)) ==
              this->observed_values->na_value) {
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
            dmultinom(x_vector.to_tmb(), prob_vector.to_tmb(), true);
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
