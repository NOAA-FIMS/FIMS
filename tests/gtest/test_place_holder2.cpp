#include "gtest/gtest.h"
#include "interface/place_holder2.hpp"

// # R code that generates true values for the test
// u <- c(5000, 6000, 7000, 5500)
// r <- 0.5
// K <- 10000
// n <- length(u) # 4
// eta <- c()
// for (t in 2:n) {
//   eta[t] <- u[t - 1] + r * u[t - 1] * (1 - u[t - 1] / K) # NA 6250 7200 8050
// }
// 
// y <- c(5050, 5950, 6950, 6000)
// sigma <- 0.1
// tau <- 0.03
// nll <- 0
// 
// for (t in 2:n) {
//   nll <- nll - dlnorm(u[t], log(eta[t]), sigma, TRUE)
// }
// 
// for (t in 1:n) {
//   nll <- nll - dlnorm(y[t], log(u[t]), tau, TRUE) # 58.13528
// } 
// 
// theta <- c(log(r), log(K)) #-0.6931472  9.2103404
// ln_sig <- log(sigma) # -2.302585
// ln_tau <- log(tau) # -3.506558

namespace {

  // Test eta calculation
  TEST(modelTest, eta) {
    
    // Test with a vector of u
    logisticGrowth<double>* inst = new logisticGrowth<double>(); //non-singleton usage
    
    model_traits<double>::parameter_vector u = {5000, 6000, 7000, 5500};
    double r = 0.5;
    double K = 10000;
    
    auto eta_val = inst->calculateEta(u, r, K);
    
    model_traits<double>::parameter_vector true_eta = {0, 6250, 7200, 8050};
    
    EXPECT_EQ( eta_val , true_eta );
    
    // Test with an empty u
    u = {};
    
    eta_val = inst->calculateEta(u, r, K);
    true_eta = {};
    
    EXPECT_EQ( eta_val , true_eta );
    
    
    // Test with a different r value
    u = {5000, 6000, 7000, 5500};
    r = 1.5;
    eta_val = inst->calculateEta(u, r, K);
    
    true_eta = {0, 6250, 7200, 8050};
    
    EXPECT_NE( eta_val , true_eta );
  }
  
  // Test nll calculation
  TEST(modelTest, nll) {
    
    logisticGrowth<double>* inst = new logisticGrowth<double>(); //non-singleton usage
    
    model_traits<double>::parameter_vector u = {5000, 6000, 7000, 5500};
    model_traits<double>::parameter_vector eta = {0, 6250, 7200, 8050};
    double sigma = 0.1;
    double tau = 0.03;
    model_traits<double>::data_vector y={5050, 5950, 6950, 6000};
    
    auto nll_val = inst->calculateNll(u, eta, sigma, tau, y);
    double true_nll = 58.13528;
    
    EXPECT_NEAR( nll_val , true_nll , 0.0001 );
    
    // Use a different sigma value and expect nll_val and true_nll have different values
    sigma = 0.3;
    nll_val = inst->calculateNll(u, eta, sigma, tau, y);
    EXPECT_NE( nll_val , true_nll );
  }
  
  // Test evaluate
  TEST(modelTest, evaluate) {
    logisticGrowth<double>* inst = new logisticGrowth<double>(); //non-singleton usage
    
    model_traits<double>::parameter_vector theta = {-0.6931472, 9.2103404};
    double ln_sig = -2.302585;
    double ln_tau = -3.506558;
    double r = 0.5;
    double K = 10000;
    
    model_traits<double>::parameter_vector u={5000, 6000, 7000, 5500};
    model_traits<double>::data_vector y={5050, 5950, 6950, 6000};
    
    double true_nll = 58.13528;
    
    inst->y = y;
    //inst->theta = theta;
    inst->r = r;
    inst->K = K;
    inst->ln_sig = ln_sig;
    inst->ln_tau = ln_tau;
    inst->u = u;
    
    double nll_val = inst->evaluate();
    
    EXPECT_NEAR( nll_val , true_nll , 0.0001 );
    
    // Use a different ln_sig input value and expect nll_val and true_nll have different values
    inst->ln_sig = -5;
    nll_val = inst->evaluate();
    EXPECT_NE( nll_val , true_nll );
    
  }
  
}


