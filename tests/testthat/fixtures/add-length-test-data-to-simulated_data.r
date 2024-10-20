#Load FIMS and get the milestone 1 data
#load("C:/Users/Nathan/Documents/GitHub/FIMS/tests/testthat/fixtures/integration_test_data.RData")

#Use life history parameters to simulation length composition data
Linf <- om_input_list[[1]]$Linf
K <- om_input_list[[1]]$K
a0 <- om_input_list[[1]]$a0
cv <- 0.1
len_bins <- seq(0,1100,50)
ages <- om_input_list[[1]]$ages
amax <- max(om_input_list[[1]]$ages)
years <- om_output_list[[1]]$year

AtoL <- function(a,Linf,K,a_0){
  L <- Linf*(1-exp(-K*(a-a_0)))
}

#Create length at age conversion matrix and fill proportions using above
#growth parameters
length_age_conversion <- matrix(NA,nrow=length(ages),ncol=length(len_bins))
for(i in seq_along(ages)){
  #Calculate mean length at age to spread lengths around
  mean_length <- AtoL(ages[i],Linf,K,a0)
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

for(i in seq_along(om_output_list)){
  om_output_list[[i]]$N.length <- om_output_list[[i]]$N.age[,rep(1,length(len_bins))]
  om_output_list[[i]]$L.length <- om_output_list[[i]]$L.age
  for(j in length(om_output_list[[i]]$L.length)){
    om_output_list[[i]]$L.length[[j]] <-  om_output_list[[i]]$L.length[[j]][,rep(1,length(len_bins))]
  }
  om_output_list[[i]]$survey_length_comp <- om_output_list[[i]]$survey_age_comp
  for(j in length(om_output_list[[i]]$survey_length_comp)){
    om_output_list[[i]]$survey_length_comp[[j]] <-  om_output_list[[i]]$survey_length_comp[[j]][,rep(1,length(len_bins))]
  }

  for(j in seq_along(years)){
    for(k in seq_along(len_bins)){
      om_output_list[[i]]$N.length[j,k] <- sum(length_age_conversion[,k]*om_output_list[[i]]$N.age[j,])
      for(l in seq_along(om_output_list[[i]]$L.length)){
        om_output_list[[i]]$L.length[[l]][j,k] <- sum(length_age_conversion[,k]*om_output_list[[i]]$L.age[[l]][j,])
      }
      for(l in seq_along(om_output_list[[i]]$survey_length_comp)){
        om_output_list[[i]]$survey_length_comp[[l]][j,k] <- sum(length_age_conversion[,k]*om_output_list[[i]]$survey_age_comp[[l]][j,])
      }
    }
  }
}

for(i in seq_along(em_input_list)){
  em_input_list[[i]]$L.length.obs <- em_input_list[[i]]$L.age.obs
  for(j in length(em_input_list[[i]]$L.length.obs)){
    em_input_list[[i]]$L.length.obs[[j]] <-  em_input_list[[i]]$L.length.obs[[j]][,rep(1,length(len_bins))]
  }
  em_input_list[[i]]$survey.length.obs <- em_input_list[[i]]$survey.age.obs
  for(j in length(om_output_list[[i]]$survey_length_comp)){
    em_input_list[[i]]$survey.length.obs[[j]] <-  em_input_list[[i]]$survey.length.obs[[j]][,rep(1,length(len_bins))]
  }

  for(j in seq_along(years)){
    for(k in seq_along(len_bins)){
      for(l in seq_along(em_input_list[[i]]$L.length.obs)){
        em_input_list[[i]]$L.length.obs[[l]][j,k] <- sum(length_age_conversion[,k]*em_input_list[[i]]$L.age.obs[[l]][j,])
      }
      for(l in seq_along(em_input_list[[i]]$survey.length.obs)){
        em_input_list[[i]]$survey.length.obs[[l]][j,k] <- sum(length_age_conversion[,k]*em_input_list[[i]]$survey.age.obs[[l]][j,])
      }
    }
  }
}

for(i in seq_along(om_input_list)){
  om_input_list[[i]]$nlengths <- length(len_bins)
  om_input_list[[i]]$lengths <- len_bins
}

rm(length_age_conversion)
rm(a0)
rm(ages)
rm(amax)
rm(cv)
rm(i)
rm(j)
rm(k)
rm(l)
rm(len_bins)
rm(Linf)
rm(mean_length)
rm(temp_len_probs)
rm(years)
rm(AtoL)
rm(K)
