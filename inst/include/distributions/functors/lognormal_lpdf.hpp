/*
 * File:   lognormal_lpdf.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Lognormal Log Probability Density Function (LPDF) module file
 * The purpose of this file is to define the Lognormal LPDF class and its fields
 * and return the log probability density function.
 *
 */
#ifndef LOGNORMAL_LPDF
#define LOGNORMAL_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions
{
    /**
     * LogNormal Log Probability Density Function
     */
    template <typename Type>
    struct LogNormalLPDF : public DensityComponentBase<Type>
    {
        fims::Vector<Type> log_logsd; /**< log of the standard deviation of the distribution on the log scale; can be a vector or scalar */
        fims::Vector<Type> logmu; /**< mean of the distribution on the log scale; can be a vector or scalar */
        fims::Vector<Type> logsd; /**< standard deviation of the distribution on the log scale; can be a vector or scalar */
        std::vector<bool> is_na; /**< Boolean; if true, data observation is NA and the likelihood contribution is skipped */
        #ifdef TMB_MODEL
        ::objective_function<Type> *of; /**< Pointer to the TMB objective function */
        #endif
        Type lpdf = 0.0; /**< total log probability density contribution of the distribution */
        // data_indicator<tmbutils::vector<Type> , Type> keep; /**< Indicator used in TMB one-step-ahead residual calculations */

        /** @brief Constructor.
         */
        LogNormalLPDF() : DensityComponentBase<Type>()
        {
        }

        /** @brief Destructor.
         */
        virtual ~LogNormalLPDF() {}

        /**
         * @brief Evaluates the lognormal probability density function
         * @param do_log Boolean; if true, log densities are returned
         */
        virtual const Type evaluate(const bool &do_log)
        {
            this->logmu.resize(this->x.size());
            this->logsd.resize(this->x.size());
            is_na.resize(this->x.size());
            this->lpdf_vec.resize(this->x.size());
            for (size_t i = 0; i < this->x.size(); i++)
            {
                if (this->expected_values.size() == 1)
                {
                    this->logmu[i] = this->expected_values[0];
                } else {
                  if(this->x.size() != this->expected_values.size()){
                    /* move error handling to CreateModel in information so not to crash R
                    Rcpp::stop("the dimensions of the observed and expected values from lognormal negative log likelihood do not match");
                     */
                  } else {
                    this->logmu[i] = this->expected_values[i];
                  }
                }
                if (log_logsd.size() == 1)
                {
                    logsd[i] = fims_math::exp(log_logsd[0]);
                } else {
                  if(this->x.size() != this->log_logsd.size()){
                    /* move error handling to CreateModel in information so not to crash R
                    Rcpp::stop("the dimensions of the observed and log logsd values from lognormal negative log likelihood do not match");
                     */
                  } else {
                    logsd[i] = fims_math::exp(log_logsd[i]);
                  }
                }

                if(!is_na[i])
                {
                  #ifdef TMB_MODEL
                  // this->lpdf_vec[i] = this->keep[i] * dnorm(this->x[i], logmu[i], logsd[i], do_log);
                  this->lpdf_vec[i] = dnorm(log(this->x[i]), logmu[i], logsd[i], true);
                  if(this->lpdf_type == "data"){
                    this->lpdf_vec[i] -= log(this->x[i]);
                  }
                  if(!do_log){
                    this->lpdf_vec[i] = exp(this->lpdf_vec[i]);
                  }
                  lpdf += this->lpdf_vec[i];
                  if (this->simulate_flag)
                  {
                      FIMS_SIMULATE_F(this->of)
                      { // preprocessor definition in interface.hpp
                          // this simulates data that is mean biased
                          this->x[i] = fims_math::exp(rnorm(logmu[i], logsd[i]));
                      }
                  }
                  #endif

                  /* osa not working yet
                    if(osa_flag){//data observation type implements osa residuals
                        //code for osa cdf method
                        this->lpdf_vec[i] = this->keep.cdf_lower[i] * log( pnorm(this->x[i], logmu[i], logsd[i]) );
                        this->lpdf_vec[i] = this->keep.cdf_upper[i] * log( 1.0 - pnorm(this->x[i], logmu[i], logsd[i]) );
                    } */
                }
            }
            #ifdef TMB_MODEL
            vector<Type> lognormal_x = this->x;
          //  FIMS_REPORT_F(lognormal_x, this->of);
            #endif
            return (lpdf);
        }
    };
} // namespace fims_distributions
#endif
