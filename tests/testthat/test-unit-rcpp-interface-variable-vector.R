test_that("Parameter vector works as expected", {

v_size <- 10
v1_value <- 1.0
v2_value <- 2.0

#Test that default constructor works
v0 <- new(ParameterVector)
expect_equal(length(v0), 1)
expect_equal(v0$at(1)$value, 0)

# Test that constructor that initializes based on size works.
v1 <- new(ParameterVector, v_size)
v1$fill(v1_value)
for(i in 1:v_size){
   expect_equal(v1$get(i-1)$value, v1_value)
}

# Test that constructor that takes vector and size works.
v2 <- new(ParameterVector, rep(v2_value, v_size), v_size)
for(i in 1:v_size){
   expect_equal(v2$get(i-1)$value, v2_value)
}


#Test plus operator works.
v_plus_test <- v1 + v2
for(i in 1:v_size){
   expect_equal(v_plus_test$get(i-1)$value, (v1[i]$value + v2[i]$value))
}


#Test minus operator works.
v_minus_test <- v1 - v2
for(i in 1:v_size){
   expect_equal(v_minus_test$get(i-1)$value, (v1[i]$value - v2[i]$value))
}


#Test mult operator works.
v_mult_test <- v1 * v2
for(i in 1:v_size){
   expect_equal(v_mult_test$get(i-1)$value, (v1[i]$value * v2[i]$value))
}


#Test div operator works.
v_div_test <- v1 / v2
for(i in 1:v_size){
   expect_equal(v_div_test$get(i-1)$value, (v1[i]$value / v2[i]$value))
}


#Test pre scalar plus operator works.
v_plus_test_scalar <- v2_value + v1
for(i in 1:v_size){
   expect_equal(v_plus_test_scalar$get(i-1)$value, (v2_value+ v1[i]$value ))
}


#Test pre scalar minus operator works.
v_minus_test_scalar <- v2_value - v1
for(i in 1:v_size){
   expect_equal(v_minus_test_scalar$get(i-1)$value, (v2_value- v1[i]$value ))
}


#Test pre scalar mult operator works.
v_mult_test_scalar <- v2_value * v1
for(i in 1:v_size){
   expect_equal(v_mult_test_scalar$get(i-1)$value, (v2_value* v1[i]$value ))
}


#Test pre scalar div operator works.
v_div_test_scalar <- v2_value / v1
for(i in 1:v_size){
   expect_equal(v_div_test_scalar$get(i-1)$value, (v2_value/ v1[i]$value ))
}


#Test post scalar plus operator works.
v_plus_test_scalar <- v1 + v2_value
for(i in 1:v_size){
   expect_equal(v_plus_test_scalar$get(i-1)$value, (v1[i]$value + v2_value))
}


#Test post scalar minus operator works.
v_minus_test_scalar <- v1 - v2_value
for(i in 1:v_size){
   expect_equal(v_minus_test_scalar$get(i-1)$value, (v1[i]$value - v2_value))
}


#Test post scalar mult operator works.
v_mult_test_scalar <- v1 * v2_value
for(i in 1:v_size){
   expect_equal(v_mult_test_scalar$get(i-1)$value, (v1[i]$value * v2_value))
}


#Test post scalar div operator works.
v_div_test_scalar <- v1 / v2_value
for(i in 1:v_size){
   expect_equal(v_div_test_scalar$get(i-1)$value, (v1[i]$value / v2_value))
}


#Test acos function works.
v_acos_test <- acos(v1)
for(i in 1:v_size){
   expect_equal(v_acos_test$get(i-1)$value, acos(v1_value))
}


#Test asin function works.
v_asin_test <- asin(v1)
for(i in 1:v_size){
   expect_equal(v_asin_test$get(i-1)$value, asin(v1_value))
}


#Test atan function works.
v_atan_test <- atan(v1)
for(i in 1:v_size){
   expect_equal(v_atan_test$get(i-1)$value, atan(v1_value))
}


#Test cos function works.
v_cos_test <- cos(v1)
for(i in 1:v_size){
   expect_equal(v_cos_test$get(i-1)$value, cos(v1_value))
}


#Test cosh function works.
v_cosh_test <- cosh(v1)
for(i in 1:v_size){
   expect_equal(v_cosh_test$get(i-1)$value, cosh(v1_value))
}


#Test sin function works.
v_sin_test <- sin(v1)
for(i in 1:v_size){
   expect_equal(v_sin_test$get(i-1)$value, sin(v1_value))
}


#Test sinh function works.
v_sinh_test <- sinh(v1)
for(i in 1:v_size){
   expect_equal(v_sinh_test$get(i-1)$value, sinh(v1_value))
}


#Test tan function works.
v_tan_test <- tan(v1)
for(i in 1:v_size){
   expect_equal(v_tan_test$get(i-1)$value, tan(v1_value))
}


#Test tanh function works.
v_tanh_test <- tanh(v1)
for(i in 1:v_size){
   expect_equal(v_tanh_test$get(i-1)$value, tanh(v1_value))
}


#Test exp function works.
v_exp_test <- exp(v1)
for(i in 1:v_size){
   expect_equal(v_exp_test$get(i-1)$value, exp(v1_value))
}


#Test log10 function works.
v_log10_test <- log10(v1)
for(i in 1:v_size){
   expect_equal(v_log10_test$get(i-1)$value, log10(v1_value))
}


#Test sqrt function works.
v_sqrt_test <- sqrt(v1)
for(i in 1:v_size){
   expect_equal(v_sqrt_test$get(i-1)$value, sqrt(v1_value))
}


#Test log function works.
v_log_test <- log(v1)
for(i in 1:v_size){
   expect_equal(v_log_test$get(i-1)$value, log(v1_value))
}

clear()

})

