library(rstan)

tuna.dat <- rbind(
  c(15.9, 61.89),
  c(25.7, 78.98),
  c(28.5, 55.59),
  c(23.7, 44.61),
  c(25.0, 56.89),
  c(33.3, 38.27),
  c(28.2, 33.84),
  c(19.7, 36.13),
  c(17.5, 41.95),
  c(19.3, 36.63),
  c(21.6, 36.33),
  c(23.1, 38.82),
  c(22.5, 34.32),
  c(22.5, 37.64),
  c(23.6, 34.01),
  c(29.1, 32.16),
  c(14.4, 26.88),
  c(13.2, 36.61),
  c(28.4, 30.07),
  c(34.6, 30.75),
  c(37.5, 23.36),
  c(25.9, 22.36),
  c(25.3, 21.91)
)
colnames(tuna.dat) <- c("C", "I")
tuna.dat <- as.data.frame(tuna.dat)


stan_data <- list(
  T = nrow(tuna.dat),
  C = tuna.dat$C,
  I = tuna.dat$I,
  m = 2
)

inits <- list(
  depletion = c(
    0.99, 0.98, 0.96, 0.94, 0.92, 0.90, 0.88, 0.86, 0.84, 0.82,
    0.80, 0.78, 0.76, 0.74, 0.72, 0.70, 0.68, 0.66, 0.64, 0.62, 0.60, 0.58, 0.56, 0.56
  ),
  r = 0.8, K = 200, sigma2 = 1 / 100, tau2 = 1 / 100
)

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


fit <- rstan::stan(
  model_code = stan_code, data = stan_data,
  warmup = 5000, iter = 30000, chains = 4,
  control = list(adapt_delta = 0.99)
)
postmle <- as.matrix(fit)[, -ncol(as.matrix(fit))]
out_par <- apply(postmle, 2, median)

df <- data.frame(label = rep("", 29), median = rep(NA, 29))
df[, 1] <- c(
  "growth_rate", "carrying_capacity", "sigma2_obs", "sigma2_depletion",
  rep("depletion", 24), "q"
)

df[, 2] <- c(
  out_par[["r"]], out_par[["K"]], out_par[["tau2"]], out_par[["sigma2"]],
  out_par[5:27], out_par[["P_final"]], out_par[["log_q_hat"]] |> exp()
)

data_limited_tuna_results <- df

usethis::use_data(data_limited_tuna_results, overwrite = TRUE)
