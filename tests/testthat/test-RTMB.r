library(FIMS)
library(RTMB)
logistic <- methods::new(LogisticSelectivity)
foo = logistic$distr_dpois( 1, 1, TRUE)

foo = logistic$evaluate_RTMB(x = advector(1),  input_slope = advector(0.2), input_inflection_point = advector(10))
foo
#logistic$evaluate_RTMB(x = 1,  input_slope = 0.2, input_inflection_point = 10)
