library(RTMB)
source("tests/testthat/stan_model.R")

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
  c(25.3, 21.91))
colnames(tuna.dat) <- c('C', 'I')
tuna.dat <- as.data.frame(tuna.dat)

sp_data <- list(T = nrow(tuna.dat),
                C = tuna.dat$C,
                I = tuna.dat$I)  # Example prior on m

inits <- list(depletion=c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56),
r=0.8, K=200, sigma2 = 1/100, tau2 = 1/100)

inits_p1 <- list(depletion=c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56, 0.56),
r=0.8, K=200, sigma2 = 1/100, tau2 = 1/100)

init_z <- rep(0, nrow(tuna.dat))
init_z[1] = (log(inits$depletion[1]) - log(1)) / sqrt(inits$sigma2)
for (i in 2:(nrow(tuna.dat))) {
  init_z[i] = (log(inits$depletion[i]) - log(pella_tomlinson(inits$depletion[i - 1], 
    inits$r, inits$K, 2, tuna.dat$C[i - 1]))) / sqrt(inits$sigma2)
}

init_z_p1 <- rep(0, nrow(tuna.dat)+1)
init_z_p1[1] = (log(inits_p1$depletion[1]) - log(1)) / sqrt(inits_p1$sigma2)
for (i in 2:(nrow(tuna.dat)+1)) {
  init_z_p1[i] = (log(inits_p1$depletion[i]) - log(pella_tomlinson(inits_p1$depletion[i - 1], 
    inits_p1$r, inits_p1$K, 2, tuna.dat$C[i - 1]))) / sqrt(inits_p1$sigma2)
}

pars_nc_sig = list(log_r=log(inits$r), log_K=log(inits$K), 
                 log_sigma2=log(inits$sigma2), 
                 log_tau2=log(inits$tau2), 
                 z = init_z)

                 #log_P=log(inits$depletion))
pars_c_sig = list(log_r=log(inits$r), log_K=log(inits$K), 
                 log_sigma2=log(inits$sigma2), 
                 log_tau2=log(inits$tau2),
                 log_P=log(inits$depletion))
pars_nc_isig = list(log_r=log(inits$r), log_K=log(inits$K), 
                 log_isigma2=log(1/inits$sigma2), 
                 log_itau2=log(1/inits$tau2), 
                 z = init_z)
                 #log_P=log(inits$depletion))
pars_c_isig = list(log_r=log(inits$r), log_K=log(inits$K), 
                 log_isigma2=log(1/inits$sigma2), 
                 log_itau2=log(1/inits$tau2),
                 log_P=log(inits$depletion))
pars_nc_sig_p1 = list(log_r=log(inits_p1$r), log_K=log(inits_p1$K), 
                 log_sigma2=log(inits_p1$sigma2), 
                 log_tau2=log(inits_p1$tau2), 
                 z = init_z_p1)
pars_nc_isig_p1 = list(log_r=log(inits_p1$r), log_K=log(inits_p1$K), 
                 log_isigma2=log(1/inits_p1$sigma2), 
                 log_itau2=log(1/inits_p1$tau2), 
                 z = init_z_p1)

obj_nc_sig <- MakeADFun(model_nc_sig,
                 parameters = pars_nc_sig)
obj_c_sig <- MakeADFun(model_c_sig,
                 parameters = pars_c_sig)
obj_nc_isig <- MakeADFun(model_nc_isig,
                 parameters = pars_nc_isig)
obj_c_isig <- MakeADFun(model_c_isig,
                 parameters = pars_c_isig)
obj_nc_sig_p1 <- MakeADFun(model_nc_sig_p1,
                 parameters = pars_nc_sig_p1)
obj_nc_isig_p1 <- MakeADFun(model_nc_isig_p1,
                 parameters = pars_nc_isig_p1)

fit_nc_sig <- fit_c_sig <- fit_nc_isig <- fit_c_isig <- list()

for(i in 1:10){
  set.seed(1234 + i)
  fit_nc_sig[[i]] <- tmbstan::tmbstan(obj_nc_sig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
        control = list(adapt_delta = 0.99))
  fit_c_sig[[i]] <- tmbstan::tmbstan(obj_c_sig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
        control = list(adapt_delta = 0.99))
  fit_nc_isig[[i]] <- tmbstan::tmbstan(obj_nc_isig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
        control = list(adapt_delta = 0.99))
  fit_c_isig[[i]] <- tmbstan::tmbstan(obj_c_isig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
        control = list(adapt_delta = 0.99))
}

