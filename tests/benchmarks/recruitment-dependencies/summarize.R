args <- commandArgs(TRUE)
input <- normalizePath(args[1])
output <- args[2]
dir.create(output, recursive = TRUE, showWarnings = FALSE)
library(Matrix)
ablation <- dplyr::bind_rows(lapply(list.files(input, "^ablation-.*[.]csv$",
                                             full.names = TRUE), read.csv))
lifted <- dplyr::bind_rows(lapply(list.files(input, "^lifted-.*[.]csv$",
                                           full.names = TRUE), read.csv))
ablation <- ablation[order(ablation$years, ablation$centered,
                          -ablation$plus_group, -ablation$stock_recruit_feedback), ]
lifted <- lifted[order(lifted$years), ]
stopifnot(nrow(ablation) == 32L, nrow(lifted) == 4L,
          all(ablation$convergence == 0L),
          all(ablation$hessian_positive_definite))
validation <- do.call(rbind, lapply(lifted$years, function(year) {
  errors <- readRDS(file.path(input, sprintf("lifted-%d.rds", year)))$errors
  stopifnot(all(errors[!is.na(errors)] < 1e-5))
  data.frame(years = year, check = names(errors), error = unname(errors))
}))
write.csv(validation, file.path(output, "validation.csv"), row.names = FALSE)
equivalence_path <- file.path(dirname(input), "equivalence.rds")
if (file.exists(equivalence_path)) {
  equivalence <- readRDS(equivalence_path)
  errors <- c(unlist(equivalence$point_checks), equivalence$mode_checks)
  stopifnot(all(errors < 1e-5))
  write.csv(data.frame(check = names(errors), error = unname(errors)),
            file.path(output, "equivalence.csv"), row.names = FALSE)
}
lifted$component_speed_ratio <-
  (lifted$reduced_hessian_cpu_s + lifted$reduced_factor_cpu_s) /
  (lifted$lifted_hessian_cpu_s + lifted$lifted_factor_cpu_s)
write.csv(ablation, file.path(output, "ablation.csv"), row.names = FALSE)
write.csv(lifted, file.path(output, "lifted.csv"), row.names = FALSE)
writeLines(capture.output(sessionInfo()), file.path(output, "session.txt"))

plot_pattern <- function(matrix, title) {
  entries <- summary(matrix)
  n <- nrow(matrix)
  pattern <- matrix(0, n, n)
  pattern[cbind(entries$i, entries$j)] <- 1
  pattern[cbind(entries$j, entries$i)] <- 1
  image(seq_len(n), seq_len(n), t(pattern[n:1, , drop = FALSE]),
        col = c("white", "#235c83"), zlim = c(0, 1), asp = 1, useRaster = TRUE,
        xlab = "Column", ylab = "Row (reversed)", main = title,
        xlim = c(0, n + 1), ylim = c(0, n + 1))
}
example <- readRDS(file.path(input, "lifted-120.rds"))
n <- example$results$recruitment_effects
permutation <- as.vector(rbind(seq_len(n), n + seq_len(n), 2L * n + seq_len(n)))
draw_exact <- function() {
  par(mfrow = c(1, 3), mar = c(4, 4, 3, 1))
  plot_pattern(example$reduced_hessian, "Recruitment Hessian (119 x 119)")
  plot_pattern(example$lifted_hessian, "Lagrangian Hessian (238 x 238)")
  plot_pattern(example$kkt[permutation, permutation], "Time-ordered KKT (357 x 357)")
}
svglite::svglite(file.path(output, "exact-lifting-sparsity.svg"), width = 12, height = 4)
draw_exact()
dev.off()
ragg::agg_png(file.path(output, "exact-lifting-sparsity.png"), width = 1440, height = 480,
              res = 120)
draw_exact()
dev.off()

svglite::svglite(file.path(output, "ablation-sparsity.svg"), width = 12, height = 6)
par(mfrow = c(2, 4), mar = c(3, 3, 3, 1))
for (centered in 0:1) for (plus in 1:0) for (feedback in 1:0) {
  case <- readRDS(file.path(input, sprintf("ablation-120-c%d-p%d-f%d.rds",
                                          centered, plus, feedback)))
  plot_pattern(case$hessian, sprintf("%s; plus=%d, SR=%d",
                                     if (centered) "log R" else "deviations",
                                     plus, feedback))
}
dev.off()

svglite::svglite(file.path(output, "component-scaling.svg"), width = 8, height = 5)
dense <- lifted$reduced_hessian_cpu_s + lifted$reduced_factor_cpu_s
sparse <- lifted$lifted_hessian_cpu_s + lifted$lifted_factor_cpu_s
plot(lifted$years, dense * 1000, type = "b", log = "xy", pch = 16,
     col = "#ae442a", xlab = "Years (12 ages)", ylab = "CPU milliseconds",
     ylim = range(c(dense, sparse) * 1000),
     main = "Hessian evaluation + fresh factorization only")
lines(lifted$years, sparse * 1000, type = "b", pch = 17, col = "#235c83")
legend("topleft", c("Dense recruitment Hessian", "Sparse deterministic lifting"),
       col = c("#ae442a", "#235c83"), pch = c(16, 17), lty = 1, bty = "n")
mtext("Not an end-to-end fitting benchmark; adjoint/setup costs excluded", side = 1,
      line = 3, cex = 0.8)
dev.off()
print(ablation[ablation$years == 120,
               c("centered", "plus_group", "stock_recruit_feedback",
                 "random_effects", "structural_entries", "density", "bandwidth")])
print(lifted)
