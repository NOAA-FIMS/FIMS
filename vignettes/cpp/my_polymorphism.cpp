#include <Rcpp.h>
#include <cmath>

  class Shape {
  public:
    //constructor
    Shape() {}

    virtual ~Shape() {}

    // Virtual function to calculate area
    virtual double area() = 0;

  };

class Circle : public Shape {
  double radius;

  public:
    Circle(double r) : Shape(){
      radius = r;
    }

  // Override area() for Circle
  virtual double area() {
    return M_PI * radius * radius;
  }
};

class Rectangle : public Shape {
  private:
    double length, height;

  public:
    Rectangle(double l, double h) : Shape(){
      length = l;
      height = h;
    }

  // Override area() for Rectangle
  virtual double area()  {
    return length * height;
  }
};

RCPP_MODULE(shape) {
  Rcpp::class_<Shape>("Shape")
  .method("area", &Shape::area);

  Rcpp::class_<Circle>("Circle")
    .derives<Shape>("Shape")
    .constructor<double>();

  Rcpp::class_<Rectangle>("Rectangle")
    .derives<Shape>("Shape")
    .constructor<double, double>();
}