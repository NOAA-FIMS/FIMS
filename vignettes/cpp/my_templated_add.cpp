#include <iostream>

template <typename Type>
  Type my_add(Type x, Type y){
    return x + y;
  }

int main(){
  //works with integers
  std::cout << "Type is int: " << my_add(1,2) << std::endl;

  //works with floats
  std::cout << "Type is float: " << my_add(1.2, 3.2) << std::endl;

  //works with doubles
  std::cout << "Type is double: " << my_add(1.52757396774, 6.83480375227) << std::endl;

  return 0;

}