for(i in 1:10){
  print(rstan::traceplot(fit_nc_sig[[i]]))
  print(rstan::traceplot(fit_c_sig[[i]]))
  print(rstan::traceplot(fit_nc_isig[[i]]))
  print(rstan::traceplot(fit_c_isig[[i]]))
}

out_par_nc_sig <-
  apply(as.matrix(fit_nc_sig[[1]])[, -ncol(as.matrix(fit_nc_sig[[1]]))], 2, median)
report_nc_sig <- obj_nc_sig$report(out_par_nc_sig)
out_par_nc_sig_p1 <-
  apply(as.matrix(fit_nc_sig_p1[[24]])[, -ncol(as.matrix(fit_nc_sig_p1[[24]]))], 2, median)
report_nc_sig_p1 <- obj_nc_sig_p1$report(out_par_nc_sig_p1)

fit_rtmb_stan <- tmbstan::tmbstan(obj, init =  "last.par.best",
  warmup = 5000, iter = 30000, chains = 4, 
        control = list(adapt_delta = 0.99))
rtmb_postmle <- as.matrix(fit_rtmb_stan)[, -ncol(as.matrix(fit_rtmb_stan))]
out_par_rtmb <- apply(rtmb_postmle, 2, median)
report_rtmb <- obj$report(out_par_rtmb)
report_rtmb$sigma2
posterior_rtmb <- as.array(fit_rtmb_stan)
np_fit = bayesplot::nuts_params(fit_rtmb_stan)
bayesplot::mcmc_parcoord(posterior_rtmb, pars = "re[1]", np = np_fit)
bayesplot::mcmc_trace(posterior_rtmb, pars = colnames(rtmb_postmle)[1], np = np_fit)
rstan::traceplot(fit_rtmb_stan)


################################################################################
# redo with parameters in natural space and bounds
library(RTMB)
source("tests/testthat/stan_model.R")

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
  c(25.3, 21.91))
colnames(tuna.dat) <- c('C', 'I')
tuna.dat <- as.data.frame(tuna.dat)

sp_data <- list(T = nrow(tuna.dat),
                C = tuna.dat$C,
                I = tuna.dat$I)  # Example prior on m

inits <- list(depletion=c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56),
r=0.8, K=200, sigma2 = 1/100, tau2 = 1/100)

inits_p1 <- list(depletion=c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56, 0.56),
r=0.8, K=200, sigma2 = 1/100, tau2 = 1/100)

init_z <- rep(0, nrow(tuna.dat))
init_z[1] = (log(inits$depletion[1]) - log(1)) / sqrt(inits$sigma2)
for (i in 2:(nrow(tuna.dat))) {
  init_z[i] = (log(inits$depletion[i]) - log(pella_tomlinson(inits$depletion[i - 1], 
    inits$r, inits$K, 2, tuna.dat$C[i - 1]))) / sqrt(inits$sigma2)
}

init_z_p1 <- rep(0, nrow(tuna.dat)+1)
init_z_p1[1] = (log(inits_p1$depletion[1]) - log(1)) / sqrt(inits_p1$sigma2)
for (i in 2:(nrow(tuna.dat)+1)) {
  init_z_p1[i] = (log(inits_p1$depletion[i]) - log(pella_tomlinson(inits_p1$depletion[i - 1], 
    inits_p1$r, inits_p1$K, 2, tuna.dat$C[i - 1]))) / sqrt(inits_p1$sigma2)
}

pars_nc_sig <- list(r=inits$r, K=inits$K, 
                 sigma2=inits$sigma2, 
                 tau2=inits$tau2, 
                 z = init_z)
lower_nc_sig <- rep(-Inf, length(unlist(pars_nc_sig)))
lower_nc_sig[1:4] <- 0  # r, K, sigma2, tau2 > 0
upper_nc_sig <- rep(Inf, length(unlist(pars_nc_sig)))

pars_c_sig <- list(r=inits$r, K=inits$K, 
                 sigma2=inits$sigma2, 
                 tau2=inits$tau2,
                 P=inits$depletion)
lower_c_sig <- rep(0, length(unlist(pars_c_sig)))
upper_c_sig <- rep(Inf, length(unlist(pars_c_sig)))

