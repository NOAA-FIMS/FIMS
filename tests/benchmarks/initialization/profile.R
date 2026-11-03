# Usage: profile.R YEARS OUTPUT_DIRECTORY [true|false for profiling]
args <- commandArgs(TRUE)
years <- as.integer(args[1])
output <- args[2]
do_profile <- length(args) < 3L || args[3] == "true"
dir.create(output,recursive=TRUE,showWarnings=FALSE)
library(FIMS)
options(rlib_message_verbosity="quiet")
data("data_big",package="FIMS")
fixed <- data_big[is.na(data_big$timing), ]
observed <- data_big[!is.na(data_big$timing) & data_big$type != "weight_at_age", ]
weight <- data_big[data_big$type == "weight_at_age", ]
observations <- lapply(seq_len(years),function(y) {
  x<-observed[observed$timing == (y-1L) %% 30L+1L, ]; x$timing<-y; x
})
weights <- lapply(seq_len(years+1L),function(y) {
  source <- if(y<=31L) y else (y-1L) %% 30L+1L
  x<-weight[weight$timing == source, ]; x$timing<-y; x
})
raw <- do.call(rbind,c(list(fixed),observations,weights))
rm(fixed,observed,weight,observations,weights)
metrics <- list()
profile_phase <- function(name, fun) {
  invisible(gc())
  prefix <- file.path(output,paste0(years,"-",name))
  if (do_profile) {
    Rprof(paste0(prefix,".Rprof"),interval=0.005)
    Rprofmem(paste0(prefix,".mem"))
  }
  start <- proc.time()
  result <- tryCatch(fun(),finally={Rprof(NULL);Rprofmem(NULL)})
  elapsed <- proc.time()-start
  bytes <- numeric(0)
  if (do_profile) {
    cpu <- summaryRprof(paste0(prefix,".Rprof"))
    write.csv(cpu$by.total,file=paste0(prefix,"-cpu.csv"))
    lines <- readLines(paste0(prefix,".mem"))
    lines <- lines[grepl("^[0-9]+",lines)]
    bytes <- as.numeric(sub(" .*","",lines))
    stacks <- sub("^[0-9]+ :? *","",lines)
    allocation <- aggregate(bytes,list(stack=stacks),sum)
    names(allocation)[2]<-"bytes"
    allocation <- allocation[order(-allocation$bytes), ]
    write.csv(head(allocation,30),paste0(prefix,"-allocations.csv"),row.names=FALSE)
  }
  metrics[[name]] <<- data.frame(years,phase=name,elapsed=elapsed[["elapsed"]],
                                 allocated_mib=if (do_profile) sum(bytes)/1024^2 else NA_real_,
                                 allocations=if (do_profile) length(bytes) else NA_integer_)
  print(metrics[[name]])
  if (do_profile) print(head(cpu$by.total,8))
  result
}
frame <- profile_phase("frame",function() FIMSFrame(raw))
parameters <- profile_phase("parameters",function() setup_default_parameters(data=frame))
input <- profile_phase("modules",function() initialize_fims(parameters,data=frame))
write.csv(do.call(rbind,metrics),file.path(output,paste0(years,"-summary.csv")),row.names=FALSE)
writeLines(trimws(capture.output(sessionInfo()),which="right"),file.path(output,"session.txt"))
# Numerical checks are outside the measured initialization phases.
obj <- TMB::MakeADFun(data=list(),parameters=input$parameters,map=input$map,
                      random=NULL,DLL="FIMS",silent=TRUE)
saveRDS(list(value=obj$fn(obj$par),gradient=obj$gr(obj$par),report=obj$report(obj$par)),
        file.path(output,paste0(years,"-numerics.rds")))
clear()
