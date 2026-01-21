#include <Rcpp.h> // provides access to Rcpp library
#include <iostream> // provides access to std library
#include <string> // provides access to ssqng library

class Shape{

public:
double length;
double width;


Shape(){}

};

class Rectangle : public Shape{
public:
  Rectangle() : Shape() {}

  public:
  //this->length: points to length within base class, etc.
  double area(){
    return this->length * this->width;
  }
};

class Square : public Shape{
public:
  Square() : Shape() {}

  public:
  double area(){
    return this->length * this->length;
  }
};

// [[Rcpp::export]]
double calculate_areas(std::string shape, double length, double width = 0){

  double out = 0;

  if(shape == "rectangle"){
      Rectangle rect;
      rect.length = length;
      rect.width = width;
      out = rect.area();
  } else if (shape == "square") {
      Square sq;
      sq.length = length;
      out = sq.area();
  } else {
      Rcpp::Rcout << "Invalid shape" << std::endl;
  }
  return out;
}