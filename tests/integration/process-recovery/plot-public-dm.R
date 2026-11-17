args <- commandArgs(trailingOnly = TRUE); stopifnot(length(args) == 1L)
directory <- args[[1]]
bins <- read.csv(file.path(directory, "bins.csv"))
differences <- read.csv(file.path(directory, "generator_differences.csv"))
metrics <- read.csv(file.path(directory, "metrics.csv"))
levels <- c(Inf, 2000, 200); colors <- c("#0072B2", "#D55E00")
png(file.path(directory, "public-dm-validation.png"), width = 1600, height = 1200, res = 160)
par(mfrow = c(2, 2), mar = c(5, 4.5, 3, 1))
for (metric in c("mean_z", "variance_z")) {
  z <- bins[[metric]]; top <- max(3, abs(z[is.finite(z)])) * 1.1
  plot(NA, xlim = c(.7, 3.3), ylim = c(-top, top), xaxt = "n", xlab = "Generating concentration",
    ylab = "Residual / Monte Carlo SE", main = if (metric == "mean_z") "Mean counts against analytical expectation" else "Count variance against analytical expectation")
  axis(1, 1:3, c("Infinite", "2000", "200")); abline(h = 0, col = "grey60")
  for (i in 1:2) for (j in 1:3) {
    x <- bins[bins$generator == c("public", "reference")[i] & bins$concentration == levels[j], ]
    points(rep(j + (i - 1.5)*.1,nrow(x)), x[[metric]], pch = 16, cex = .6, col = colors[i])
  }
  if (metric == "mean_z") legend("bottomleft",c("Public API","Independent reference"),col=colors,pch=16,bty="n",cex=.8)
}
z <- unlist(differences[c("mean_z", "variance_z")]); top <- max(3, abs(z[is.finite(z)])) * 1.1
plot(NA,xlim=c(.7,3.3),ylim=c(-top,top),xaxt="n",xlab="Generating concentration",
  ylab="Difference / Monte Carlo SE",main="Public minus independent sampler")
axis(1,1:3,c("Infinite","2000","200"));abline(h=0,col="grey60")
for (i in 1:2) for (j in 1:3) {
 x<-differences[differences$concentration==levels[j],]
 points(rep(j+(i-1.5)*.1,nrow(x)),x[[c("mean_z","variance_z")[i]]],pch=16,cex=.6,col=colors[i])
}
legend("bottomleft",c("Mean","Variance"),col=colors,pch=16,bty="n",cex=.8)
x<-metrics[metrics$process=="M" & metrics$eligible,]
plot(NA,xlim=c(.7,2.3),ylim=c(0,1),xaxt="n",xlab="Generating concentration",
 ylab="Pointwise M-state interval coverage",main="Fresh 18-year public-simulator datasets")
axis(1,1:2,c("2000","200"));abline(h=.95,lty=2,col="grey60")
for(i in 1:2) for(j in 1:2){
 z<-x[x$case==c("known_dm","estimated_dm")[i] & x$scenario==c("moderate","strong")[j],]
 points(rep(j+(i-1.5)*.1,nrow(z)),z$coverage,pch=ifelse(z$regular,16,4),col=colors[i])
}
legend("bottomleft",c("Known concentration","Estimated concentration","Cross: parameter bound"),
 col=c(colors,"black"),pch=c(16,16,4),bty="n",cex=.8)
dev.off()
