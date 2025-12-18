
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

    return nll;

}
/// @endcond
