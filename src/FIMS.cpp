
#include <cmath>

#include "../inst/include/interface/interface.hpp"
#include "../inst/include/interface/TMB/init_tmb.hpp"
#include "../inst/include/common/model.hpp"

/// @cond
/**
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template <class Type>
Type objective_function<Type>::operator()()
{

  PARAMETER_VECTOR(p);
  PARAMETER_VECTOR(re);

  // code below copied from ModularTMBExample/src/tmb_objective_function.cpp

  // get the singleton instance for Model Class
  std::shared_ptr<fims_model::Model<Type>> model =
      fims_model::Model<Type>::GetInstance();
  // get the singleton instance for Information Class
  std::shared_ptr<fims_info::Information<Type>> information =
      fims_info::Information<Type>::GetInstance();

  if (p.size() != information->fixed_effects_parameters.size())
  {
    Rf_error(
        "Fixed-parameter length mismatch in objective_function: parameter vector has size %zu but Information has %zu registered fixed parameters.",
        static_cast<size_t>(p.size()),
        information->fixed_effects_parameters.size());
  }

  if (re.size() != information->random_effects_parameters.size())
  {
    Rf_error(
        "Random-effects length mismatch in objective_function: random vector has size %zu but Information has %zu registered random effects.",
        static_cast<size_t>(re.size()),
        information->random_effects_parameters.size());
  }

  // update the fixed effects parameter values
  for (size_t i = 0; i < information->fixed_effects_parameters.size(); i++)
  {
    if (information->fixed_effects_parameters[i] == nullptr)
    {
      Rf_error("Null fixed-parameter pointer encountered at index %zu.", i);
    }
    *information->fixed_effects_parameters[i] = p[i];
  }
  // update the random effects parameter values
  for (size_t i = 0; i < information->random_effects_parameters.size(); i++)
  {
    if (information->random_effects_parameters[i] == nullptr)
    {
      Rf_error("Null random-effects pointer encountered at index %zu.", i);
    }
    *information->random_effects_parameters[i] = re[i];
  }
  model->of = this;

  Type nll = 0;
  // evaluate the model objective function value
  try
  {
    nll = model->Evaluate();
  }
  catch (const std::exception &e)
  {
    Rf_error("Error during model evaluation: %s", std::string(e.what()).c_str());
  }

  return nll;
}
/// @endcond
