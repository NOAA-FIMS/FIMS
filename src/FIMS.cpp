
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

    int do_mcmc = 0;
    SEXP data_names = Rf_getAttrib(TMB_OBJECTIVE_PTR->data, R_NamesSymbol);
    if (!Rf_isNull(data_names)) {
      R_xlen_t data_size = Rf_xlength(data_names);
      for (R_xlen_t i = 0; i < data_size; i++) {
        if (std::string(CHAR(STRING_ELT(data_names, i))) == "do_mcmc") {
          SEXP do_mcmc_data = VECTOR_ELT(TMB_OBJECTIVE_PTR->data, i);
          if (!Rf_isNull(do_mcmc_data)) {
            do_mcmc = Rf_asInteger(do_mcmc_data);
          }
          break;
        }
      }
    }


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
    model -> jacobian_flag = (do_mcmc == 1);

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
