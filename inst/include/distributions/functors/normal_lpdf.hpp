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
 * Normal Log Probability Density Function
 */
template<typename Type>
struct NormalLPDF : public DensityComponentBase<Type> {
    fims::Vector<Type> log_sd; /**< log of the standard deviation of the distribution; can be a vector or scalar */
    fims::Vector<Type> mu; /**< mean of the distribution; can be a vector or scalar */
    fims::Vector<Type> sd; /**< standard deviation of the distribution; can be a vector or scalar */
    Type lpdf = 0.0; /**< total log probability density contribution of the distribution */
    std::vector<bool> is_na; /**< Boolean; if true, data observation is NA and the likelihood contribution is skipped */
    #ifdef TMB_MODEL
    ::objective_function<Type> *of; /**< Pointer to the TMB objective function */
    #endif
    //data_indicator<tmbutils::vector<Type> , Type> keep; /**< Indicator used in TMB one-step-ahead residual calculations */

    /** @brief Constructor.
     */
    NormalLPDF() : DensityComponentBase<Type>() {

    }

    /** @brief Destructor.
     */
    virtual ~NormalLPDF() {}

    /**
     * @brief Evaluates the normal probability density function
     */
    virtual const Type evaluate(){
        this->mu.resize(this->x.size());
        this->sd.resize(this->x.size());
        this->lpdf_vec.resize(this->x.size());
        for(size_t i=0; i<this->x.size(); i++){
            if(this->expected_values.size() == 1){
                this->mu[i] = this->expected_values[0];
            } else {
              if(this->x.size() != this->expected_values.size()){
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
              if(this->x.size() != this->log_sd.size()){
                /* move error handling to CreateModel in information so not to crash R
                Rcpp::stop("the dimensions of the observed and log sd values from normal negative log likelihood do not match");
                 */
              } else {
                sd[i] = fims_math::exp(log_sd[i]);
              }
            }
            if(!is_na[i])
            {
              // this->lpdf_vec[i] = this->keep[i] * -dnorm(this->x[i], mu[i], sd[i], true);
              #ifdef TMB_MODEL
              this->lpdf_vec[i] = dnorm(this->x[i], mu[i], sd[i], true);

              lpdf += this->lpdf_vec[i];
              if(this->simulate_flag){
                  FIMS_SIMULATE_F(this->of){
                      this->x[i] = rnorm(mu[i], sd[i]);
                  }
              }
              #endif
            /* osa not working yet
              if(osa_flag){//data observation type implements osa residuals
                  //code for osa cdf method
                  this->lpdf_vec[i] = this->keep.cdf_lower[i] * log( pnorm(this->x[i], mu[i], sd[i]) );
                  this->lpdf_vec[i] = this->keep.cdf_upper[i] * log( 1.0 - pnorm(this->x[i], mu[i], sd[i]) );
              } */
            }
        }
        #ifdef TMB_MODEL
        vector<Type> normal_x = this->x;
        //FIMS_REPORT_F(normal_x, this->of);
        #endif
        return(lpdf);
    }

};

} // namespace fims_distributions
#endif
