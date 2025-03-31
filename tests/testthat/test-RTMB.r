library(FIMS)
library(RTMB)
logistic <- methods::new(LogisticSelectivity)
foo = logistic$distr_dpois( advector(1), advector(1), TRUE)

foo = logistic$evaluate_RTMB(x = advector(1),  input_slope = advector(0.2), input_inflection_point = advector(10))
foo
#logistic$evaluate_RTMB(x = 1,  input_slope = 0.2, input_inflection_point = 10)

library(FIMS)
library(RTMB)

logistic <- methods::new(LogisticSelectivity)

# Test with minimal inputs
x <- advector(1)
input_slope <- advector(0.2)
input_inflection_point <- advector(10)

# Call the method
foo <- logistic$evaluate_RTMB(x = x, input_slope = input_slope, input_inflection_point = input_inflection_point)
print(foo)
