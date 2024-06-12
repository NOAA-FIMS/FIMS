#ifndef LOGNORMAL_LPDF
#define LOGNORMAL_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions
{
    /**
     * LogNormal Negative Log-Likelihood
     */
    template <typename Type>
    struct LogNormalLPDF : public DensityComponentBase<Type>
    {
        fims::Vector<Type> log_sd;
        fims::Vector<Type> mu;
        fims::Vector<Type> sd;
        std::vector<bool> is_na;
        #ifdef TMB_MODEL
        ::objective_function<Type> *of;
        #endif
        Type nll = 0.0;
        // data_indicator<tmbutils::vector<Type> , Type> keep;

        LogNormalLPDF() : DensityComponentBase<Type>()
        {
        }

        virtual ~LogNormalLPDF() {}

        virtual const Type evaluate(const bool &do_log)
        {
            this->mu.resize(this->observed_values.size());
            this->sd.resize(this->observed_values.size());
            is_na.resize(this->observed_values.size());
            this->nll_vec.resize(this->observed_values.size());
            for (size_t i = 0; i < this->observed_values.size(); i++)
            {
                if (this->expected_values.size() == 1)
                {
                    this->mu[i] = this->expected_values[0];
                } else {
                  if(this->observed_values.size() != this->expected_values.size()){
                    /* move error handling to CreateModel in information so not to crash R
                    Rcpp::stop("the dimensions of the observed and expected values from lognormal negative log likelihood do not match");
                     */
                  } else {
                    this->mu[i] = this->expected_values[i];
                  }
                }
                if (log_sd.size() == 1)
                {
                    sd[i] = fims_math::exp(log_sd[0]);
                } else {
                  if(this->observed_values.size() != this->log_sd.size()){
                    /* move error handling to CreateModel in information so not to crash R
                    Rcpp::stop("the dimensions of the observed and log sd values from lognormal negative log likelihood do not match");
                     */
                  } else {
                    sd[i] = fims_math::exp(log_sd[i]);
                  }
                }

                if(!is_na[i])
                {
                  #ifdef TMB_MODEL
                  // this->nll_vec[i] = this->keep[i] * -dnorm(this->observed_values[i], mu[i], sd[i], do_log);
                  this->nll_vec[i] = -dnorm(log(this->observed_values[i]), mu[i], sd[i], true) + log(this->observed_values[i]);
                  if(!do_log){
                    this->nll_vec[i] = -exp(-this->nll_vec[i]);
                  }
                  nll += this->nll_vec[i];
                  if (this->simulate_flag)
                  {
                      FIMS_SIMULATE_F(this->of)
                      { // preprocessor definition in interface.hpp
                          // this simulates data that is mean biased
                          this->observed_values[i] = fims_math::exp(rnorm(mu[i], sd[i]));
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
            vector<Type> lognormal_observed_values = this->observed_values;
          //  FIMS_REPORT_F(lognormal_observed_values, this->of);
            #endif
            return (nll);
        }
    };
} // namespace fims_distributions
#endif