pars_nc_isig <- list(r=inits$r, K=inits$K, 
                 isigma2=1/inits$sigma2, 
                 itau2=1/inits$tau2, 
                 z = init_z)
lower_nc_isig <- rep(-Inf, length(unlist(pars_nc_isig)))
lower_nc_isig[1:4] <- 0  # r, K, isigma2, itau2 > 0
upper_nc_isig <- rep(Inf, length(unlist(pars_nc_isig)))

pars_c_isig <- list(r=inits$r, K=inits$K, 
                 isigma2=1/inits$sigma2, 
                 itau2=1/inits$tau2,
                 P=inits$depletion)
lower_c_isig <- rep(0, length(unlist(pars_c_isig)))
upper_c_isig <- rep(Inf, length(unlist(pars_c_isig)))

pars_nc_sig_p1 <- list(r=inits_p1$r, K=inits_p1$K, 
                 sigma2=inits_p1$sigma2, 
                 tau2=inits_p1$tau2, 
                 z = init_z_p1)
lower_nc_sig_p1 <- rep(-Inf, length(unlist(pars_nc_sig_p1)))
lower_nc_sig_p1[1:4] <- 0  # r, K, sigma2, tau2 > 0
upper_nc_sig_p1 <- rep(Inf, length(unlist(pars_nc_sig_p1)))

pars_nc_isig_p1 <- list(r=inits_p1$r, K=inits_p1$K, 
                 isigma2=1/inits_p1$sigma2, 
                 itau2=1/inits_p1$tau2, 
                 z = init_z_p1)
lower_nc_isig_p1 <- rep(-Inf, length(unlist(pars_nc_isig_p1)))
lower_nc_isig_p1[1:4] <- 0  # r, K, isigma2, itau2 > 0
upper_nc_isig_p1 <- rep(Inf, length(unlist(pars_nc_isig_p1)))

obj_nc_sig <- MakeADFun(model_nc_sig,
                 parameters = pars_nc_sig)
obj_c_sig <- MakeADFun(model_c_sig,
                 parameters = pars_c_sig)
obj_nc_isig <- MakeADFun(model_nc_isig,
                 parameters = pars_nc_isig)
obj_c_isig <- MakeADFun(model_c_isig,
                 parameters = pars_c_isig)
obj_nc_sig_p1 <- MakeADFun(model_nc_sig_p1,
                 parameters = pars_nc_sig_p1)
obj_nc_isig_p1 <- MakeADFun(model_nc_isig_p1,
                 parameters = pars_nc_isig_p1)

fit_nc_sig <- fit_c_sig <- fit_nc_isig <- fit_c_isig <- 
  fit_nc_sig_p1 <- fit_nc_isig_p1 <- list()

for(i in 1:10){
  set.seed(1234 + i)
  fit_nc_sig[[i]] <- tmbstan::tmbstan(obj_nc_sig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
    lower = lower_nc_sig, upper = upper_nc_sig,
        control = list(adapt_delta = 0.99))

  fit_c_sig[[i]] <- tmbstan::tmbstan(obj_c_sig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
    lower = lower_c_sig, upper = upper_c_sig,
        control = list(adapt_delta = 0.99))

  fit_nc_isig[[i]] <- tmbstan::tmbstan(obj_nc_isig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
    lower = lower_nc_isig, upper = upper_nc_isig,
        control = list(adapt_delta = 0.99))

  fit_c_isig[[i]] <- tmbstan::tmbstan(obj_c_isig, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
    lower = lower_c_isig, upper = upper_c_isig,
        control = list(adapt_delta = 0.99))
  
  fit_nc_sig_p1[[i]] <- tmbstan::tmbstan(obj_nc_sig_p1, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
    lower = lower_nc_sig_p1, upper = upper_nc_sig_p1,
        control = list(adapt_delta = 0.99))
  
  fit_nc_isig_p1[[i]] <- tmbstan::tmbstan(obj_nc_isig_p1, init =  "last.par.best",
    warmup = 5000, iter = 10000, chains = 4, 
    lower = lower_nc_isig_p1, upper = upper_nc_isig_p1,
        control = list(adapt_delta = 0.99))
}

