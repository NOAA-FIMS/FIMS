#ifndef MULTINOMIAL_LPMF
#define MULTINOMIAL_LPMF

#include "density_components_base.hpp"
#include "../common/fims_vector.hpp"
#include "../common/def.hpp"

namespace fims_distributions
{
    /**
     * Multinomial Negative Log-Likelihood
     */
    template <typename Type>
    struct MultinomialLPMF : public DensityComponentBase<Type>
    {
        bool osa_flag;
        Type nll = 0.0;
        vector<int> dims;
        // data_indicator<tmbutils::vector<Type> , Type> keep;

        MultinomialLPMF() : DistributionComponentBase<Type>()
        {
        }

        virtual ~MultinomialLPMF() {}

        virtual const Type evaluate()
        {
            this->nll_vec.resize(dims[0]);
            for (int i = 0; i < dims[0]; i++)
            {
                for (int j = 0; j < dims[1]; j++)
                {
                    idx = (i * dims[1]) + j;
                    this->nll_vec[i] = dmultinom(this->observed_value[idx], expected_value[idx]);

                    nll += this->nll_vec[i];
                    if (this->simulate_flag)
                    {
                        FIMS_SIMULATE_F(this->of)
                        {
                            this->observed_value[i] = rmultinom(observed_value[i], expected_value[i]);
                        }
                    }               
                }
            }
        }

            vector<Type> multinomial_observed_value = this->observed_value;
            FIMS_REPORT_F(multinomial_observed_value, this->of);

            return (nll);
        }
    } // namespace fims_distributions

};
#endif;