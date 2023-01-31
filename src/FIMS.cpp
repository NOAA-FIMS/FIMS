
#include <cmath>
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
#include "../inst/include/interface/init.hpp"

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

    // get the singleton instance for type Type
    std::shared_ptr<fims::Model> model =
    fims::Model<Type>::GetInstance();

    //update the parameter values for type Type
    for(int i =0; i < model->parameters.size(); i++){
        *model->parameters[i] = p[i];
    }
    
    //evaluate the model objective function value
    Type nll = model->evaluate();

    return nll;

}
/// @endcond