for(i in 1:10){
  #print(rstan::traceplot(fit_nc_sig[[i]]))
  print(rstan::traceplot(fit_c_sig[[i]]))
  # print(rstan::traceplot(fit_nc_isig[[i]]))
  # print(rstan::traceplot(fit_c_isig[[i]]))
  # print(rstan::traceplot(fit_nc_sig_p1[[i]]))
  # print(rstan::traceplot(fit_nc_isig_p1[[i]]))
}

mean_par_nc_sig <- mean_par_c_sig <- mean_par_nc_isig <- mean_par_c_isig <-
  mean_par_nc_sig_p1 <- mean_par_nc_isig_p1 <- matrix(0, 10, 4) 
median_par_nc_sig <- median_par_c_sig <- median_par_nc_isig <- median_par_c_isig <-
  median_par_nc_sig_p1 <- median_par_nc_isig_p1 <- matrix(0, 10, 4) 
report_nc_sig <- report_c_sig <- report_nc_isig <- report_c_isig <-
  report_nc_sig_p1 <- report_nc_isig_p1 <- list()

for(i in 1:10){
  postmle <- as.matrix(fit_nc_sig[[i]])[, -ncol(as.matrix(fit_nc_sig[[i]]))]
  mean_par_nc_sig[i, ] <- apply(postmle[,1:4], 2, mean)
  median_par_nc_sig[i, ] <- apply(postmle[,1:4], 2, median)
  report_nc_sig[[i]] <- obj_nc_sig$report(apply(postmle, 2, median))

  postmle <- as.matrix(fit_c_sig[[i]])[, -ncol(as.matrix(fit_c_sig[[i]]))]
  mean_par_c_sig[i, ] <- apply(postmle[,1:4], 2, mean)
  median_par_c_sig[i, ] <- apply(postmle[,1:4], 2, median)
  report_c_sig[[i]] <- obj_c_sig$report(apply(postmle, 2, median))

  postmle <- as.matrix(fit_nc_isig[[i]])[, -ncol(as.matrix(fit_nc_isig[[i]]))]
  mean_par_nc_isig[i, ] <- apply(postmle[,1:4], 2, mean)
  median_par_nc_isig[i, ] <- apply(postmle[,1:4], 2, median)
  report_nc_isig[[i]] <- obj_nc_isig$report(apply(postmle, 2, median))

  postmle <- as.matrix(fit_c_isig[[i]])[, -ncol(as.matrix(fit_c_isig[[i]]))]
  mean_par_c_isig[i, ] <- apply(postmle[,1:4], 2, mean)
  median_par_c_isig[i, ] <- apply(postmle[,1:4], 2, median)
  report_c_isig[[i]] <- obj_c_isig$report(apply(postmle, 2, median))

  postmle <- as.matrix(fit_nc_sig_p1[[i]])[, -ncol(as.matrix(fit_nc_sig_p1[[i]]))]
  mean_par_nc_sig_p1[i, ] <- apply(postmle[,1:4], 2, mean)
  median_par_nc_sig_p1[i, ] <- apply(postmle[,1:4], 2, median)
  report_nc_sig_p1[[i]] <- obj_nc_sig_p1$report(apply(postmle, 2, median))

  postmle <- as.matrix(fit_nc_isig_p1[[i]])[, -ncol(as.matrix(fit_nc_isig_p1[[i]]))]
  mean_par_nc_isig_p1[i, ] <- apply(postmle[,1:4], 2, mean)
  median_par_nc_isig_p1[i, ] <- apply(postmle[,1:4],    2, median)
  report_nc_isig_p1[[i]] <- obj_nc_isig_p1$report(apply(postmle, 2, median))

}

apply(mean_par_nc_sig, 2, mean)
apply(mean_par_c_sig, 2, mean)
apply(mean_par_nc_isig, 2, mean)
apply(mean_par_c_isig, 2, mean)
apply(mean_par_nc_sig_p1, 2, mean)
apply(mean_par_nc_isig_p1, 2, mean)

apply(median_par_nc_sig, 2, mean)
apply(median_par_c_sig, 2, mean)
apply(median_par_nc_isig, 2, mean)
apply(median_par_c_isig, 2, mean)
apply(median_par_nc_sig_p1, 2, mean)
apply(median_par_nc_isig_p1, 2, mean)

for(i in 1:10){
  print(report_nc_sig[[i]]$FMSY)
  print(report_c_sig[[i]]$FMSY)
  print(report_nc_isig[[i]]$FMSY)
  print(report_c_isig[[i]]$FMSY)
  print(report_nc_sig_p1[[i]]$FMSY)
  print(report_nc_isig_p1[[i]]$FMSY)
}

