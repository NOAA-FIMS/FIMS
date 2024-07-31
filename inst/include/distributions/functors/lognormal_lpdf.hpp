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
         */
        virtual const Type evaluate()
        {
          size_t n_x;
          if(this->input_type == "data"){
            n_x = this->observed_values->data.size();
          } else {
            n_x = this->x.size();
          }
            this->logmu.resize(n_x);
            this->logsd.resize(n_x);
            this->lpdf_vec.resize(n_x);
            std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), 0); 
            lpdf = 0;
            for (size_t i = 0; i < n_x; i++)
            {
                if (this->expected_values.size() == 1)
                {
                    this->logmu[i] = this->expected_values[0];
                } else {
                  if(n_x != this->expected_values.size()){
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
                  if(n_x != this->log_logsd.size()){
                    /* move error handling to CreateModel in information so not to crash R
                    Rcpp::stop("the dimensions of the observed and log logsd values from lognormal negative log likelihood do not match");
                     */
                  } else {
                    logsd[i] = fims_math::exp(log_logsd[i]);
                  }
                }

                #ifdef TMB_MODEL
                if(this->input_type == "data"){
                  if(this->observed_values->at(i) != this->observed_values->na_value){
                  // this->lpdf_vec[i] = this->keep[i] * -dnorm(log(this->observed_values->at(i)), logmu[i], logsd[i], true) - log(this->observed_values->->at(i));
                      this->lpdf_vec[i] = dnorm(log(this->observed_values->at(i)), logmu[i], logsd[i], true) - log(this->observed_values->at(i));
                    } else {
                    this->lpdf_vec[i] = 0;
                    MODEL_LOG << "lpdf_vec for obs " << i << " is: " << this->lpdf_vec[i] <<std::endl;
                  } 
                } else {
                  this->lpdf_vec[i] = dnorm(log(this->x[i]), logmu[i], logsd[i], true);
                }

                lpdf += this->lpdf_vec[i];
                if (this->simulate_flag)
                {
                    FIMS_SIMULATE_F(this->of)
                    { // preprocessor definition in interface.hpp
                        // this simulates data that is mean biased
                        if(this->input_type == "data"){
                          this->observed_values->at(i) = fims_math::exp(rnorm(logmu[i], logsd[i]));
                        } else {
                          this->x[i] = fims_math::exp(rnorm(logmu[i], logsd[i]));
                        }
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
            #ifdef TMB_MODEL
            vector<Type> lognormal_x = this->x;
          //  FIMS_REPORT_F(lognormal_x, this->of);
            #endif
            return (lpdf);
        }
    };
} // namespace fims_distributions
#endif
