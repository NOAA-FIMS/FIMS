/** \file tmb_distributions.hpp
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare distribution classes
 * which implement distribution functions from TMB.
 */
#ifndef DISTRIBUTIONS_TMB_DISTRIBUTIONS_HPP
#define DISTRIBUTIONS_TMB_DISTRIBUTIONS_HPP

#ifdef TMB_MODEL

#include "distributions_base.hpp"
#include "../../common/fims_vector.hpp"

namespace fims_distributions {

/**
 * @brief Dnorm class returns the TMB dnorm function
 */
template <typename Type>
struct Dnorm : public DistributionsBase<Type> {
  Type x;    /**< observation */
  Type mean; /**< mean of the normal distribution */
  Type sd; /**< standard deviation of the normal distribution, must be strictly
           positive.*/

  Dnorm() : DistributionsBase<Type>() {}

  virtual ~Dnorm() {}

  /**
   * @brief Probability density function of the normal distribution.
   *
   * \f[ \frac{1.0}{ sd\sqrt{2\pi} }exp(-\frac{(x - mean)^{2}}{2sd^{2}}) \f]
   *
   * @param do_log Boolean; if true, log densities are returned
   */
  virtual const Type evaluate(const bool& do_log) {
    return dnorm(x, mean, sd, do_log);
  }
};

/**
 * @brief Dmultinom class returns the TMB dmultinom function
 */
template <typename Type>
struct Dmultinom : public DistributionsBase<Type> {
  fims::Vector<Type> x; /*!< Vector of length K of integers */
  fims::Vector<Type> p; /*!< Vector of length K, specifying the probability for the K
               classes (note, unlike in R these must sum to 1). */

  Dmultinom() : DistributionsBase<Type>() {}

  /**
   * @brief Probability mass function of the multinomial distribution.
   *
   * \f[ \frac{n!}{x_{1}! \dots x_{K}!}p_{1}^{x_{1}} \dots p_{K}^{x_{K}} \text{,
   * } x_{i} \in \{0,...,n\}, i \in \{1,...,K\}, \text{ with } \sum_{i}x_{i} = n
   * \text{ and } \sum^{K}_{k=1}p_{k}=1 \f]
   *
   * @param do_log Boolean; if true, log densities are returned
   */
  virtual const Type evaluate(const bool& do_log) {
    return dmultinom<Type>(x, p, do_log);
  }
};

/**
 * @brief Dlnorm uses the TMB dnorm function to construct the lognormal density
 * function
 */
template <typename Type>
struct Dlnorm : public DistributionsBase<Type> {
  Type x;       /**< observation */
  Type meanlog; /**< mean of the distribution of log(x) */
  Type sdlog;   /**< standard deviation of the distribution of log(x) */

  Dlnorm() : DistributionsBase<Type>() {}

  /**
   * @brief Probability density function of the lognormal distribution.
   *
   * \f[ \frac{1.0}{ xsd\sqrt{2.0\pi} }exp(-\frac{(ln(x) -
   * mean)^{2.0}}{2.0sd^{2.0}}) \f]
   *
   * @param do_log Boolean; if true, log densities are returned
   */

  virtual const Type evaluate(const bool& do_log) {
    Type logx = log(x);
    Type nll;

    nll = dnorm(logx, meanlog, sdlog, true) - logx;

    if (do_log) {
      return nll;
    } else {
      return exp(nll);
    }
  }
};
}  // namespace fims_distributions

#endif

#endif /* DISTRIBUTIONS_TMB_DISTRIBUTIONS_HPP */
