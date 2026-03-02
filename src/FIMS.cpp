
#include <cmath>

#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
#include "init.hpp"
#include "fims_modules.hpp"
#include "../inst/include/common/model.hpp"

/// @cond
/**
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template<class Type>
Type objective_function<Type>::operator()() {


    PARAMETER_VECTOR(p);
    PARAMETER_VECTOR(re);

    // code below copied from ModularTMBExample/src/tmb_objective_function.cpp

    // get the singleton instance for Model Class
    std::shared_ptr<fims_model::Model<Type>> model =
      fims_model::Model<Type>::GetInstance();
    // get the singleton instance for Information Class
    std::shared_ptr<fims_info::Information<Type>> information =
      fims_info::Information<Type>::GetInstance();

    //update the fixed effects parameter values
    for(size_t i =0; i < information->fixed_effects_parameters.size(); i++){
        *information->fixed_effects_parameters[i] = p[i];
    }
    //update the random effects parameter values
    for(size_t i =0; i < information->random_effects_parameters.size(); i++){
        *information->random_effects_parameters[i] = re[i];
    }
    model -> of = this;

    Type nll = 0;
    //evaluate the model objective function value
    try{
      nll = model->Evaluate();
    } catch (const std::exception& e) {
      Rf_error("Error during model evaluation: %s",  std::string(e.what()).c_str());
    }

    // [TEMPORARY_FEATURE] Begin recruitment deviation penalty calculation
    // This temporary feature adds a penalty term to constrain recruitment deviations
    Type sum_devs = 0.0;  // Accumulator for sum of recruitment log deviations
    int start_index = 36; // Corresponds to recruitment log_devs index 2 in parameter vector
    int end_index = 64;   // Corresponds to recruitment log_devs index 30 in parameter vector

    // Safety check to ensure we don't crash if vector size changes
    if (p.size() > end_index) {
        // Accumulate the sum of the selected recruitment deviation parameters
        // Iterates through recruitment log_devs indices 2-30 (29 values)
        for (int i = start_index; i <= end_index; ++i) {
            sum_devs += p[i];  // Add each recruitment deviation to running sum
        }

        // Apply a strong penalty (weight = 10000) to force the sum close to zero
        // This constrains recruitment deviations to sum to zero as a modeling assumption
        // Penalty term: 10000 * (sum_devs)^2
        Type penalty = 10000.0 * sum_devs * sum_devs;
        nll += penalty;  // Add penalty to negative log likelihood
    }
    // [TEMPORARY_FEATURE] End recruitment deviation penalty calculation

    return nll;

}
/// @endcond
