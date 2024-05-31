#ifndef MULTINOMIAL_LPMF
#define MULTINOMIAL_LPMF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions
{
    /**
     * Multinomial Negative Log-Likelihood
     */
    template <typename Type>
    struct MultinomialLPMF : public DensityComponentBase<Type>
    {
        Type nll = 0.0;
        fims::Vector<size_t> dims;
        std::vector<bool> is_na;
        // data_indicator<tmbutils::vector<Type> , Type> keep;

        MultinomialLPMF() : DensityComponentBase<Type>()
        {
        }

        virtual ~MultinomialLPMF() {}

        virtual const Type evaluate(const bool& do_log)
        {
            this->nll_vec.resize(dims[0]);
            fims::Vector<Type> observed_vector;
            fims::Vector<Type> expected_vector;
            observed_vector.resize(dims[1]);
            expected_vector.resize(dims[1]);
            for (size_t i = 0; i < dims[0]; i++)
            {
              if(!is_na[i]){
                for (size_t j = 0; j < dims[1]; j++)
                {
                    size_t idx = (i * dims[1]) + j;
                    observed_vector[j] = this->observed_values[idx];
                    expected_vector[j] = this->expected_values[idx];
                }
                #ifdef TMB_model
                this->nll_vec[i] = -dmultinom((vector<Type>)observed_vector, (vector<Type>)expected_vector, do_log);
                #endif
                nll += this->nll_vec[i];
                /*
                if (this->simulate_flag)
                {
                    FIMS_SIMULATE_F(this->of)
                    {
                        fims::Vector<Type> sim_observed;
                        sim_observed.resize(dims[1]);
                        sim_observed = rmultinom(expected_vector);
                        sim_observed.resize(this->observed_values);
                        for (size_t j = 0; j < dims[1]; j++)
                        {
                            idx = (i * dims[1]) + j;
                            this->observed_values[idx] = sim_observed[j];
                        }
                    }
                }
                */
              }
            }
            #ifdef TMB_MODEL
            vector<Type> observed_values = this->observed_values;
          //  FIMS_REPORT_F(observed_values, this->of);
            #endif
            return (nll);
        }

};
} // namespace fims_distributions
#endif
