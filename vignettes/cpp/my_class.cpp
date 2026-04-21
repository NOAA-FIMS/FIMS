#include <Rcpp.h> // provides access to Rcpp library
#include <iostream> // provides access to std library
#include <memory> // provides access to shared pointers

class MyClass{
  //Access specifier: can be private, public, or protected
  public:
    //Data members: variables to be used
  float a;
  float b;

  MyClass(){} //Constructor - automatically called when new object created

  //Member functions: Methods to access data members
  float my_add(){
    return a + b;
  }

};


// [[Rcpp::export]]
void my_class_add(){ // use void since the main function has no return

  //create instance of class using a declaration
  MyClass m1;

  //members can be accessed using the . command
  m1.a = 1.2;
  m1.b = 2.4;

  Rcpp::Rcout << "m1.my_add() = " << m1.my_add() << std::endl;

  //create instance of class using a pointer
  MyClass *m2 = new MyClass();

  //members can be access using the -> command
  m2->a = 3.3;
  m2->b = 4.4;

  Rcpp::Rcout << "m2->my_add() = " << m2->my_add() << std::endl;

  //need to delete memory after use when initializing with new
  delete m2;

  // create instance of class with shared pointer
  // Note the type of the shared pointer is MyClass
  std::shared_ptr<MyClass> m3 = std::make_shared<MyClass>();

  //members can be access using the -> command
  m3->a = 4.2;
  m3->b = 1.7;

  Rcpp::Rcout << "m3->my_add() = " << m3->my_add() << std::endl;
}