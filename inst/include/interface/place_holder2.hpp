#ifndef MODEL_HPP
#define MODEL_HPP
#include <vector>
#include "place_holder0.hpp"
#include "place_holder1.hpp"
  
  
template<class Type>
class logisticGrowth{
  
  using DataVector = typename model_traits<Type>::data_vector;
  using ParameterVector = typename model_traits<Type>::parameter_vector;
  
public:
  // /* Data section */
  DataVector  y;
  // /* Parameter section */
  ParameterVector u;
  Type r;
  Type K;
  //ParameterVector theta;
  Type ln_sig;
  Type ln_tau;
  static logisticGrowth<Type>* instance;

  
  logisticGrowth(){}
  
  static logisticGrowth<Type>* getinstance(){
    return logisticGrowth<Type>::instance;
  }
  
  ParameterVector calculateEta(ParameterVector u, Type r, Type K){
    
    ParameterVector eta(u.size());
    
    for(int t=1; t<u.size(); t++){
      eta[t] = u[t-1] + r * u[t-1] * (1-u[t-1]/K);
    }
  
    return eta; 
  }
  
  
  Type calculateNll(ParameterVector u, ParameterVector eta, Type sigma, Type tau, DataVector y){
  
    Type nll = 0.0; 
    int n=y.size();
    
    for(int t=1; t<n; t++){
      nll -= dlognorm(u[t], log(eta[t]), sigma);
    }
    
    for(int t=0; t<n; t++){
      nll -= dlognorm(y[t], log(u[t]), tau);
    }
  
    return nll;  
  }
  
  Type evaluate(){
    //Type r = exp(theta[0]);
    //Type K = exp(theta[1]);
    Type sigma = exp(ln_sig);
    Type tau = exp(ln_tau);
    
    auto eta = calculateEta(u, r, K); //auto is used to automatically figure out the Type of eta that is defined by calculateEta
    Type nll = calculateNll(u, eta, sigma, tau, y);

    return nll;
  }

};

template<class Type>
logisticGrowth<Type>* logisticGrowth<Type>::instance = new logisticGrowth<Type>();

#endif
