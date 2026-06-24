# --- Helper Functions (from functions block) ---

# Pella-Tomlinson step
pella_tomlinson <- function(P0, r, K, m, C) {
  m1 <- m - 1
  surplus_prod <- r / m1 * P0 * (1 - P0^m1)
  P1 <- P0 + surplus_prod - C / K
  return(P1)
}

pt_pmsy <- function(m) {
  m^(-1 / (m - 1))
}
pt_bmsy <- function(K, m) {
  PMSY <- m^(-1 / (m - 1))
  K * PMSY
}
pt_fmsy <- function(r, m) {
  r / m
}
pt_msy <- function(fmsy, bmsy) {
  fmsy * bmsy
}

dinvgamma <- function(x, shape, scale, logscale = TRUE) {
  ret <- shape * log(scale) - lgamma(shape) - (shape + 1) * log(x) - scale / x
  if (logscale) {
    return(ret)
  } else {
    return(exp(ret))
  }
}


stan_code <- "
functions {
  // Define a single step forward in time using Pella-Tomlinson dynamics. This
  // uses the parameterization presented in Winker et al. 2018.
  real pella_tomlinson(real P0, real r, real K, real m, real C) {
    real m1;
    real surplus_prod;
    real P1;

    m1 = m - 1;
    surplus_prod = r / m1 * P0 * (1 - P0 ^ m1);
    P1 = P0 + surplus_prod - C / K;

    return P1;
  }

  // Calculate PMSY
  real pt_pmsy(real m) {
    return m ^ (-1 / (m - 1));
  }

  // Calculate BMSY
  real pt_bmsy(real K, real m) {
    real PMSY;
    PMSY = m ^ (-1 / (m - 1));
    return K * PMSY;
  }

  // Calculate FMSY
  real pt_fmsy(real r, real m) {
    return r / m;
  }

  // Use above to calculate MSY
  real pt_msy(real fmsy, real bmsy) {
    return fmsy * bmsy;
  }
}

data {
  int<lower=0> T;                  // Number of years
  vector[T] C;                     // Observed catch
  vector[T] I;                     // CPUE index
  int<lower=0> m;                  // Shape parameter
}

parameters {
  real<lower=0> r;                 // Population growth
  real<lower=0> K;                 // Carrying capacity
  real<lower=0> sigma2;            // Process variability
  real<lower=0> tau2;              // Observation variability
  vector<lower=0>[T] P;            // Predicted depletion
}

transformed parameters {
  real<lower=0> sigma;             // Process standard deviation
  real<lower=0> tau;               // Observation standard deviation
  vector[T] P_med;                 // Median depletion; no process error
  vector[T] Z;                     // Per year catchability; follow the notation
                                   // of Walters and Ludwig 1994
  real log_q_hat;                  // Estimate of q

  // Priors from Meyer & Millar 1999 are on the variance (originally precision)
  // parameter, but Stan takes a standard deviation parameter.
  sigma = sqrt(sigma2);
  tau = sqrt(tau2);

  // Initial depletion and catch
  P_med[1] = 1;
  for (t in 2:T) {
    // Note `fmax` here to keep depletion positive!
    P_med[t] = fmax(pella_tomlinson(P[t - 1], r, K, m, C[t - 1]),
                    0.001);
  }

  // Calculate log q for each year
  for (t in 1:T) {
    Z[t] = log(I[t]) - log(P[t]) - log(K);
  }
  log_q_hat = mean(Z);
}

model {
  // Priors specified in Meyer and Millar 1999
  r ~ lognormal(-1.38, 1 / sqrt(3.845));
  K ~ lognormal(5.042905, 1 / sqrt(3.7603664));
  sigma2 ~ inv_gamma(3.785518, 0.010223);
  tau2 ~ inv_gamma(1.708603, 0.008613854);
  // Prior on m with Jacobian correction

  // Process likelihood
  P ~ lognormal(log(P_med), sigma);
  // Observation likelihood
  /* NOTE: This will give a warning about using a transformed variable on the
   * left-hand side of a sapling statement. I'm pretty sure I don't need a
   * Jacobian adjustment here, but this is definitely the line causing the
   * warning.*/
  Z ~ normal(log_q_hat, tau);
}

