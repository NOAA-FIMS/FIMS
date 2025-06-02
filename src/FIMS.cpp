
#include <cmath>

#include "../inst/include/interface/interface.hpp"
#include "../inst/include/common/model.hpp"
#include <R_ext/Rdynload.h>
#include <stdlib.h>
#include <Rinternals.h>

/// @cond
/**
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template<class Type>
Type objective_function<Type>::operator()() {

    DATA_VECTOR(y);
    PARAMETER_VECTOR(p);

    // code below copied from ModularTMBExample/src/tmb_objective_function.cpp

    // get the singleton instance for Model Class
    std::shared_ptr<fims_model::Model<Type>> model =
      fims_model::Model<Type>::GetInstance();
    // get the singleton instance for Information Class
    std::shared_ptr<fims_info::Information<Type>> information =
      fims_info::Information<Type>::GetInstance();

    //update the data vector 
    for(size_t i =0; i < information->data.size(); i++){
      *information->data[i] = y[i];
    }
    //update the parameter values
    for(size_t i =0; i < information->fixed_effects_parameters.size(); i++){
        *information->fixed_effects_parameters[i] = p[i];
    }
    model -> of = this;

    //evaluate the model objective function value
    Type nll = model->Evaluate();

    return nll;

}


extern "C" {

SEXP _rcpp_module_boot_fims();
SEXP _rtmb_set_shared_pointers();
// Redefine CallEntries here so it's not just in the header!
static const R_CallMethodDef CallEntries[] = {
    TMB_CALLDEFS,
    {"_rcpp_module_boot_fims", (DL_FUNC)&_rcpp_module_boot_fims, 0},
    {"_rtmb_set_shared_pointers", (DL_FUNC)&_rtmb_set_shared_pointers, 0},
    {NULL, NULL, 0}
};

void R_init_FIMS(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}

}

/// @endcond
