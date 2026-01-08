
#include <cmath>

#include "../inst/include/common/model.hpp"
#include "../inst/include/interface/interface.hpp"
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "fims_modules.hpp"
#include "init.hpp"

/// @cond
/**
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template <class Type> Type objective_function<Type>::operator()() {

  PARAMETER_VECTOR(p);
  PARAMETER_VECTOR(re);

  // code below copied from ModularTMBExample/src/tmb_objective_function.cpp

  // get the singleton instance for Model Class
  std::shared_ptr<fims_model::Model<Type>> model =
      fims_model::Model<Type>::GetInstance();
  // get the singleton instance for Information Class
  std::shared_ptr<fims_info::Information<Type>> information =
      fims_info::Information<Type>::GetInstance();

  // update the fixed effects parameter values
  for (size_t i = 0; i < information->fixed_effects_parameters.size(); i++) {
    *information->fixed_effects_parameters[i] = p[i];
  }
  // update the random effects parameter values
  for (size_t i = 0; i < information->random_effects_parameters.size(); i++) {
    *information->random_effects_parameters[i] = re[i];
  }
  model->of = this;

  // evaluate the model objective function value
  Type nll = static_cast<Type>(0.0);
  try {
    nll = model->Evaluate();
  } catch (const std::exception &e) {
    std::stringstream err_msg;
    err_msg << "Exception caught during model evaluation: " << e.what()
            << std::endl;
    Rf_error(err_msg.str().c_str());
  }

  return nll;
}
/// @endcond
