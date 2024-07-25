/*
 * File:   multinomial_lpmf.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * Multinomial Log Probability Mass Function (LPMF) module file
 * The purpose of this file is to define the Multinomial LPMF class and its fields
 * and return the log probability mass function.
 *
 */

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
        Type nll = 0.0; /**< total negative log-likelihood contribution of the distribution */
        fims::Vector<size_t> dims; /**< Dimensions of the number of rows and columns of the multivariate dataset */
        std::vector<bool> is_na; /**< Boolean; if true, data observation is NA and the likelihood contribution for the entire row is skipped */
        #ifdef TMB_MODEL
        ::objective_function<Type> *of; /**< Pointer to the TMB objective function */
        #endif
        // data_indicator<tmbutils::vector<Type> , Type> keep;  /**< Indicator used in TMB one-step-ahead residual calculations */

        /** @brief Constructor.
         */
        MultinomialLPMF() : DensityComponentBase<Type>()
        {
        }

        /** @brief Destructor.
         */
        virtual ~MultinomialLPMF() {}

        /**
         * @brief Evaluates the negative log-likelihood of the multinomial probability mass function
         * @param do_log Boolean; if true, log densities are returned
         */
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
                #ifdef TMB_MODEL
                for (size_t j = 0; j < dims[1]; j++)
                {
                    size_t idx = (i * dims[1]) + j;
                    observed_vector[j] = this->x[idx];
                    expected_vector[j] = this->expected_values[idx];
                }

                this->nll_vec[i] = -dmultinom((vector<Type>)observed_vector, (vector<Type>)expected_vector, do_log);
                nll += this->nll_vec[i];
                /*
                if (this->simulate_flag)
                {
                    FIMS_SIMULATE_F(this->of)
                    {
                        fims::Vector<Type> sim_observed;
                        sim_observed.resize(dims[1]);
                        sim_observed = rmultinom(expected_vector);
                        sim_observed.resize(this->x);
                        for (size_t j = 0; j < dims[1]; j++)
                        {
                            idx = (i * dims[1]) + j;
                            this->x[idx] = sim_observed[j];
                        }
                    }
                }
                */
               #endif
              }
            }
            #ifdef TMB_MODEL
            vector<Type> x = this->x;
          //  FIMS_REPORT_F(x, this->of);
            #endif
            return (nll);
        }

};
} // namespace fims_distributions
#endif
