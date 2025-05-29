fleet1_nested_tbl <- tibble::tibble(
  selectivity = list(tibble(form = "LogisticSelectivity")),
  data_distribution = list(
    tibble(
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution"
    )
  )
)
