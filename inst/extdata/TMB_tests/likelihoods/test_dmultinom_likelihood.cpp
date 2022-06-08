#include "test_dmultinom_likelihood.hpp"

template<class Type>
Type objective_function<Type>::operator()(){
  
  fims::Model<Type>* inst = fims::Model<Type>::getInstance();

  DATA_VECTOR(y);
  PARAMETER_VECTOR(p);
  inst -> x = x;
  inst -> p = p;
  inst -> sd = sd;
  Type nll = inst -> evaluate();
  return nll;
}
