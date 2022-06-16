
#include <cmath>
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
#include "../inst/include/interface/init.hpp"



// Double version of Lambert W function
double LambertW(double x) {
  double logx = log(x);
  double y = (logx > 0 ? logx : 0);
  int niter = 100, i=0;
  for (; i < niter; i++) {
    if ( fabs( logx - log(y) - y) < 1e-9) break;
    y -= (y - exp(logx - y)) / (1 + y);
  }
  if (i == niter) Rf_warning("W: failed convergence");
  return y;
}
TMB_ATOMIC_VECTOR_FUNCTION(
    // ATOMIC_NAME
    LambertW
    ,
    // OUTPUT_DIM
    1,
    // ATOMIC_DOUBLE
    ty[0] = LambertW(tx[0]); // Call the 'double' version
    ,
    // ATOMIC_REVERSE
    Type W  = ty[0];                    // Function value from forward pass
    Type DW = 1. / (exp(W) * (1. + W)); // Derivative
    px[0] = DW * py[0];                 // Reverse mode chain rule
)
// Scalar version
template<class Type>
Type LambertW(Type x){
  CppAD::vector<Type> tx(1);
  tx[0] = x;
  return LambertW(tx)[0];
}
// Vectorized version
VECTORIZE1_t(LambertW)



/*
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template<class Type>
Type objective_function<Type>::operator()() {


    PARAMETER_VECTOR(x);

    Type nll = LambertW(x).sum();

    return nll;

}
