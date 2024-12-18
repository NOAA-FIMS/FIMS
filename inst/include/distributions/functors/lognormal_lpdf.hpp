/**
 * @file lognormal_lpdf.hpp
 * @brief Lognormal Log Probability Density Function (LPDF) defines the
 * Lognormal LPDF class and its fields and returns the log probability density
 * function.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef LOGNORMAL_LPDF
#define LOGNORMAL_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions
{
    /**
     * LogNormal Log Probability Density Function
     */
    template <typename Type>
    struct LogNormalLPDF : public DensityComponentBase<Type>
    {
        fims::Vector<Type> log_sd; /**< log of the standard deviation of the distribution on the log scale; can be a vector or scalar */
        Type lpdf = 0.0; /**< total log probability density contribution of the distribution */
        // data_indicator<tmbutils::vector<Type> , Type> keep; /**< Indicator used in TMB one-step-ahead residual calculations */

        /** @brief Constructor.
         */
        LogNormalLPDF() : DensityComponentBase<Type>()
        {
        }

        /** @brief Destructor.
         */
        virtual ~LogNormalLPDF() {}

        /**
         * @brief Evaluates the lognormal probability density function
         */
        virtual const Type evaluate()
        {
          // set vector size based on input type (prior, process, or data)
          size_t n_x;
          if(this->input_type == "data"){
            n_x = this->observed_values->data.size();
          } else {
            n_x = this->x.size();
          }
          // setup vector for recording the log probability density function values
          this->lpdf_vec.resize(n_x);
          std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), 0);
          lpdf = 0;

          for (size_t i = 0; i < n_x; i++)
          {
            #ifdef TMB_MODEL
            if(this->input_type == "data"){
              // if data, check if there are any NA values and skip lpdf calculation if there are
              // See Deroba and Miller, 2016 (https://doi.org/10.1016/j.fishres.2015.12.002) for
              // the use of lognormal constant
              if(this->observed_values->at(i) != this->observed_values->na_value){
                  this->lpdf_vec[i] = dnorm(log(this->observed_values->at(i)), this->expected_values.get_force_scalar(i),
                                            fims_math::exp(log_sd.get_force_scalar(i)), true) - log(this->observed_values->at(i));
                } else {
                this->lpdf_vec[i] = 0;
              }
              // if not data (i.e. prior or process), use x vector instead of observed_values and no lognormal constant needs to be applied
            } else {
              this->lpdf_vec[i] = dnorm(log(this->x[i]), this->expected_values.get_force_scalar(i),
                                        fims_math::exp(log_sd.get_force_scalar(i)), true);
            }

            lpdf += this->lpdf_vec[i];
            if (this->simulate_flag)
            {
                FIMS_SIMULATE_F(this->of)
                { // preprocessor definition in interface.hpp
                    // this simulates data that is mean biased
                    if(this->input_type == "data"){
                      this->observed_values->at(i) = fims_math::exp(rnorm(this->expected_values.get_force_scalar(i),
                                                                          fims_math::exp(log_sd.get_force_scalar(i))));
                    } else {
                      this->x[i] = fims_math::exp(rnorm(this->expected_values.get_force_scalar(i),
                                                        fims_math::exp(log_sd.get_force_scalar(i))));
                    }
                }
            }
            #endif
            }
            #ifdef TMB_MODEL
            vector<Type> lognormal_x = this->x;
          //  FIMS_REPORT_F(lognormal_x, this->of);
            #endif
            return (lpdf);
        }
    };
} // namespace fims_distributions
#endif
