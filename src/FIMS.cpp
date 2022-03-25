#include "../inst/include/interface/interface.hpp"
#include "init.hpp"

#define RCPP_NO_SUGAR
#include <Rcpp.h>

void hello_fims(){
std::cout<<"hello fims";
}

RCPP_MODULE(fims) {
    //place holder for module elements
    Rcpp::function("hello_fims", hello_fims);
};


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
