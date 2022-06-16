
#include <cmath>
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
#include "../inst/include/interface/init.hpp"

// Double version of InitialF function

double InitialF(double x) {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > info = 
            fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
    
    return info->InitialFDummyFunction();
}

TMB_ATOMIC_VECTOR_FUNCTION(
        // ATOMIC_NAME
        InitialF
        ,
        // OUTPUT_DIM
        1,
        // ATOMIC_DOUBLE
        ty[0] = InitialF(tx[0]); // Call the 'double' version
        ,
        // ATOMIC_REVERSE
        Type W = ty[0]; // Function value from forward pass
        Type DW = 1.; // Derivative
        px[0] = DW * py[0]; // Reverse mode chain rule
        )
// Scalar version
template<class Type>
Type InitialF(Type x) {
    CppAD::vector<Type> tx(1);
    tx[0] = x;
    return InitialF(tx)[0];
}
// Vectorized version
VECTORIZE1_t(InitialF)



template<class Type>
Type objective_function<Type>::operator()() {
    PARAMETER_VECTOR(x);
    std::cout<<x<<"  "<<InitialF(x)<<"\n";
    Type f = InitialF(x).sum();
    return f;
}
