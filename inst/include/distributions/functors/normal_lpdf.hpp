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
      size_t n_x;
      if(this->input_type == "data"){
        n_x = this->observed_values->data.size();
      } else {
        n_x = this->x.size();
      }
        this->mu.resize(n_x);
        this->sd.resize(n_x);
        this->lpdf_vec.resize(n_x);
        std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), 0); 
        lpdf = 0;
        for(size_t i=0; i<n_x; i++){
            if(this->expected_values.size() == 1){
                this->mu[i] = this->expected_values[0];
            } else {
              if(n_x != this->expected_values.size()){
                /* move error handling to CreateModel in information so not to crash R
                Rcpp::stop("the dimensions of the observed and expected values from normal negative log likelihood do not match");
                 */
              } else {
                this->mu[i] = this->expected_values[i];
              }
            }
            #ifdef TMB_MODEL
            if(this->input_type == "data"){
              if(this->observed_values->at(i) != this->observed_values->na_value){
              // this->lpdf_vec[i] = this->keep[i] * -dnorm(this->observed_values->at(i), mu[i], sd[i], true);
                  this->lpdf_vec[i] = dnorm(this->observed_values->at(i), mu[i], fims_math::exp(log_sd.get_force_scalar(i)), true);
          
              } else {
                this->lpdf_vec[i] = 0;
              } 
              
            } else {
              this->lpdf_vec[i] = dnorm(this->x[i], mu[i], fims_math::exp(log_sd.get_force_scalar(i)), true);
            }
            lpdf += this->lpdf_vec[i];
            if(this->simulate_flag){
                FIMS_SIMULATE_F(this->of){
                  if(this->input_type == "data"){
                    this->observed_values->at(i) = rnorm(mu[i], fims_math::exp(log_sd.get_force_scalar(i)));
                  } else {
                    this->x[i] = rnorm(mu[i], fims_math::exp(log_sd.get_force_scalar(i)));
                  }
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
        #ifdef TMB_MODEL
        vector<Type> normal_x = this->x;
        //FIMS_REPORT_F(normal_x, this->of);
        #endif
        return(lpdf);
    }

};

} // namespace fims_distributions
#endif