generated quantities {
  vector[T + 1] Biomass;           // Biomass series with one step ahead prediction
  real P_medfinal;                 // One step ahead median depletion
  real P_final;                    // One step ahead depletion
  real PMSY;                       // Depletion at MSY
  real BMSY;                       // Biomass at MSY
  real FMSY;                       // Fishing mortality to achieve MSY
  real MSY;                        // Maximum sustainable yield
  real ll_r;                      // Log-likelihood component for r
  real ll_K;                      // Log-likelihood component for K
  real ll_sigma2;                 // Log-likelihood component for sigma2
  real ll_tau2;                   // Log-likelihood component for tau2
  real ll_P;                      // Log-likelihood component for P
  real ll_Z;                      // Log-likelihood component for Z

  // Calculate biomass at each time step from depletion and K, then simulate
  // one step ahead and include that final biomass
  for (t in 1:T) {
    Biomass[t] = K * P[t];
  }
  // One-step-ahead projection, including process error
  P_medfinal = fmax(pella_tomlinson(P[T], r, K, m, C[T]),
                    0.001);
  P_final = lognormal_rng(log(P_medfinal), sigma);
  Biomass[T + 1] = K * P_final;

  // Management values
  PMSY = pt_pmsy(m);
  BMSY = K * PMSY;
  FMSY = pt_fmsy(r, m);
  MSY  = pt_msy(BMSY, FMSY);

  //calculate log-likelihood components
  ll_r = lognormal_lpdf(r | -1.38, 1 / sqrt(3.845));
  ll_K = lognormal_lpdf(K | 5.042905, 1 / sqrt(3.7603664));
  ll_sigma2 = inv_gamma_lpdf(sigma2 | 3.785518, 0.010223);
  ll_tau2 = inv_gamma_lpdf(tau2 | 1.708603, 0.008613854);
  ll_P = lognormal_lpdf(P | log(P_med), sigma);
  ll_Z = normal_lpdf(Z | log_q_hat, tau);
}
"

################################################################################
# models for testing different parameterizations and features

np <- TRUE

if (np == FALSE) {
  model_c_isig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- exp(par$log_r)
    K <- exp(par$log_K)
    m <- 2.0
    isigma2 <- exp(par$log_isigma2)
    itau2 <- exp(par$log_itau2)
    sigma2 <- 1 / isigma2
    tau2 <- 1 / itau2
    sigma <- sqrt(sigma2)
    tau <- sqrt(tau2)
    P <- exp(par$log_P)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    # nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    # nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)
    nll_sig2 <- -dgamma(x = isigma2, shape = 3.785518, rate = 0.010223, log = TRUE) + log(sigma2^2)
    nll_tau2 <- -dgamma(x = itau2, shape = 1.708603, rate = 0.008613854, log = TRUE) + log(tau2^2)

    # Process likelihood
    nll_P <- -sum(dlnorm(P[1:Tlen], meanlog = log(P_med[1:Tlen]), sd = sigma, log = TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z

    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_c_sig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- exp(par$log_r)
    K <- exp(par$log_K)
    m <- 2.0
    sigma2 <- exp(par$log_sigma2)
    tau2 <- exp(par$log_tau2)
    sigma <- sqrt(sigma2)
    tau <- sqrt(tau2)
    P <- exp(par$log_P)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)

    # Process likelihood
    nll_P <- -sum(dlnorm(P[1:Tlen], meanlog = log(P_med[1:Tlen]), sd = sigma, log = TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z

    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_isig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- exp(par$log_r)
    K <- exp(par$log_K)
    m <- 2.0
    z <- par$z
    isigma2 <- exp(par$log_isigma2)
    sigma2 <- 1 / isigma2
    itau2 <- exp(par$log_itau2)
    tau2 <- 1 / itau2
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dgamma(x = isigma2, shape = 3.785518, rate = 0.010223, log = TRUE) + log(sigma2^2)
    nll_tau2 <- -dgamma(x = itau2, shape = 1.708603, rate = 0.008613854, log = TRUE) + log(tau2^2)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_sig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- exp(par$log_r)
    K <- exp(par$log_K)
    m <- 2.0
    z <- par$z
    sigma2 <- exp(par$log_sigma2)
    tau2 <- exp(par$log_tau2)
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_isig_p1 <- function(par) {
    #### Parameters and reparameterizations ####
    r <- exp(par$log_r)
    K <- exp(par$log_K)
    m <- 2.0
    z <- par$z
    isigma2 <- exp(par$log_isigma2)
    sigma2 <- 1 / isigma2
    itau2 <- exp(par$log_itau2)
    tau2 <- 1 / itau2
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen + 1))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen + 1)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dgamma(x = isigma2, shape = 3.785518, rate = 0.010223, log = TRUE) + log(sigma2^2)
    nll_tau2 <- -dgamma(x = itau2, shape = 1.708603, rate = 0.008613854, log = TRUE) + log(tau2^2)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen + 1)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_sig_p1 <- function(par) {
    #### Parameters and reparameterizations ####
    r <- exp(par$log_r)
    K <- exp(par$log_K)
    m <- 2.0
    z <- par$z
    sigma2 <- exp(par$log_sigma2)
    tau2 <- exp(par$log_tau2)
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen + 1))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen + 1)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen + 1)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }
}

