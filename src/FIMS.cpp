
#include <cmath>
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
#include "../inst/include/interface/init.hpp"

/// @cond 
/**
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template<class Type>
Type objective_function<Type>::operator()() {


    PARAMETER_VECTOR(p);

    fims::Population<Type> population;
    population.Evaluate();
    

    Type nll = 0;

    return nll;

}
/// @endcond