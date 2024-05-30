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
        vector<size_t> dims;
        #ifdef TMB_MODEL
        ::objective_function<Type> *of;
        #endif
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
            for (int i = 0; i < dims[0]; i++)
            {
                for (int j = 0; j < dims[1]; j++)
                {
                    int idx = (i * dims[1]) + j;
                    observed_vector[j] = this->observed_values[idx];
                    expected_vector[j] = this->expected_values[idx];
                }
                this->nll_vec[i] = -dmultinom(observed_vector, expected_vector, do_log);
                nll += this->nll_vec[i];
                /*
                if (this->simulate_flag)
                {
                    FIMS_SIMULATE_F(this->of)
                    {
                        fims::Vector<Type> sim_observed;
                        sim_observed.resize(dims[1]);
                        sim_observed = rmultinom(expected_vector);
                        sim_observed.resize(this->observed_value); 
                        for (int j = 0; j < dims[1]; j++)
                        {
                            idx = (i * dims[1]) + j;
                            this->observed_values[idx] = sim_observed[j]; 
                        }
                    }               
                }
                */
            }

            vector<Type> observed_values = this->observed_values;
            FIMS_REPORT_F(observed_values, this->of);

            return (nll);
        }
   

};
 } // namespace fims_distributions
#endif;