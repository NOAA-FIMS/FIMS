
//[[Rcpp::depends(TMB)]]
#include "../inst/include/interface/RTMB.h"
#include "../inst/include/population_dynamics/selectivity/selectivity.hpp"


// [[Rcpp::export]]
ADrep evaluate_logistic_selectivity(ADrep x, ADrep input_slope, ADrep input_inflection_point){
    fims_popdy::LogisticSelectivity<ad> LogSel;
    // inflection_point and slope are fims::Vector<Type>
    // initial_value_m is a double
    const ad* IP = adptr(input_inflection_point);
    LogSel.inflection_point.resize(1);
    LogSel.inflection_point[0] = *IP;
    LogSel.slope.resize(1);
    const ad* Slope = adptr(input_slope);
    LogSel.slope[0] = *Slope;
    const ad* X = adptr(x);
    int n = x.size();
    ADrep ans(n); 
    ad* Y = adptr(ans); 
    for(int i=0; i<n; i++){
        Y[i] = LogSel.evaluate(X[i]);
    }
    return ans; 
}