#ifndef NORMAL_LPDF 
#define NORMAL_LPDF 

#include "distributions_base.hpp"
#include "../common/fims_vector.hpp"
#include "../common/def.hpp"

namespace fims_distributions {
/**
 * Normal Negative Log-Likelihood
 */
template<typename Type>
struct NormalLPDF : public DensityComponentBase<Type> {
    fims::Vector<Type> log_sd;
    fims::Vector<Type> mu;
    fims::Vector<Type> sd;
    bool osa_flag;
    Type nll = 0.0;
    //data_indicator<tmbutils::vector<Type> , Type> keep;
   

    NormalLPDF() : DistributionComponentBase<Type>() {

    }

    virtual ~NormalLPDF() {}

    virtual const Type evaluate(){
        this->mu.resize(this->observed_value.size());
        this->sd.resize(this->observed_value.size());
        for(int i=0; i<this->expected_value.size(); i++){
            if(this->expected_value.size() == 1){
                this->mu[i] = this->expected_value[0];
            } else {
                this->mu[i] = this->expected_value[i];
            }
       
            if(log_sd.size() == 1){
                sd[i] = exp(log_sd[0]);
            } else {
                sd[i] = exp(log_sd[i]);
            }
        }
        this->nll_vec.resize(this->observed_value.size());
        for(int i=0; i<this->observed_value.size(); i++){
           // this->nll_vec[i] = this->keep[i] * -dnorm(this->observed_value[i], mu[i], sd[i], true);
            this->nll_vec[i] = -dnorm(this->observed_value[i], mu[i], sd[i], true);
            nll += this->nll_vec[i];
            if(this->simulate_flag){
                FIMS_SIMULATE_F(this->of){
                    this->observed_value[i] = rnorm(mu[i], sd[i]);
                }
                
            }
            
          /* osa not working yet
            if(osa_flag){//data observation type implements osa residuals
                //code for osa cdf method
                this->nll_vec[i] = this->keep.cdf_lower[i] * -log( pnorm(this->observed_value[i], mu[i], sd[i]) );
                this->nll_vec[i] = this->keep.cdf_upper[i] * -log( 1.0 - pnorm(this->observed_value[i], mu[i], sd[i]) );
            } */

           
            
        }
        #ifdef TMB_MODEL
            vector<Type> normal_observed_value = this->observed_value;
            REPORT_F(normal_observed_value, this->of);
        #endif
        return(nll);
    }
} // namespace fims_distributions

};
#endif;