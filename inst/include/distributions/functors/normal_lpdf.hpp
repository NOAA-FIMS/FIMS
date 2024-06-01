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
    fims::Vector<Type> log_sd;
    fims::Vector<Type> mu;
    fims::Vector<Type> sd;
    Type nll = 0.0;
    std::vector<bool> is_na;
    #ifdef TMB_MODEL
    ::objective_function<Type> *of;
    #endif
    //data_indicator<tmbutils::vector<Type> , Type> keep;

    NormalLPDF() : DensityComponentBase<Type>() {

    }

    virtual ~NormalLPDF() {}

    virtual const Type evaluate(const bool& do_log){
        this->mu.resize(this->observed_values.size());
        this->sd.resize(this->observed_values.size());
        for(size_t i=0; i<this->expected_values.size(); i++){
            if(this->expected_values.size() == 1){
                this->mu[i] = this->expected_values[0];
            } else {
                this->mu[i] = this->expected_values[i];
            }
            if(log_sd.size() == 1){
                sd[i] = fims_math::exp(log_sd[0]);
            } else {
                sd[i] = fims_math::exp(log_sd[i]);
            }
        }
        this->nll_vec.resize(this->observed_values.size());
        for(size_t i=0; i<this->observed_values.size(); i++){
          if(!is_na[i]){
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
