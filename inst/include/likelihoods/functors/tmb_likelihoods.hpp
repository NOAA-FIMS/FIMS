/*
  * This File is part of the NOAA, National Marine Fisheries Service
* Fisheries Integrated Modeling System project.
* Refer to the LICENSE file for reuse information.
*
  * The purpose of this file is to declare the module_name functor class
* which is the base class for all module_name functors.
*/
  #ifndef LIKELIHOODS_TMB_LIKELIHOODS_HPP
  #define LIKELIHOODS_TMB_LIKELIHOODS_HPP

  #ifdef TMB_MODEL

  //#include "../../../interface/interface.hpp"
  //#include "../../../common/fims_math.hpp"
  #include "likelihoods_base.hpp"

  namespace fims {

  template <typename T>
  struct Dnorm : public LikelihoodsBase<T> {
    T sd; /*!< standard deviation of the distribution.;
                */

    Dnorm() : LikelihoodsBase<T>() {}

    /**
     * @brief return the (log) probability density function at x.
     *
     * @param x The observed value
     * @param mean The expected value
     * @param do_log Boolean, if true returns the log of the likelihood 
     */
    virtual const T evaluate(const T& x, const T& mean, const bool& do_log) {
      return dnorm(x, mean, sd, do_log);
    }
  };

template <typename T>
  struct Dmultinom : public LikelihoodsBase<T> {
                

    Dmultinom() : LikelihoodsBase<T>() {}

    /**
     * @brief return the (log) probability density function at x.
     *    
     * @param x The observed values (frequencies)
     * @param p probabilities
     * @param do_log Boolean, if true returns the log of the likelihood 
     */
    virtual const T evaluate(const T& x, const T& p, const bool& do_log) {
      return dmultinom(observed, p, do_log);
    }
  };

template <typename T>
  struct Dlnorm : public LikelihoodsBase<T> {
    T sdlog; /*!< standard deviation of the distribution of log(x) */

    Dlnorm() : LikelihoodsBase<T>() {}

    /**
     * @brief return the (log) probability density function at x.
     *
     *
     * 
     *
     * @param x  The independent variable
     * @param meanlog expected value of log(x) 
     * @param do_log Boolean, if true returns the log of the likelihood 
     */
    
    virtual const T evaluate(const T& observed, const T& meanlog, const bool& do_log) {
      Type logx = log(observed):
      Type nll = dnorm(logx, meanlog, sdlog, true) - logx;
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
