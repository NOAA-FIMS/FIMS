#Load FIMS and get the milestone 1 data
library(FIMS)

data(package = "FIMS")

#Create a milestone 2 dataframe matching milestone 1
data_mile2 <- data_mile1

#Add new column for length values and set to NA for all milestone 1 data
data_mile2 <- data_mile2[,c(1:3,3:8)]
colnames(data_mile2)[4] <- "len"
data_mile2[,4] <- NA

#Growth function values to create length age conversion matrix from model
#comparison project

Linf <- 800
K <- 0.18
a0 <- -1.36
amax <- 12
cv <- 0.1

L2Wa <- 2.5e-08
L2Wb <- 3


AtoL <- function(a,Linf,K,a_0){
  L <- Linf*(1-exp(-K*(a-a_0)))
}

ages <- 1:amax
len_bins <- seq(0,1100,50)

#Create length at age conversion matrix and fill proportions using above 
#growth parameters
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

#Extract years and fleets from milestone 1 data
start_date <- unique(data_mile2$datestart[data_mile2$type=="weight-at-age"])
end_date <- unique(data_mile2$dateend[data_mile2$type=="weight-at-age"])
observers <- unique(data_mile2$name[data_mile2$type=="age"])

#Create data frame for new fleet and year specific length at age conversion proportions
#These are identical across years and fleets in this default example
length_age_data <- data_mile2[rep(1,length(ages)*
                                    length(len_bins)*
                                    length(start_date)*
                                    length(observers)),]

row_index <- 1
for(k in seq_along(start_date)){
  for(l in seq_along(observers)){
    for(i in seq_along(ages)){
      for(j in seq_along(len_bins)){
        length_age_data[row_index,] <- c("length-at-age",
                                         observers[l],
                                         ages[i],
                                         len_bins[j],
                                         start_date[k],
                                         end_date[k],
                                         length_age_conversion[i,j],
                                         "proportion",
                                         200)
        row_index <- row_index + 1
      }
    }
  }
}

#Create a length compostion data frame that will be filled by transforming 
#the milestone 1 age composition data
length_comp_data <- data_mile2[rep(1,length(len_bins)*
                                    length(start_date)*
                                    length(observers)),]

age_comp_data <- data_mile2[data_mile2$type=="age",]

row_index<-1
for(k in seq_along(start_date)){
  year_sub <- age_comp_data[age_comp_data$datestart==start_date[k],]
  for(l in seq_along(observers)){
    obs_sub <- year_sub[year_sub$name==observers[l],]
    for(j in seq_along(len_bins)){
      temp_len_props <- length_age_conversion[,j]
      temp_len_prob <- sum(temp_len_props*obs_sub$value)
      length_comp_data[row_index,] <- c("length-comp",
                                       observers[l],
                                       NA,
                                       len_bins[j],
                                       start_date[k],
                                       end_date[k],
                                       temp_len_prob,
                                       "proportion",
                                       200)
      row_index <- row_index + 1
    }
  }
}

#Add the conversion matrix and length composition data to milestone 2 dataframe
data_mile2 <- rbind(data_mile2,length_comp_data,length_age_data)

#save(data_mile2,file="C:/Users/Nathan/Documents/GitHub/FIMS/data/data_mile2.rda")
