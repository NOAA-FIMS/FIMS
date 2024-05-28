
test_that("Parameter vector works as expected", {

v_size <- 10
v1_value <- 1.0
v2_value <- 2.0

v1 <- create_vector(v_size)
v1$fill(v1_value)

v2 <- create_vector(v_size)
v2$fill(v2_value)


v3 <- v1+v2
v3_value<-v1_value+v2_value

for(i in 1:v_size){
   expect_equal(v3$at(i)$value, v3_value)
}

v3 <- v1-v2
v3_value <- v1_value-v2_value

for(i in 1:v_size){
   expect_equal(v3$at(i)$value, v3_value)
}

v3 <- v1*v2
v3_value <- v1_value*v2_value

for(i in 1:v_size){
   expect_equal(v3$at(i)$value, v3_value)
}

v3<- v1/v2
v3_value<-v1_value/v2_value

for(i in 1:v_size){
   expect_equal(v3$at(i)$value, v3_value)
}


p<-create_vector(100) #new(ParameterVector, 100)



p[1]$value
p[1]$value<-1
p[1]$value

for( i in 1:length(p)){
print(p[i]$id)
}

p$resize(5)

for( i in 1:length(p)){
print(p[i]$id)
}

p$resize(10)

for( i in 1:length(p)){
print(p[i]$id)
}


var<-p[1] + cos(p[2])
#str(var)
print(p[1]$value)
print(p[2]$value)
print(cos(p[2]$value))
#str(var)
#par$value<-3.1459
#str(var)
#a<-apply(X = v, MARGIN = 1, FUN = sum)
cc<-c(1,2,3)
dim(cc)
l<-p$data
r<-sum(p)
#str(r)
#q()
#str(l)
#a<-lapply(X = l, MARGIN = 1, FUN = sum)
x <- vector("numeric",   length = 10)


typeof(x)



p1<-new(ParameterVector, 10)
#str(fill)
#fill(v1,3.1459)

p2<-new(ParameterVector, 10)
})
