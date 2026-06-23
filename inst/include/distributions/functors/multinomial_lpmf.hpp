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
    
    this->distribution_type = "multinomial";

    // Throw an error if dims not specified. TODO: Setup automatic dimension 
    // setup based on the parameter pointers passed to the distribution.
    if (dims.size() != 2) {
      throw std::invalid_argument(
          "No dimensions specified for multinomial distribution");
    }

    // extract evaluation vector size based on inputs 
    size_t n_evals = this->check_input_sizes();

    // setup vector for recording the log probability density function values
    this->lpdf = static_cast<Type>(0.0); /**< total log probability mass
                                           contribution of the distribution */
    this->lpdf_vec.resize(dims[0] * dims[1]);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), 0);
    size_t lpdf_vec_idx = 0; /**< index for lpdf_vec vector */
    
    // Create new observed_values and prob vectors
    fims::Vector<Type> observed_values_vector;
    fims::Vector<Type> prob_vector;
    observed_values_vector.resize(dims[1]);
    prob_vector.resize(dims[1]);

    for (size_t i = 0; i < dims[0]; i++) {
#ifdef TMB_MODEL
      // Skips the entire row if any values are NA
      bool containsNA = false;
      for (size_t j = 0; j < dims[1]; j++) {
        
        size_t idx = (i * dims[1]) + j;
        
        if (this->get_observed(idx) != this->na_value &&
              this->get_expected(idx) != this->na_value) {
          observed_values_vector[j] = this->get_observed(idx);
          prob_vector[j] = this->get_expected(idx);     
        }else{
          containsNA = true;
          break;
        }
      }

      if (!containsNA) {
        std::fill(this->lpdf_vec.begin() + lpdf_vec_idx,
                  this->lpdf_vec.begin() + lpdf_vec_idx + dims[1],
                  this->get_lambda(i) * 
                  (dmultinom(observed_values_vector.to_tmb(),
                            prob_vector.to_tmb(), true)));

        this->lpdf += this->lpdf_vec[lpdf_vec_idx];
      } else {
        std::fill(this->lpdf_vec.begin() + lpdf_vec_idx,
                  this->lpdf_vec.begin() + lpdf_vec_idx + dims[1], 0);
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
