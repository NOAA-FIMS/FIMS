#include "test_dnorm_likelihood.hpp"

template<class Type>
Type objective_function<Type>::operator()(){

  fims::Model<Type>* inst = fims::Model<Type>::getInstance();

  DATA_VECTOR(y);
  PARAMETER_VECTOR(p);
  Type mean = p[0];
  Type sd = exp(p[1]);
  inst -> y = y;
  inst -> mean = mean;
  inst -> sd = sd;
  Type nll = inst -> evaluate();
  return nll;
}
