
#include <cmath>

#include "rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
//#include "../inst/include/interface/init.hpp"
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

    // code below copied from ModularTMBExample/src/tmb_objective_function.cpp

    // get the singleton instance for Model Class
    std::shared_ptr<fims_model::Model<Type>> model =
      fims_model::Model<Type>::GetInstance();
    // get the singleton instance for Information Class
    std::shared_ptr<fims_info::Information<Type>> information =
      fims_info::Information<Type>::GetInstance();

    //update the parameter values
    for(size_t i =0; i < information->fixed_effects_parameters.size(); i++){
        *information->fixed_effects_parameters[i] = p[i];
    }
    model -> of = this;

    //evaluate the model objective function value
    Type nll = model->Evaluate();

    return nll;

}
/// @endcond
