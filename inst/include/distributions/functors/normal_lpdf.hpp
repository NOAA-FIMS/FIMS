/*
 * File:   normal_lpdf.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Normal Log Probability Density Function (LPDF) module file
 * The purpose of this file is to define the Normal LPDF class and its fields
 * and return the log probability density function.
 *
 */

#ifndef NORMAL_LPDF
#define NORMAL_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions {
/**
 * Normal Negative Log-Likelihood
 */
template<typename Type>
struct NormalLPDF : public DensityComponentBase<Type> {
    fims::Vector<Type> log_sd; /**< log of the standard deviation of the distribution; can be a vector or scalar */
    fims::Vector<Type> mu; /**< mean of the distribution; can be a vector or scalar */
    fims::Vector<Type> sd; /**< standard deviation of the distribution; can be a vector or scalar */
    Type nll = 0.0; /**< total negative log-likelihood contribution of the distribution */
    std::vector<bool> is_na; /**< Boolean; if true, data observation is NA and the likelihood contribution is skipped */
    #ifdef TMB_MODEL
    ::objective_function<Type> *of; /**< Pointer to the TMB objective function */
    #endif
    //data_indicator<tmbutils::vector<Type> , Type> keep; /**< total negative log-likelihood contribution of the distribution */

    /** @brief Constructor.
     */
    NormalLPDF() : DensityComponentBase<Type>() {

    }

    /** @brief Destructor.
     */
    virtual ~NormalLPDF() {}

    /**
     * @brief Evaluates the negative log-likelihood of the normal probability density function
     * @param do_log Boolean; if true, log densities are returned
     */
    virtual const Type evaluate(const bool& do_log){
        this->mu.resize(this->observed_values.size());
        this->sd.resize(this->observed_values.size());
        this->nll_vec.resize(this->observed_values.size());
        for(size_t i=0; i<this->observed_values.size(); i++){
            if(this->expected_values.size() == 1){
                this->mu[i] = this->expected_values[0];
            } else {
              if(this->observed_values.size() != this->expected_values.size()){
                /* move error handling to CreateModel in information so not to crash R
                Rcpp::stop("the dimensions of the observed and expected values from normal negative log likelihood do not match");
                 */
              } else {
                this->mu[i] = this->expected_values[i];
              }
            }
            if(log_sd.size() == 1){
                sd[i] = fims_math::exp(log_sd[0]);
            } else {
              if(this->observed_values.size() != this->log_sd.size()){
                /* move error handling to CreateModel in information so not to crash R
                Rcpp::stop("the dimensions of the observed and log sd values from normal negative log likelihood do not match");
                 */
              } else {
                sd[i] = fims_math::exp(log_sd[i]);
              }
            }
            if(!is_na[i])
            {
              // this->nll_vec[i] = this->keep[i] * -dnorm(this->observed_values[i], mu[i], sd[i], do_log);
              #ifdef TMB_MODEL
              this->nll_vec[i] = -dnorm(this->observed_values[i], mu[i], sd[i], do_log);

              nll += this->nll_vec[i];
              if(this->simulate_flag){
                  FIMS_SIMULATE_F(this->of){
                      this->observed_values[i] = rnorm(mu[i], sd[i]);
                  }
              }
              #endif
            /* osa not working yet
              if(osa_flag){//data observation type implements osa residuals
                  //code for osa cdf method
                  this->nll_vec[i] = this->keep.cdf_lower[i] * -log( pnorm(this->observed_values[i], mu[i], sd[i]) );
                  this->nll_vec[i] = this->keep.cdf_upper[i] * -log( 1.0 - pnorm(this->observed_values[i], mu[i], sd[i]) );
              } */
            }
        }
        #ifdef TMB_MODEL
        vector<Type> normal_observed_values = this->observed_values;
        //FIMS_REPORT_F(normal_observed_values, this->of);
        #endif
        return(nll);
    }

};

} // namespace fims_distributions
#endif
