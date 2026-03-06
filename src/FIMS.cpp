

#undef isNull
// #include <Rcpp.h>
// #include <cmath>

// // #include "../inst/include/interface/rcpp/rcpp_interface.hpp"
// // #include "../inst/include/interface/interface.hpp"
// #include "init.hpp"
// FIMS_DEPENDS(fims_modules.hpp);
// // #include "../inst/include/common/model.hpp"
// // #include "FIMS.h"
// // FIMS_DEPENDS(init.hpp);
// // FIMS_DEPENDS(fims_modules.hpp);
// FIMS_DEPENDS(rcpp_interface.hpp);
// FIMS_DEPENDS(interface.hpp);
// //  FIMS_DEPENDS(model.hpp);
// #include "F_common_model.h"
// #ifndef TMB_HPP
// #define TMB_HPP
// #include <TMB.hpp>
// #endif
#include "FIMS_all.hpp"





namespace fims {
namespace adapter {

// Convert TMB vector -> FIMS vector
template<typename Type>
fims::Vector<Type> from_tmb(const tmbutils::vector<Type>& x) {
    Vector<Type> out(x.size());
    for (int i = 0; i < x.size(); ++i) {
        out[i] = x[i];
    }
    return out;
}

// Convert FIMS vector -> TMB vector
template<typename Type>
tmbutils::vector<Type> to_tmb(const fims::Vector<Type>& x) {
    tmbutils::vector<Type> out(x.size());
    for (int i = 0; i < x.size(); ++i) {
        out[i] = x[i];
    }
    return out;
}

} // namespace adapter
} // namespace fims




// #define ADREPORT_F(name, F) F->reportvector.push(name, #name);
// #define FIMS_SIMULATE_F(F) if (isDouble<Type>::value && F->do_simulate)


template <typename Type>
tmbutils::vector<Type> ADREPORTvector(tmbutils::vector<tmbutils::vector<Type> > x) {
  int outer_dim = x.size();
  int dim = 0;
  for (int i = 0; i < outer_dim; i++) {
    dim += x(i).size();
  }
  vector<Type> res(dim);
  int idx = 0;
  for (int i = 0; i < outer_dim; i++) {
    int inner_dim = x(i).size();
    for (int j = 0; j < inner_dim; j++) {
      res(idx) = x(i)(j);
      idx += 1;
    }
  }
  return res;
}

/// @cond
/**
 * @brief TMB objective function
 *
 * @return Returns a joint negative log likelihood
 */
template<class Type>
Type objective_function<Type>::operator()() {


    PARAMETER_VECTOR(p);
    PARAMETER_VECTOR(re);

    // code below copied from ModularTMBExample/src/tmb_objective_function.cpp

    // get the singleton instance for Model Class
    std::shared_ptr<fims_model::Model<Type>> model =
      fims_model::Model<Type>::GetInstance();
    // get the singleton instance for Information Class
    std::shared_ptr<fims_info::Information<Type>> information =
      fims_info::Information<Type>::GetInstance();

    //update the fixed effects parameter values
    for(size_t i =0; i < information->fixed_effects_parameters.size(); i++){
        *information->fixed_effects_parameters[i] = p[i];
    }
    //update the random effects parameter values
    for(size_t i =0; i < information->random_effects_parameters.size(); i++){
        *information->random_effects_parameters[i] = re[i];
    }
    model -> of = this;

    Type nll = 0;
    //evaluate the model objective function value
    try{
      nll = model->Evaluate();
    } catch (const std::exception& e) {
      Rf_error("Error during model evaluation: %s",  std::string(e.what()).c_str());
    }

    return nll;

}
/// @endcond