fit_rtmb_stan <- tmbstan::tmbstan(obj, init =  "last.par.best",
  warmup = 5000, iter = 30000, chains = 4, 
        control = list(adapt_delta = 0.99))
rtmb_postmle <- as.matrix(fit_rtmb_stan)[, -ncol(as.matrix(fit_rtmb_stan))]
out_par_rtmb <- apply(rtmb_postmle, 2, median)
report_rtmb <- obj$report(out_par_rtmb)
report_rtmb$sigma2
posterior_rtmb <- as.array(fit_rtmb_stan)
np_fit = bayesplot::nuts_params(fit_rtmb_stan)
bayesplot::mcmc_parcoord(posterior_rtmb, pars = "re[1]", np = np_fit)
bayesplot::mcmc_trace(posterior_rtmb, pars = colnames(rtmb_postmle)[1], np = np_fit)
rstan::traceplot(fit_rtmb_stan)

save(fit_nc_sig, fit_c_sig, fit_nc_isig, fit_c_isig, fit_nc_sig_p1, fit_nc_isig_p1,
     file = "tests/testthat/stan_fits.RData")

################################################################################
# specify stan model
library(rstan)
source("tests/testthat/stan_model.R")


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
  c(25.3, 21.91))
colnames(tuna.dat) <- c('C', 'I')
tuna.dat <- as.data.frame(tuna.dat)


inits <- list(depletion=c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56,0.56),
r=0.8, K=200, sigma2 = 1/100, tau2 = 1/100)

stan_data <- list(
  T = nrow(tuna.dat),
  C = tuna.dat$C,
  I = tuna.dat$I,
  m = 2
)

fit <- rstan::stan(model_code = stan_code, data = stan_data,
            warmup = 5000, iter = 30000, chains = 4, 
        control = list(adapt_delta = 0.99))
print(fit)
postmle <- as.matrix(fit)[, -ncol(as.matrix(fit))]
out_par <- apply(postmle, 2, median)
posterior <- as.array(fit)
np_fit = bayesplot::nuts_params(fit)
bayesplot::mcmc_parcoord(posterior, pars = "re[1]", np = np_fit)
bayesplot::mcmc_trace(posterior, pars = colnames(postmle)[1], np = np_fit)
rstan::traceplot(fit)


mod <- rstan::stan_model(model_code = stan_code)
fit_opt <- rstan::optimizing( mod, data = stan_data,
            init = inits, hessian=TRUE)
fit_opt_par <- c(fit_opt$par[1:27], fit_opt$par["P_final"])

new_par <- c(log_r = log(fit_opt_par["r"]),
                log_K = log(fit_opt_par["K"]),
                log_isigma2 = log(fit_opt_par["sigma2"]^-1),
                log_itau2 = log(fit_opt_par["tau2"]^-1),
                log_P = log(fit_opt_par[5:28])
)
report <- obj$report(new_par)
fit_opt$par["ll_r"]

mod <- rstan::stan_model(model_code = stan_code)

# 2. Prepare your data and initial values
stan_data <- list(
  T = nrow(tuna.dat),
  C = tuna.dat$C,
  I = tuna.dat$I,
  m = 2
)
init_list <- list(
  r = 0.8,
  K = 200,
  sigma2 = 0.01,
  tau2 = 0.01,
  P = c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56)
)

# 3. Run a short sampling to get a stanfit object (or use optimizing)
fit <- rstan::sampling(mod, data = stan_data, chains = 1, iter = 1, init = list(init_list), algorithm = "Fixed_param")

# 4. Extract generated quantities (log-likelihood components)
ll_components <- rstan::extract(fit, pars = c("ll_r", "ll_K", "ll_sigma2", "ll_tau2", "ll_P", "ll_Z"))

# 5. Print results
print(ll_components)


##################################################################################
# Run with natural parameters and centered sigma case

# --- Helper Functions (from functions block) ---

# Pella-Tomlinson step
pella_tomlinson <- function(P0, r, K, m, C) {
  m1 <- m - 1
  surplus_prod <- r / m1 * P0 * (1 - P0 ^ m1)
  P1 <- P0 + surplus_prod - C / K
  return(P1)
}

