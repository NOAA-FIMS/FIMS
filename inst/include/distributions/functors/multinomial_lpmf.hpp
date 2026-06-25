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
 * row is skipped and contributes zero to the objective. Contributions are
 * stored in `lpdf_vec`. The summed total is returned by `evaluate()` and
 * stored in `lpdf`.
 *
 * Row observations could be counts of each age for a given time step, where
 * additional time steps would be additional rows. Thus, columns are bins.
 */
template <typename Type>
struct MultinomialLPMF : public DensityComponentBase<Type> {
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

  virtual std::string name() const override { return "MultinomialLPMF"; }

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
    // set dims using data_observed_values if no user input
    if (dims.size() != 2) {
      dims.resize(2);
      dims[0] = this->data_observed_values->get_imax();
      dims[1] = this->data_observed_values->get_jmax();
    }

    size_t n = dims[0] * dims[1];
    this->prepare_lpdf(n);
    size_t lpdf_vec_idx = 0; /**< index for lpdf_vec vector */
    // Dimension checks
    if (this->distribution_type ==
        fims_distributions::Distribution_Kind::DATA) {
      if (this->expected_ptr->size() > 0) {
        if (n != this->expected_ptr->size()) {
          throw std::invalid_argument(
              this->name() +
              ": Vector index out of bounds. The dimension of "
              "the number of rows times the number of columns is of size " +
              std::to_string(n) + " and the expected vector is of size " +
              std::to_string(this->expected_ptr->size()));
        }
      }
    } else {
      if (n != this->observed_ptr->size()) {
        throw std::invalid_argument(
            this->name() +
            ": Vector index out of bounds. The dimension of the "
            "number of rows times the number of columns is of size " +
            std::to_string(n) + " and the observed vector is of size " +
            std::to_string(this->observed_ptr->size()));
      }
      if (this->observed_ptr->size() != this->expected_ptr->size()) {
        throw std::invalid_argument(
            this->name() +
            ": Vector index out of bounds. The observed vector of size " +
            std::to_string(this->observed_ptr->size()) +
            " and the expected vector is of size " +
            std::to_string(this->expected_ptr->size()));
      }
    }

    for (size_t i = 0; i < dims[0]; i++) {
      // for each row, create new observed_values and prob vectors
      fims::Vector<Type> observed_values_vector;
      fims::Vector<Type> prob_vector;
      observed_values_vector.resize(dims[1]);
      prob_vector.resize(dims[1]);

      // Skips the entire row if any values are NA
      bool containsNA = false;

#ifdef TMB_MODEL
      for (size_t j = 0; j < dims[1]; j++) {
        if (this->distribution_type ==
            fims_distributions::Distribution_Kind::DATA) {
          // if data, check if there are any NA values and skip lpdf calculation
          // for entire row if there are
          if (this->get_observed(static_cast<size_t>(i),
                                 static_cast<size_t>(j)) ==
              this->data_observed_values->na_value) {
            containsNA = true;
            break;
          }
          if (!containsNA) {
            size_t idx = (i * dims[1]) + j;
            observed_values_vector[j] = this->get_observed(i, j);
            prob_vector[j] = this->get_expected(idx);
          }
        } else {
          // if not data (i.e. prior or process), use observed_values vector
          // instead of data_observed_values
          size_t idx = (i * dims[1]) + j;
          observed_values_vector[j] = this->get_observed(idx);
          prob_vector[j] = this->get_expected(idx);
        }
      }

      if (!containsNA) {
        std::fill(this->lpdf_vec.begin() + lpdf_vec_idx,
                  this->lpdf_vec.begin() + lpdf_vec_idx + dims[1],
                  dmultinom(observed_values_vector.to_tmb(),
                            prob_vector.to_tmb(), true));

        this->lpdf += this->lpdf_vec[lpdf_vec_idx];
      } else {
        this->lpdf_vec[i] = 0;
      }
      lpdf_vec_idx += dims[1];
/*
if (this->simulate_flag)
{
    FIMS_SIMULATE_F(this->of)
    {
        fims::Vector<Type> sim_observed;
        sim_observed.resize(dims[1]);
        sim_observed = rmultinom(prob_vector);
        sim_observed.resize(this->observed_values.size());
        for (size_t j = 0; j < dims[1]; j++)
        {
            idx = (i * dims[1]) + j;
            this->observed_values[idx] = sim_observed[j];
        }
    }
}
*/
#endif
    }

#ifdef TMB_MODEL
#endif
    return (this->lpdf);
  }
};
}  // namespace fims_distributions
#endif
