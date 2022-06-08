#include "test_dlnorm_likelihood.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  
  fims::Model<Type>* inst = fims::Model<Type>::getInstance();

  DATA_VECTOR(logy);
  PARAMETER_VECTOR(p);
  Type meanlog = p[0];
  Type sdlog = exp(p[1]);
  inst -> logy = logy;
  inst -> meanlog = meanlog;
  inst -> sdlog = sdlog;
  Type nll = inst -> evaluate();
  return nll;
}