pt_pmsy <- function(m) {
  m ^ (-1 / (m - 1))
}
pt_bmsy <- function(K, m) {
  PMSY <- m ^ (-1 / (m - 1))
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
  if(logscale) {
    return(ret)
  } else {
    return(exp(ret))
  }
}


library(RTMB)

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
  c(25.3, 21.91))
colnames(tuna.dat) <- c('C', 'I')
tuna.dat <- as.data.frame(tuna.dat)



inits <- list(depletion=c(0.99,0.98,0.96,0.94,0.92,0.90,0.88,0.86,0.84,0.82,
0.80,0.78,0.76,0.74,0.72,0.70,0.68,0.66,0.64,0.62,0.60,0.58,0.56, 0.56),
r=0.8, K=200, sigma2 = 1/100, tau2 = 1/100)


sp_data <- list(T = nrow(tuna.dat),
                C = tuna.dat$C,
                I = tuna.dat$I)  # Example prior on m

pars_inits <- list(r=inits$r, K=inits$K, 
                sigma2=inits$sigma2, 
                tau2=inits$tau2,
                 P=inits$depletion)
lower_inits <- rep(0, length(unlist(pars_inits)))
upper_inits <- rep(Inf, length(unlist(pars_inits)))


obj_inits <- MakeADFun(model_c_sig,
                 parameters = pars_inits)

fit <- tmbstan::tmbstan(obj_inits, init =  "last.par.best",
  warmup = 5000, iter = 10000, chains = 4, 
  lower = lower_inits, upper = upper_inits,
      control = list(adapt_delta = 0.99))

postmle <- as.matrix(fit)[, -ncol(as.matrix(fit))]
out_par <- apply(postmle, 2, median)


model_c_sig <- function(par){
    #### Parameters and reparameterizations ####
    r <- par$r
    K <- par$K
    m <- 2.0
    #sigma2 <- 3.1e-3 
    sigma2 <- par$sigma2
    #tau2 <- 1.151663e-02 
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
    nll_r <- -dlnorm(r, meanlog=-1.38, sdlog=sqrt(3.845), log=TRUE)
    nll_K <- -dlnorm(K, meanlog=5.042905, sdlog=sqrt(3.7603664), log=TRUE)
   # nll_sig2 <- -dinvgamma(sigma2, 3.785518, 0.010223, logscale = TRUE)
   # nll_tau2 <- -dinvgamma(tau2, 1.708603, 0.008613854, logscale = TRUE) 

    # Process likelihood
    nll_P <- -sum(dlnorm(P[1:Tlen], meanlog=log(P_med[1:Tlen]), sd=sigma, log=TRUE))
    # Observation likelihood
    nll_Z <- -sum(RTMB::dnorm(Z, mean=log_q_hat, sd=tau, log=TRUE))
    
    jnll <- nll_r + nll_K + nll_sig2 + nll_tau2 + nll_P + nll_Z

  for (i in 1:(Tlen)) {
    Biomass[i] = K * P[i]
  }


    # Management values
    PMSY <- pt_pmsy(m)
    BMSY <- K * PMSY
    FMSY <- pt_fmsy(r, m)
    MSY  <- pt_msy(BMSY, FMSY)

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




# test gamma:
# TMB:
dgamma_TMB <- function(x, shape, scale, logscale = TRUE) {
  -lgamma(shape)+(shape-1.0)*log(x)-x/scale-shape*log(scale)
}
dgamma_TMB(5, 4, 3, TRUE)
dgamma(5, shape=4, scale=3, log=TRUE)

dinvgamma_FIMS <- function(x, shape, scale, log = TRUE){
   shape * log(scale) - lgamma(shape) -
             (shape + 1.0) * log(x) - scale / x
}

dgamma(1 / 5, shape=4, scale=3, log=TRUE) - 2 * log(5)

dinvgamma_FIMS(5, shape=4, scale=1/3, log = TRUE)
dinvgamma(5, shape=4, scale=3, log = TRUE)
dinvgamma_FIMS(1, shape = 2, scale = 1, log = TRUE)
dinvgamma(1, shape = 2, scale = 1, log = TRUE)
dinvgamma_FIMS(2, shape = 3, scale = 5, log = TRUE)
dinvgamma(2, shape = 3, scale = 5, log = TRUE)
dinvgamma_FIMS(.5, shape = 1, scale = 2, log = TRUE)
dinvgamma(.5, shape = 1, scale = 2, log = TRUE)

