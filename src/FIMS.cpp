
//#include "../inst/include/interface/interface.hpp"

#include "../inst/include/interface/rcpp_interface.hpp"
#include "init.hpp"










//void hello_fims(){
//std::cout<<"hello fims";
//}
//
//RCPP_MODULE(fims) {
//    //place holder for module elements
//    Rcpp::function("hello_fims", hello_fims);
//};


#include <cmath>
#include "../inst/include/interface/interface.hpp"
#include "init.h"


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