################################################################################
# redo tests with natural parameters

if (np == TRUE) {
  model_c_isig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    isigma2 <- par$isigma2
    itau2 <- par$itau2
    sigma2 <- 1 / isigma2
    tau2 <- 1 / itau2
    sigma <- sqrt(sigma2)
    tau <- sqrt(tau2)
    P <- par$P

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dgamma(x = isigma2, shape = 3.785518, rate = 0.010223, log = TRUE) + log(sigma2^2)
    nll_tau2 <- -dgamma(x = itau2, shape = 1.708603, rate = 0.008613854, log = TRUE) + log(tau2^2)

    # Process likelihood
    nll_P <- -sum(dlnorm(P[1:Tlen], meanlog = log(P_med[1:Tlen]), sd = sigma, log = TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z

    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_c_sig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    sigma2 <- par$sigma2
    tau2 <- par$tau2
    sigma <- sqrt(sigma2)
    tau <- sqrt(tau2)
    P <- par$P

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)

    # Process likelihood
    nll_P <- -sum(dlnorm(P[1:Tlen], meanlog = log(P_med[1:Tlen]), sd = sigma, log = TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z

    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_isig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    z <- par$z
    isigma2 <- par$isigma2
    sigma2 <- 1 / isigma2
    itau2 <- par$itau2
    tau2 <- 1 / itau2
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dgamma(x = isigma2, shape = 3.785518, rate = 0.010223, log = TRUE) + log(sigma2^2)
    nll_tau2 <- -dgamma(x = itau2, shape = 1.708603, rate = 0.008613854, log = TRUE) + log(tau2^2)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_sig <- function(par) {
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    z <- par$z
    sigma2 <- par$sigma2
    tau2 <- par$tau2
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_isig_p1 <- function(par) {
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    z <- par$z
    isigma2 <- par$isigma2
    sigma2 <- 1 / isigma2
    itau2 <- par$itau2
    tau2 <- 1 / itau2
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen + 1))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen + 1)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dgamma(x = isigma2, shape = 3.785518, rate = 0.010223, log = TRUE) + log(sigma2^2)
    nll_tau2 <- -dgamma(x = itau2, shape = 1.708603, rate = 0.008613854, log = TRUE) + log(tau2^2)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen + 1)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }

  model_nc_sig_p1 <- function(par) {
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    z <- par$z
    sigma2 <- par$sigma2
    tau2 <- par$tau2
    tau <- sqrt(tau2)
    sigma <- sqrt(sigma2)

    Tlen <- sp_data["T"]$T

    #### Transformed parameters ####
    P_med <- Biomass <- P <- rep(0, (Tlen + 1))
    Z <- rep(0, Tlen)

    # Initial depletion and catch
    P_med[1] <- 1
    P[1] <- exp(log(P_med[1]) + sigma * z[1])
    for (i in 2:(Tlen + 1)) {
      P_med[i] <- max(pella_tomlinson(P[i - 1], r, K, m, sp_data$C[i - 1]), 0.001)
      P[i] <- exp(log(P_med[i]) + sigma * z[i])
    }
    # Calculate "log q" for each year
    for (i in 1:Tlen) {
      Z[i] <- log(sp_data$I[i]) - log(P[i]) - log(K)
    }
    log_q_hat <- mean(Z)

    #### Log-likelihood ####
    jnll <- 0
    nll_r <- 0
    nll_K <- 0
    nll_P <- 0
    nll_Z <- 0
    nll_sig2 <- 0
    nll_tau2 <- 0

    # Priors (as in Stan)
    nll_r <- -dlnorm(r, meanlog = -1.38, sdlog = 1 / sqrt(3.845), log = TRUE)
    nll_K <- -dlnorm(K, meanlog = 5.042905, sdlog = 1 / sqrt(3.7603664), log = TRUE)
    nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
    nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE)

    # Process likelihood
    nll_P <- -sum(dnorm(z, 0, 1, TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean = log_q_hat, sd = tau, log = TRUE))

    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z
    for (i in 1:(Tlen + 1)) {
      Biomass[i] <- K * P[i]
    }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY <- pt_msy(BMSY, FMSY)

    # Report section
    REPORT(nll_r)
    REPORT(nll_K)
    REPORT(nll_sig2)
    REPORT(nll_tau2)
    REPORT(nll_P)
    REPORT(nll_Z)
    REPORT(jnll)
    REPORT(P)
    REPORT(P_med)
    REPORT(Z)
    REPORT(log_q_hat)
    REPORT(r)
    REPORT(K)
    REPORT(sigma2)
    REPORT(tau2)
    REPORT(Biomass)
    REPORT(PMSY)
    REPORT(BMSY)
    REPORT(FMSY)
    REPORT(MSY)

    return(jnll)
  }
}
