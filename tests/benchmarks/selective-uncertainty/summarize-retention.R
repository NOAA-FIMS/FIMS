args <- commandArgs(TRUE)
input <- args[1]
output <- args[2]
dir.create(output, recursive=TRUE, showWarnings=FALSE)
runs <- jsonlite::fromJSON(file.path(input,"runs.json"))
stopifnot(nrow(runs)==4L,all(runs$exit_code==0))
reference <- readRDS(file.path(input,"1-full.rds"))$check
checks <- lapply(runs$label,function(label) {
  result <- readRDS(file.path(input,paste0(label,".rds")))$check
  passed <- isTRUE(all.equal(result,reference,tolerance=1e-10))
  if (!passed) print(all.equal(result,reference,tolerance=1e-10))
  stopifnot(passed)
  data.frame(label,passed)
})
summary <- aggregate(cbind(peak_rss_mib,wall_seconds) ~ mode, data=runs,FUN=median)
write.csv(runs,file.path(output,"retention-runs.csv"),row.names=FALSE)
write.csv(summary,file.path(output,"retention-summary.csv"),row.names=FALSE)
write.csv(do.call(rbind,checks),file.path(output,"retention-validation.csv"),row.names=FALSE)
print(summary)
