
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

    //evaluate the model objective function value
    Type nll = model->Evaluate();

    // Temporary test: add sum-to-zero constraint
    Type sum_devs = 0.0;
    int start_index = 36; // R index 37
    int end_index = 64;   // R index 65

    // Safety check to ensure we don't crash if vector size changes
    if (p.size() > end_index) {
        for (int i = start_index; i <= end_index; ++i) {
            sum_devs += p[i];
        }

        // Apply strong penalty (Weight = 10,000)
        Type penalty = 10000.0 * sum_devs * sum_devs;
        nll += penalty;
    }

    return nll;

}
/// @endcond
