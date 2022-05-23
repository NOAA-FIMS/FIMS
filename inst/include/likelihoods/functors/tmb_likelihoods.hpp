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

  #include "../../../interface/interface.hpp"
  //#include "../../../common/fims_math.hpp"
  #include "likelihoods_base.hpp"

  namespace fims {

  template <typename T>
  struct dnorm : public LikelihoodsBase<T> {
    T sd; /*!< standard deviation of the distribution.;
                */

    dnorm() : LikelihoodsBase<T>() {}

    /**
     * @brief return the (log) probability density function at x.
     *
     *
     * 
     *
     * @param x  The independent variable
     */
    virtual const T evaluate(const T& observed, const T& expected, const bool& do_log) {
      inline dnorm(T x, T mean, T sd, bool do_log){ //bool or int?
        return dnorm(observed, expected, sd, do_log);
      }
    }
  };
}

#endif

#endif /* LIKELIHOODS_TMB_LIKELIHOODS_HPP */
