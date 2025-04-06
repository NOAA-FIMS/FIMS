/**
 * @file normal_lpdf.hpp
 * @brief Normal Log Probability Density Function (LPDF) module file defines
 * the Normal LPDF class and its fields and returns the log probability density
 * function.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
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
    fims::Vector<Type> log_sd; /**< the natural log of the standard deviation of the distribution; can be a vector or scalar */
    Type lpdf = Type(0.0); /**< total log probability density contribution of the distribution */

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
      // set vector size based on input type (prior, process, or data)
      size_t n_x;
      if(this->input_type == "data"){
        n_x = this->observed_values->data.size();
      } else {
        n_x = this->x.size();
      }
      // setup vector for recording the log probability density function values
      this->lpdf_vec.resize(n_x);
      std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), Type(0));
      lpdf = Type(0);

      for(size_t i=0; i<n_x; i++){
        #ifdef TMB_MODEL
        if(this->input_type == "data"){
          // if data, check if there are any NA values and skip lpdf calculation if there are
          if(this->observed_values->at(i) != this->observed_values->na_value){
            this->lpdf_vec[i] = dnorm(this->observed_values->at(i), this->expected_values.get_force_scalar(i), fims_math::exp(log_sd.get_force_scalar(i)), true);
          } else {
            this->lpdf_vec[i] = 0;
          }
          // if not data (i.e. prior or process), use x vector instead of observed_values
        } else {
          this->lpdf_vec[i] = dnorm(this->x[i], this->expected_values.get_force_scalar(i), fims_math::exp(log_sd.get_force_scalar(i)), true);
        }
        lpdf += this->lpdf_vec[i];
        if(this->simulate_flag){
            FIMS_SIMULATE_F(this->of){
              if(this->input_type == "data"){
                this->observed_values->at(i) = rnorm(this->expected_values.get_force_scalar(i), fims_math::exp(log_sd.get_force_scalar(i)));
              } else {
                this->x[i] = rnorm(this->expected_values.get_force_scalar(i), fims_math::exp(log_sd.get_force_scalar(i)));
              }
            }
        }
        #endif
        /* osa not working yet
          if(osa_flag){//data observation type implements osa residuals
              //code for osa cdf method
              this->lpdf_vec[i] = this->keep.cdf_lower[i] * log( pnorm(this->x[i], this->expected_values.get_force_scalar(i), sd[i]) );
              this->lpdf_vec[i] = this->keep.cdf_upper[i] * log( 1.0 - pnorm(this->x[i], this->expected_values.get_force_scalar(i), sd[i]) );
          } */

        }
        #ifdef TMB_MODEL
        vector<Type> normal_x = this->x;
        #endif
        return(lpdf);
    }

};

} // namespace fims_distributions
#endif
