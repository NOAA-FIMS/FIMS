test_that("reference points reproduce operating-model equilibrium benchmarks", {
  fixture <- new.env()
  load(test_path("fixtures", "integration_test_data.RData"), envir = fixture)
  for (i in seq_along(fixture$om_output_list)) {
    om <- fixture$om_input_list[[i]]
    benchmark <- fixture$om_output_list[[i]]$msy
    inputs <- list(
      natural_mortality = om$M.age, weight = om$W.mt,
      maturity = om$mat.age, proportion_female = om$proportion.female,
      fleets = list(list(share = 1, selectivity = om$selex_fleet[[1]], weight = om$W.mt)),
      recruitment = list(type = "beverton_holt", rzero = om$R0,
        steepness = om$h, phi0 = om$Phi.0)
    )
    result <- calculate_reference_points(inputs, max_f = benchmark$maxF)$msy
    expect_identical(result$status, "converged")
    expect_lte(abs(result$fishing_mortality - benchmark$Fmsy), benchmark$step)
    expect_equal(result$total_yield, benchmark$msy, tolerance = 1e-5)
    # FIMS refines continuously; OM reports quantities at the nearest grid F.
    expect_equal(result$spawning_biomass, benchmark$SSBmsy, tolerance = 0.002)
    expect_equal(result$total_biomass, benchmark$Bmsy, tolerance = 0.002)
    expect_equal(result$recruitment, benchmark$Rmsy, tolerance = 0.002)
    expect_equal(result$achieved_spr, benchmark$SPRmsy, tolerance = 0.002)
    # Also verify the full stored curves with SPR roots at selected OM grid Fs.
    indices <- c(101L, 501L, 1001L)
    targets <- benchmark$spr[indices] / om$Phi.0
    grid <- calculate_reference_points(inputs, spr_targets = targets, msy = FALSE)$spr
    expect_equal(grid$fishing_mortality, benchmark$f_seq[indices], tolerance = 1e-6)
    expect_equal(grid$spawning_biomass, benchmark$SSB_eq[indices], tolerance = 1e-6)
    expect_equal(grid$recruitment, benchmark$R_eq[indices], tolerance = 1e-6)
    expect_equal(grid$total_yield, benchmark$L_eq[indices], tolerance = 1e-6)
  }
})
