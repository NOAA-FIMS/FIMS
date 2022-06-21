/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare likelihood classes
 * which implement likelihood functions from TMB.
 */
  #ifndef LIKELIHOODS_TMB_LIKELIHOODS_HPP
  #define LIKELIHOODS_TMB_LIKELIHOODS_HPP

  #ifdef TMB_MODEL

  #include "likelihoods_base.hpp"

  namespace fims {

/**
 * @brief Dnorm class returns the TMB dnorm function
 */
  template <typename T>
  struct Dnorm : public LikelihoodsBase<T> {

    T x; /*!< observation */
    T mean; /*!< mean of the normal distribution */
    T sd; /*!< standard deviation of the normal distribution, must be strictly positive.*/


    Dnorm() : LikelihoodsBase<T>() {}

    /**
     * @brief Probability mass function of the normal distribution.
     *
     * \f[ \frac{1.0}{ sd\sqrt{2\pi} }exp(-\frac{(x - mean)^{2}}{2sd^{2}}) \f]
     * 
     * @param do_log Boolean; if true, log-likelihoods are returned
     */
    virtual const T evaluate(const bool& do_log) {
      return dnorm(x, mean, sd, do_log);
    }
  };

/**
 * @brief Dmultinom class returns the TMB dmultinom function
 */
template <typename T>
  struct Dmultinom : public LikelihoodsBase<T> {

    /** EigenVector defined in interface.hpp */
    using Vector = typename ModelTraits<T>::EigenVector;
    Vector x; /*!< Vector of length K of integers */
    Vector p; /*!< Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1). */


    Dmultinom() : LikelihoodsBase<T>() {}

    /**
     * @brief Probability mass function of the multinomial distribution.
     * 
     * \f[ \frac{n!}{x_{1}! \dots x_{K}!}p_{1}^{x_{1}} \dots p_{K}^{x_{K}} \text{, }
     * x_{i} \in \{0,...,n\}, i \in \{1,...,K\}, 
     * \text{ with } \sum_{i}x_{i} = n 
     * \text{ and } \sum^{K}_{k=1}p_{k}=1 \f]
     * 
     * @param do_log Boolean; if true, log-likelihoods are returned
     */
      virtual const T evaluate(const bool& do_log) {
      return dmultinom(x, p, do_log);
    }
  };

/**
 * @brief Dlnorm uses the TMB dnorm function to construct the lognormal density function
 */
template <typename T>
  struct Dlnorm : public LikelihoodsBase<T> {

    T x; /*!< observation */
    T meanlog; /*!< mean of the distribution of log(x) */
    T sdlog; /*!< standard deviation of the distribution of log(x) */

    Dlnorm() : LikelihoodsBase<T>() {}

    /**
     * @brief Probability mass function of the lognormal distribution.
     *
     * \f[ \frac{1.0}{ xsd\sqrt{2\pi} }exp(-\frac{(ln(x) - mean)^{2}}{2sd^{2}}) \f]
     * 
     * @param do_log Boolean, if true returns the log of the likelihood
     */

    virtual const T evaluate(const bool& do_log) {
      T logx = log(x);
      T nll = dnorm(logx, meanlog, sdlog, true) - logx;
      if(do_log) {
        return nll;
      } else {
        return exp(nll);
      }
    }
  };
}

#endif

#endif /* LIKELIHOODS_TMB_LIKELIHOODS_HPP */
