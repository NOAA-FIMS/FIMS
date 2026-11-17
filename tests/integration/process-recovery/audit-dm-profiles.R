# Repository root: baseline RDS, completed profile output directory.
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 2L)
baseline <- readRDS(args[[1L]])
directory <- args[[2L]]
x <- readRDS(file.path(directory, "result.rds"))
r <- x$tables$refits; p <- x$tables$profiles; points <- x$tables$points
stopifnot(nrow(r) == 42L, nrow(p) == 70L, nrow(x$tables$reports) == 7L,
  nrow(x$tables$metrics) == 21L, all(table(interaction(r$replicate,r$case,drop=TRUE))==6L),
  all(tapply(r$selected,interaction(r$replicate,r$case,drop=TRUE),sum)==1L))
for (field in c("source_md5","production_md5")) {
 h<-x$configuration[[field]];stopifnot(identical(unname(h),unname(tools::md5sum(names(h)))))
}
stopifnot(identical(unname(x$configuration$baseline_md5), unname(tools::md5sum(args[[1L]]))))
valid <- p$optimizer_valid
stopifnot(all(is.finite(p$delta_nll[valid])),all(p$delta_nll[valid]>=-1e-9),
 all(is.na(p$delta_nll[!valid])))
M <- subset(points,stage=="profile" & parameter=="M")
stopifnot(nrow(M)==70L,all(abs(M$value-log(M$fixed_M_sd))<1e-12))
for(rep in 1:5) {
 f<-subset(r,replicate==rep & case=="estimated_dm" & optimizer_valid)
 g<-subset(p,replicate==rep & optimizer_valid)
 reference<-min(c(f$objective,g$objective))
 stopifnot(all(abs(g$reference_objective-reference)<1e-9))
}
cat("Counts, selection, map values, local references and hashes: PASS\n")
print(table(r$control,r$optimizer_valid));print(table(p$optimizer_valid))
print(subset(r,selected)[,c("replicate","case","control","start","objective","M_sd","optimizer_valid")])
print(x$tables$reports[,c("replicate","case","optimizer_valid","status","eligible","regular")])
cat("Profile gaps:\n");print(unique(p[,c("replicate","free_optimization_gap")]))
print(subset(p,profile_sd==.05)[,c("replicate","start","optimizer_valid","delta_nll")])
# Compact comparison preserves the previous pilot's acceptance and denominators.
selected <- subset(r,selected)
old <- subset(baseline$tables$reports,warm & scenario=="strong")
comparison <- merge(selected,old[,c("replicate","case","optimizer_valid","eligible")],
 by=c("replicate","case"),suffixes=c("_new","_old"))
old_nll <- vapply(seq_len(nrow(comparison)),function(i)
 baseline$snapshots[[paste(comparison$replicate[i],"strong",comparison$case[i],sep="/")]]$objective,numeric(1))
comparison$objective_change <- comparison$objective-old_nll
write.csv(comparison,file.path(directory, "refit-comparison.csv"),row.names=FALSE)
print(comparison[,c("replicate","case","optimizer_valid_old","optimizer_valid_new","objective_change")])
