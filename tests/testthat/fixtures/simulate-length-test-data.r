working_dir <- getwd()

maindir <- tempdir()

AtoL <- function(a_f,Linf_f,K_f,a_0){
  L <- Linf_f*(1-exp(-K_f*(a_f-a_0)))
}

AtoLSchnute <- function(a,L1,L2,a1,a2,Ks){
  L <- L1+(L2-L1)*((1-exp(-K*(a-a1)))/(1-exp(-K*(a2-a1))))
}

amax <- 12
Linf <- 80
K <- 0.18
a0 <- -1.36
amin <- 1
cv <- 0.005
ages <- 1:amax
len_bins <- 22:75

L1 <- 27.69
L2 <- 72.78
a1 <- 1
a2 <- 12
Ks <- 0.18

mean_length_age <- AtoL(1:amax,Linf,K,a0)

om_input_list <- list("amax" = amax, "Linf" = Linf, "K" = K, "a0" = a0, "amin" = amin, "cv" = cv, "ages" = ages, "len_bins" = len_bins) 

length_age_conversion <- matrix(NA,nrow=length(ages),ncol=length(len_bins))
for(i in seq_along(ages)){
  #Calculate mean length at age to spread lengths around
  mean_length <- AtoL(ages[i],Linf,K,a0)
  #mean_length <- AtoLSchnute(ages[i],L1,L2,a1,a2,Ks)
  #Calculate the cumulative proportion shorter than each composition length
  temp_len_probs<-pnorm(q=len_bins,mean=mean_length,sd=mean_length*cv)
  #Reset the first length proportion to zero so the first bin includes all
  #density smaller than that bin
  temp_len_probs[1]<-0
  #subtract the offset length probabilities to calculate the proportion in each
  #bin. For each length bin the proportion is how many fish are larger than this
  #length but shorter than the next bin length.
  temp_len_probs <- c(temp_len_probs[-1],1)-temp_len_probs
  length_age_conversion[i,] <- temp_len_probs
}
colnames(length_age_conversion) <- len_bins
rownames(length_age_conversion) <- ages


#add code to simulate length data
length_data <- 0L

on.exit(unlink(maindir, recursive = TRUE), add = TRUE)

setwd(working_dir)
on.exit(setwd(working_dir), add = TRUE)

save(om_input_list, length_age_conversion, length_data,
  file = test_path("fixtures", "length_test_data.RData")
)
