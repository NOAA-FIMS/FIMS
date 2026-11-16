# check_sdreport_convergence() returns correct error messages

    Code
      FIMS:::check_sdreport_convergence(obj, opt, sdreport, parameter_names = c(
        "Recruitment.1.log_rzero.1", "Fleet.1.log_q.3"))
    Condition
      Warning:
      x sdreport convergence issues detected:
      i 1 fixed effect has NA standard error: "Recruitment.1.log_rzero.1".
      Warning:
      ! Large condition number detected in Hessian; the matrix may be near singular.
      i Condition number of Hessian ("1e+08") exceeds threshold of 1e+05.
      i This suggests the model is weakly identified and results may be unreliable.
      i Consider simplifying the model, improving data quality, or fixing poorly informed parameters.
      i Among fixed effects, the 1 largest standard error value is:
      i 1. "Fleet.1.log_q.3": "9e+00"
      i Standard errors and MLEs may be unreliable.

