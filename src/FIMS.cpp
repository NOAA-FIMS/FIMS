#include <cmath>
#include "../inst/include/interface/interface.hpp"
#include "init.hpp"

/*
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template<class Type>
Type objective_function<Type>::operator() ()
{


  PARAMETER_VECTOR(p);

  Type nll = 0;

  return nll;

}
