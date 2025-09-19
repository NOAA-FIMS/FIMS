/**
 * @file rcpp_shared_primitive.hpp
 * @brief Code to create shared pointers to allow for the wrapper functions
 * in R to work correctly so the live objects do not go out of scope.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_SHARED_PRIMITIVE_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_SHARED_PRIMITIVE_HPP

#include <memory>

/**
 * @class SharedInt
 * @brief A class that provides shared ownership of an integer value.
 *
 * @details This class wraps a `std::shared_ptr` to provide a reference-counted
 * integer value. When a `SharedInt` object is copied, it shares the same
 * underlying integer with the original object. The integer's memory is
 * automatically deallocated when the last `SharedInt` object pointing to it
 * is destroyed.
 */
class SharedInt {
 private:
  std::shared_ptr<int> value;

 public:
  /**
   * @brief Construct a new `SharedInt` object.
   *
   */
  SharedInt() : value(std::make_shared<int>(0)) {}

  /**
   * @brief Constructs a new `SharedInt` object.
   *
   * Initializes the object by creating a `std::shared_ptr` to an integer
   * with the provided value.
   *
   * @param val The integer value to be stored in the shared pointer.
   */
  SharedInt(int val) : value(std::make_shared<int>(val)) {}

  /**
   * @brief Copy constructs a new `SharedInt` object.
   *
   * Initializes the object by creating a new shared pointer that
   * shares ownership of the integer managed by another `SharedInt` object.
   *
   * @param other The existing `SharedInt` object to copy from.
   */
  SharedInt(const SharedInt& other) : value(other.value) {}

  /**
   * @brief Overloaded assignment operator for copying a `SharedInt` object.
   *
   * Assigns the shared state of `other` to the current object. If the object
   * is not self-assigned, it will share ownership of the same integer value.
   *
   * @param other The `SharedInt` object to assign from.
   * @return A reference to the current object (`*this`) to allow for chaining
   * assignments.
   */
  SharedInt& operator=(const SharedInt& other) {
    if (this != &other) {
      value = other.value;
    }
    return *this;
  }

  /**
   * @brief Overloaded assignment operator for an integer value.
   *
   * Assigns a new integer value to the underlying shared integer. This
   * operation modifies the shared state of the object.
   *
   * @param other The integer value to assign.
   * @return A reference to the current object (`*this`).
   */
  SharedInt& operator=(const int& other) {
    *value = other;

    return *this;
  }

  /**
   * @brief Constructs a new `SharedInt` object by moving resources.
   *
   * Transfers ownership of the shared integer from `other` to the new object.
   * This is a non-throwing operation that is more efficient than a copy.
   *
   * @param other The temporary `SharedInt` object to move from. After this
   * call, `other` will no longer own the resource.
   */
  SharedInt(SharedInt&& other) noexcept : value(std::move(other.value)) {}

  /**
   * @brief Overloaded move assignment operator.
   *
   * Moves the shared state from `other` to the current object, releasing any
   * resources the current object might have held. This is more efficient than a
   * copy operation.
   *
   * @param other The temporary `SharedInt` object to move from.
   * @return A reference to the current object (`*this`).
   */
  SharedInt& operator=(SharedInt&& other) noexcept {
    if (this != &other) {
      value = std::move(other.value);
    }
    return *this;
  }

  // Access the value

  /**
   * @brief Retrieve the value of the integer.
   *
   * @return int
   */
  int get() const { return *value; }

  /**
   * @brief Change the value of the integer
   *
   * @param val An integer value to set.
   */
  void set(int val) { *value = val; }

  /**
   * @brief User-defined conversion to `int`.
   *
   * @details This conversion operator allows the object to be implicitly
   * converted to an `int`. The returned value is the integer managed by the
   * object's internal shared pointer, which is retrieved by calling `get()`.
   *
   * @return The integer value held by the object.
   */
  operator int() { return this->get(); }

  // Overloaded operators for SharedPrimitive

  /**
   * @brief Overloads the dereference operator.
   *
   * @details Provides a convenient way to access the integer value held by the
   * shared pointer. This function is `const`, meaning it provides read-only
   * access and does not modify the object's state.
   *
   * @return The integer value managed by the shared pointer.
   */
  int operator*() const { return *value; }

  /**
   * @brief Overloads the member access operator (`->`).
   *
   * @details Provides a way to access the integer value by returning a raw
   * pointer to it. This function returns the result of
   * `std::shared_ptr::get()`.
   *
   * @return A raw pointer (`int*`) to the integer managed by the shared
   * pointer.
   */
  int* operator->() { return value.get(); }

  /**
   * @brief Overloads the member access operator (`->`) for const objects.
   *
   * @details This function provides a way to get a read-only raw pointer to the
   * underlying integer. The `const` return type prevents modification of the
   * integer value through this pointer. The function itself is also `const`,
   * ensuring it can be called on `const` instances of the class.
   *
   * @return A `const` raw pointer to the integer managed by the shared pointer.
   */
  const int* operator->() const { return value.get(); }

  // Unary operators

  /**
   * @brief Overloads the prefix increment operator (`++`).
   *
   * @details Increments the integer value managed by the shared pointer. As a
   * prefix operator, it increments the value *before* returning the result.
   *
   * @return A reference to the current object (`*this`) after the increment
   * has occurred.
   */
  SharedInt& operator++() {
    ++(*value);
    return *this;
  }

  /**
   * @brief Overloads the postfix increment operator (`++`).
   *
   * @details This is a postfix operation. It first creates a copy of the
   * object's current state, then increments the underlying integer value, and
   * finally returns the temporary copy, which holds the original value.
   * A dummy parameter used by the compiler to distinguish this
   * operator from the prefix version.
   * @return A copy of the object's state *before* the increment occurred.
   */
  SharedInt operator++(int) {
    SharedInt temp(*this);
    ++(*value);
    return temp;
  }

  /**
   * @brief Overloads the prefix decrement operator (`--`).
   *
   * @details Decrements the integer value managed by the shared pointer. As a
   * prefix operator, it decrements the value *before* returning the result.
   *
   * @return A reference to the current object (`*this`) after the decrement
   * has occurred.
   */
  SharedInt& operator--() {
    --(*value);
    return *this;
  }

  /**
   * @brief Overloads the postfix decrement operator.
   *
   * @details This is a postfix operation. It first creates a copy of the
   * object's current state, then decrements the underlying integer value, and
   * finally returns the temporary copy, which holds the original value.
   * A dummy parameter used by the compiler to distinguish this
   * operator from the prefix version.
   * @return A copy of the object's state *before* the decrement occurred.
   */
  SharedInt operator--(int) {
    SharedInt temp(*this);
    --(*value);
    return temp;
  }

  // Arithmetic operators with SharedPrimitive

  /**
   * @brief Overloads the binary addition operator (`+`).
   *
   * @details Adds the integer value of the current object to the integer value
   * of the `other` object. This operation does not modify either object; it
   * returns a new `SharedInt` object containing the sum.
   *
   * @param other The `SharedInt` object to add to the current object.
   * @return A new `SharedInt` object containing the sum of the two values.
   */
  SharedInt operator+(const SharedInt& other) const {
    return SharedInt(*value + *other.value);
  }

  /**
   * @brief Overloads the binary subtraction operator (`-`).
   *
   * @details Subtracts the integer value of the current object to the integer
   * value of the `other` object. This operation does not modify either object;
   * it returns a new `SharedInt` object containing the difference.
   *
   * @param other The `SharedInt` object to subtract from the current object.
   * @return A new `SharedInt` object containing the difference of the two
   * values.
   */
  SharedInt operator-(const SharedInt& other) const {
    return SharedInt(*value - *other.value);
  }

  /**
   * @brief Overloads the binary multiplication operator (`*`).
   *
   * @details Multiplies the integer value of the current object by the integer
   * value of the `other` object. This operation does not modify either object;
   * it returns a new `SharedInt` object containing the product.
   *
   * @param other The `SharedInt` object to multiply the current object by.
   * @return A new `SharedInt` object containing the product of the two values.
   */
  SharedInt operator*(const SharedInt& other) const {
    return SharedInt(*value * *other.value);
  }

  /**
   * @brief Overloads the binary division operator (`/`).
   *
   * @details Divides the integer value of the current object by the integer
   * value of the `other` object. This operation does not modify either object;
   * it returns a new `SharedInt` object containing the result of the division.
   *
   * @param other The `SharedInt` object to divide the current object by.
   * @return A new `SharedInt` object containing the quotient of the two values.
   */
  SharedInt operator/(const SharedInt& other) const {
    return SharedInt(*value / *other.value);
  }

  // Arithmetic operators with primitives

  /**
   * @brief Overloads the binary addition operator for an integer value.
   *
   * @details Adds an integer value to the integer value of the current object.
   * This operation does not modify the current object; it returns a new
   * `SharedInt` object containing the sum.
   *
   * @param other The integer value to add to the current object.
   * @return A new `SharedInt` object containing the sum of the two values.
   */
  SharedInt operator+(const int& other) const {
    return SharedInt(*value + other);
  }

  /**
   * @brief Overloads the binary subtraction operator for an integer value.
   *
   * @details Subtracts an integer value from the integer value of the current
   * object. This operation does not modify the current object; it returns a
   * new `SharedInt` object containing the result.
   *
   * @param other The integer value to subtract from the current object.
   * @return A new `SharedInt` object containing the result of the subtraction.
   */
  SharedInt operator-(const int& other) const {
    return SharedInt(*value - other);
  }

  /**
   * @brief Overloads the binary multiplication operator for an integer value.
   *
   * @details Multiplies the integer value of the current object by the integer
   * value of `other`. This operation does not modify the current object; it
   * returns a new `SharedInt` object containing the product.
   *
   * @param other The integer value to multiply the current object by.
   * @return A new `SharedInt` object containing the product of the two values.
   */
  SharedInt operator*(const int& other) const {
    return SharedInt(*value * other);
  }

  /**
   * @brief Overloads the binary division operator for an integer value.
   *
   * @details Divides the integer value of the current object by the integer
   * value of `other`. This operation does not modify the current object; it
   * returns a new `SharedInt` object containing the quotient.
   *
   * @param other The integer value to divide the current object by.
   * @return A new `SharedInt` object containing the quotient of the two values.
   */
  SharedInt operator/(const int& other) const {
    return SharedInt(*value / other);
  }

  // Compound assignment operators with SharedPrimitive

  /**
   * @brief Overloads the compound assignment operator (`+=`).
   *
   * @details Adds the integer value of `other` to the current object's value,
   * modifying the object in place.
   *
   * @param other The `SharedInt` object to add.
   * @return A reference to the current object (`*this`) after the addition.
   */
  SharedInt& operator+=(const SharedInt& other) {
    *value += *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`-=`).
   *
   * @details Subtracts the integer value of `other` from the current object's
   * value, modifying the object in place.
   *
   * @param other The `SharedInt` object to subtract.
   * @return A reference to the current object (`*this`) after the subtraction.
   */
  SharedInt& operator-=(const SharedInt& other) {
    *value -= *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`*=`).
   *
   * @details Multiplies the integer value of the current object by the value
   * of `other`, modifying the object in place.
   *
   * @param other The `SharedInt` object to multiply by.
   * @return A reference to the current object (`*this`) after the
   * multiplication.
   */
  SharedInt& operator*=(const SharedInt& other) {
    *value *= *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`/=`).
   *
   * @details Divides the integer value of the current object by the value
   * of `other`, modifying the object in place.
   *
   * @param other The `SharedInt` object to divide by.
   * @return A reference to the current object (`*this`) after the division.
   */
  SharedInt& operator/=(const SharedInt& other) {
    *value /= *other.value;
    return *this;
  }

  // Compound assignment operators with primitives

  /**
   * @brief Overloads the compound assignment operator (`+=`) for a shared
   * integer value.
   *
   * @details Adds an integer value to the current object's value, modifying the
   * object in place.
   *
   * @param other The integer value to add to the current object.
   * @return A reference to the current object (`*this`) after the addition.
   */
  SharedInt& operator+=(const int& other) {
    *value += other;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`-=`) for a shared
   * integer value.
   *
   * @details Subtracts an integer value from the current object's value,
   * modifying the object in place.
   *
   * @param other The integer value to subtract from the current object.
   * @return A reference to the current object (`*this`) after the subtraction.
   */
  SharedInt& operator-=(const int& other) {
    *value -= other;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`*=`) for a shared
   * integer value.
   *
   * @details Multiplies the integer value of the current object by the integer
   * value of `other`, modifying the object in place.
   *
   * @param other The integer value to multiply the current object by.
   * @return A reference to the current object (`*this`) after the
   * multiplication.
   */
  SharedInt& operator*=(const int& other) {
    *value *= other;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`/=`) for a shared
   * integer value.
   *
   * @details Divides the integer value of the current object by the integer
   * value of `other`, modifying the object in place.
   *
   * @param other The integer value to divide the current object by.
   * @return A reference to the current object (`*this`) after the division.
   */
  SharedInt& operator/=(const int& other) {
    *value /= other;
    return *this;
  }

  // Relational operators

  /**
   * @brief Overloads the equality operator (`==`).
   *
   * @details Compares the integer value of the current object with the integer
   * value of `other` for equality. This is a non-mutating operation.
   *
   * @param other The `SharedInt` object to compare against.
   * @return `true` if the values are equal, `false` otherwise.
   */
  bool operator==(const SharedInt& other) const {
    return *value == *other.value;
  }

  /**
   * @brief Overloads the inequality operator (`!=`).
   *
   * @details Compares the integer value of the current object with the integer
   * value of `other` for inequality. This is a non-mutating operation.
   *
   * @param other The `SharedInt` object to compare against.
   * @return `true` if the values are not equal, `false` otherwise.
   */
  bool operator!=(const SharedInt& other) const {
    return *value != *other.value;
  }

  /**
   * @brief Overloads the less-than operator (`<`).
   *
   * @details Compares the integer value of the current object with the integer
   * value of `other`. This is a non-mutating operation.
   *
   * @param other The `SharedInt` object to compare against.
   * @return `true` if the current object's value is less than `other`'s value,
   * `false` otherwise.
   */
  bool operator<(const SharedInt& other) const { return *value < *other.value; }

  /**
   * @brief Overloads the less-than-or-equal-to operator (`<=`).
   *
   * @details Compares the integer value of the current object with the integer
   * value of `other`. This is a non-mutating operation.
   *
   * @param other The `SharedInt` object to compare against.
   * @return `true` if the current object's value is less than or equal to
   * `other`'s value, `false` otherwise.
   */
  bool operator<=(const SharedInt& other) const {
    return *value <= *other.value;
  }

  /**
   * @brief Overloads the greater-than operator (`>`).
   *
   * @details Compares the integer value of the current object with the integer
   * value of `other`. This is a non-mutating operation.
   *
   * @param other The `SharedInt` object to compare against.
   * @return `true` if the current object's value is greater than `other`'s
   * value, `false` otherwise.
   */
  bool operator>(const SharedInt& other) const { return *value > *other.value; }

  /**
   * @brief Overloads the greater-than-or-equal-to operator (`>=`).
   *
   * @details Compares the integer value of the current object with the integer
   * value of `other`. This is a non-mutating operation.
   *
   * @param other The `SharedInt` object to compare against.
   * @return `true` if the current object's value is greater than or equal to
   * `other`'s value, `false` otherwise.
   */
  bool operator>=(const SharedInt& other) const {
    return *value >= *other.value;
  }

  // Relational operators with primitives

  /**
   * @brief Overloads the equality operator (`==`) for an integer value.
   *
   * @details Compares the integer value of the current object with an `int`
   * for equality. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the values are equal, `false` otherwise.
   */
  bool operator==(const int& other) const { return *value == other; }

  /**
   * @brief Overloads the inequality operator (`!=`) for an integer value.
   *
   * @details Compares the integer value of the current object with an `int`
   * for inequality. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the values are not equal, `false` otherwise.
   */
  bool operator!=(const int& other) const { return *value != other; }

  /**
   * @brief Overloads the less-than operator (`<`) for an integer value.
   *
   * @details Compares the integer value of the current object with an `int`.
   * This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is less than `other`, `false`
   * otherwise.
   */
  bool operator<(const int& other) const { return *value < other; }

  /**
   * @brief Overloads the less-than-or-equal-to operator (`<=`) for an integer
   * value.
   *
   * @details Compares the integer value of the current object with an `int`.
   * This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is less than or equal to
   * `other`, `false` otherwise.
   */
  bool operator<=(const int& other) const { return *value <= other; }

  /**
   * @brief Overloads the greater-than operator (`>`) for an integer value.
   *
   * @details Compares the integer value of the current object with an `int`.
   * This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is greater than `other`,
   * `false` otherwise.
   */
  bool operator>(const int& other) const { return *value > other; }

  /**
   * @brief Overloads the greater-than-or-equal-to operator (`>=`) for an
   * integer value.
   *
   * @details Compares the integer value of the current object with an `int`.
   * This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is greater than or equal to
   * `other`, `false` otherwise.
   */
  bool operator>=(const int& other) const { return *value >= other; }

  /**
   * @brief Overloads the stream insertion operator (`<<`) for `SharedInt`.
   *
   * @details Provides a way to print the integer value of a `SharedInt` object
   * to an output stream.
   *
   * @param os The output stream to write to.
   * @param sp The `SharedInt` object to print.
   * @return A reference to the output stream, allowing for chained insertions.
   */
  friend std::ostream& operator<<(std::ostream& os, const SharedInt& sp) {
    os << *sp.value;
    return os;
  }
};

// Non-member operators for primitive + SharedPrimitive

/**
 * @brief Overloads the binary addition operator (`+`) for an integer
 * left-hand side operand.
 *
 * @details Adds an integer to a `SharedInt`'s value, returning a new
 * `SharedInt` object with the result. This enables expressions
 * like `5 + my_shared_int`.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return A new `SharedInt` object containing the sum.
 */
SharedInt operator+(const int& lhs, const SharedInt& rhs) {
  return SharedInt(lhs + rhs.get());
}

/**
 * @brief Overloads the binary subtraction operator (`-`) for an integer
 * left-hand side operand.
 *
 * @details Subtracts a `SharedInt`'s value from an integer, returning a
 * new `SharedInt` object with the result. This enables expressions
 * like `10 - my_shared_int`.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return A new `SharedInt` object containing the result of the subtraction.
 */
SharedInt operator-(const int& lhs, const SharedInt& rhs) {
  return SharedInt(lhs - rhs.get());
}

/**
 * @brief Overloads the binary multiplication operator (`*`) for an
 * integer left-hand side operand.
 *
 * @details Multiplies an integer by a `SharedInt`'s value, returning a
 * new `SharedInt` object with the product.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return A new `SharedInt` object containing the product.
 */
SharedInt operator*(const int& lhs, const SharedInt& rhs) {
  return SharedInt(lhs * rhs.get());
}

/**
 * @brief Overloads the binary division operator (`/`) for an integer
 * left-hand side operand.
 *
 * @details Divides an integer by a `SharedInt`'s value, returning a new
 * `SharedInt` object with the quotient.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return A new `SharedInt` object containing the quotient.
 */
SharedInt operator/(const int& lhs, const SharedInt& rhs) {
  return SharedInt(lhs / rhs.get());
}

/**
 * @brief Overloads the less-than operator (`<`) for an integer
 * left-hand side operand.
 *
 * @details Compares an integer to a `SharedInt`'s value.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return `true` if the integer is less than the `SharedInt`'s value,
 * `false` otherwise.
 */
bool operator<(const int& lhs, const SharedInt& rhs) {
  return (lhs < rhs.get());
}

/**
 * @brief Overloads the less-than-or-equal-to operator (`<=`) for an
 * integer left-hand side operand.
 *
 * @details Compares an integer to a `SharedInt`'s value.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return `true` if the integer is less than or equal to the `SharedInt`'s
 * value, `false` otherwise.
 */
bool operator<=(const int& lhs, const SharedInt& rhs) {
  return (lhs <= rhs.get());
}

/**
 * @brief Overloads the greater-than operator (`>`) for an integer
 * left-hand side operand.
 *
 * @details Compares an integer to a `SharedInt`'s value.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return `true` if the integer is greater than the `SharedInt`'s value,
 * `false` otherwise.
 */
bool operator>(const int& lhs, const SharedInt& rhs) {
  return (lhs > rhs.get());
}

/**
 * @brief Overloads the greater-than-or-equal-to operator (`>=`) for an
 * integer left-hand side operand.
 *
 * @details Compares an integer to a `SharedInt`'s value.
 *
 * @param lhs The integer value on the left-hand side of the operator.
 * @param rhs The `SharedInt` object on the right-hand side.
 * @return `true` if the integer is greater than or equal to the
 * `SharedInt`'s value, `false` otherwise.
 */
bool operator>=(const int& lhs, const SharedInt& rhs) {
  return (lhs >= rhs.get());
}

/**
 * @class SharedReal
 * @brief A class that provides shared ownership of an integer value.
 *
 * @details This class wraps a `std::shared_ptr` to provide a reference-counted
 * integer value. When a SharedReal object is copied, it shares the same
 * underlying integer with the original object. The integer's memory is
 * automatically deallocated when the last SharedReal object pointing to it
 * is destroyed.
 */
class SharedReal {
 private:
  std::shared_ptr<int> value;

 public:
  /**
   * @brief Constructs a new `SharedReal` object with a default value.
   *
   * @details Initializes the object's internal shared pointer to manage a new
   * integer with a value of zero.
   */
  SharedReal() : value(std::make_shared<int>(0)) {}

  /**
   * @brief Constructs a new `SharedReal` object from an integer value.
   *
   * @details Initializes the object's internal shared pointer to manage a new
   * integer with the value provided by the `val` parameter.
   *
   * @param val The integer value to be stored in the shared pointer.
   */
  SharedReal(int val) : value(std::make_shared<int>(val)) {}

  /**
   * @brief Copy constructs a new `SharedReal` object.
   *
   * @details Initializes the object by creating a new shared pointer that
   * shares ownership of the integer managed by another `SharedReal` object.
   *
   * @param other The existing `SharedReal` object to copy from.
   */
  SharedReal(const SharedReal& other) : value(other.value) {}

  /**
   * @brief Overloaded copy assignment operator.
   *
   * @details Assigns the shared state of `other` to the current object. If the
   * objects are not the same, the current object will release its old resource
   * and share ownership of the integer value managed by `other`.
   *
   * @param other The `SharedReal` object to assign from.
   * @return A reference to the current object (`*this`) to allow for chained
   * assignments.
   */
  SharedReal& operator=(const SharedReal& other) {
    if (this != &other) {
      value = other.value;
    }
    return *this;
  }

  /**
   * @brief Overloaded assignment operator for an integer value.
   *
   * @details Assigns a new integer value to the underlying shared integer,
   * modifying the object in place.
   *
   * @param other The integer value to assign.
   * @return A reference to the current object (`*this`) to allow for
   * chained assignments.
   */
  SharedReal& operator=(const int& other) {
    *value = other;

    return *this;
  }

  /**
   * @brief Constructs a new `SharedReal` object by moving resources.
   *
   * @details Transfers ownership of the shared integer from `other` to the new
   * object. This is a non-throwing operation that is more efficient than a
   * copy and is also known as a move constructor.
   *
   * @param other The temporary `SharedReal` object to move from. After this
   * call, `other` will no longer own the resource.
   */
  SharedReal(SharedReal&& other) noexcept : value(std::move(other.value)) {}

  /**
   * @brief Overloaded move assignment operator.
   *
   * @details Moves the shared state from `other` to the current object,
   * releasing any resources the current object might have held. This is a
   * non-throwing and more efficient operation than a copy assignment and is
   * also known as a move assignment operator.
   *
   * @param other The temporary `SharedReal` object to move from.
   * @return A reference to the current object (`*this`).
   */
  SharedReal& operator=(SharedReal&& other) noexcept {
    if (this != &other) {
      value = std::move(other.value);
    }
    return *this;
  }

  /**
   * @brief Retrieves the integer value managed by the object.
   *
   * @details This is a getter function that returns a copy of the integer
   * value. Since the function is `const`, it can be safely called on a constant
   * instance of the class without modifying its state.
   *
   * @return The integer value managed by the shared pointer.
   */
  int get() const { return *value; }

  /**
   * @brief Sets the integer value of the object.
   *
   * @details Assigns a new integer value to the underlying shared integer.
   *
   * @param val The integer value to set.
   */
  void set(int val) { *value = val; }

  /**
   * @brief User-defined conversion to `int`.
   *
   * @details This conversion operator allows the object to be implicitly
   * converted to an `int`. The returned value is the integer managed by the
   * object's internal shared pointer, which is retrieved by calling `get()`.
   *
   * @return The integer value held by the object.
   */
  operator int() { return this->get(); }

  // Overloaded operators for SharedPrimitive

  /**
   * @brief Overloads the dereference operator (`*`).
   *
   * @details Provides direct access to the integer value managed by the
   * internal shared pointer. This function is `const` and provides a
   * read-only view of the data.
   *
   * @return The integer value held by the object.
   */
  int operator*() const { return *value; }

  /**
   * @brief Overloads the member access operator (`->`).
   *
   * @details This function provides a way to access the underlying integer
   * value by returning a raw pointer to it. It returns the result of the
   * `std::shared_ptr::get()` method.
   *
   * @return A raw pointer (`int*`) to the integer managed by the shared
   * pointer.
   */
  int* operator->() { return value.get(); }

  /**
   * @brief Overloads the member access operator (`->`) for const objects.
   *
   * @details This function provides a way to get a read-only raw pointer to the
   * underlying integer. The `const` return type prevents modification of the
   * integer value through this pointer. The function itself is also `const`,
   * ensuring it can be called on `const` instances of the class.
   *
   * @return A `const` raw pointer to the integer managed by the shared pointer.
   */
  const int* operator->() const { return value.get(); }

  // Unary operators

  /**
   * @brief Overloads the prefix increment operator (`++`).
   *
   * @details Increments the integer value managed by the shared pointer. As a
   * prefix operator, it increments the value *before* returning the result.
   *
   * @return A reference to the current object (`*this`) after the increment has
   * occurred.
   */
  SharedReal& operator++() {
    ++(*value);
    return *this;
  }

  /**
   * @brief Overloads the postfix increment operator.
   *
   * @details This is a postfix operation. It first creates a copy of the
   * object's current state, then increments the underlying integer value, and
   * finally returns the temporary copy, which holds the original value.
   * A dummy parameter used by the compiler to distinguish this
   * operator from the prefix version.
   * @return A copy of the object's state *before* the increment occurred.
   */
  SharedReal operator++(int) {
    SharedReal temp(*this);
    ++(*value);
    return temp;
  }

  /**
   * @brief Overloads the prefix decrement operator (`--`).
   *
   * @details Decrements the integer value managed by the shared pointer.
   * As a prefix operator, it decrements the value *before* returning the
   * result.
   *
   * @return A reference to the current object (`*this`) after the decrement has
   * occurred.
   */
  SharedReal& operator--() {
    --(*value);
    return *this;
  }

  /**
   * @brief Overloads the postfix decrement operator.
   *
   * @details This is a postfix operation. It first creates a copy of the
   * object's current state, then decrements the underlying integer value, and
   * finally returns the temporary copy, which holds the original value.
   * A dummy parameter used by the compiler to distinguish this
   * operator from the prefix version.
   * @return A copy of the object's state *before* the decrement occurred.
   */
  SharedReal operator--(int) {
    SharedReal temp(*this);
    --(*value);
    return temp;
  }

  // Arithmetic operators with SharedPrimitive

  /**
   * @brief Overloads the binary addition operator (`+`).
   *
   * @details Adds the integer value of the current object to the integer
   * value of `other`. This operation does not modify either object; it
   * returns a new `SharedReal` object containing the sum.
   *
   * @param other The `SharedReal` object to add to the current object.
   * @return A new `SharedReal` object containing the sum of the two values.
   */
  SharedReal operator+(const SharedReal& other) const {
    return SharedReal(*value + *other.value);
  }

  /**
   * @brief Overloads the binary subtraction operator (`-`).
   *
   * @details Subtracts the integer value of `other` from the current
   * object's value. This operation does not modify either object; it
   * returns a new `SharedReal` object containing the result.
   *
   * @param other The `SharedReal` object to subtract from the current
   * object.
   * @return A new `SharedReal` object containing the result of the
   * subtraction.
   */
  SharedReal operator-(const SharedReal& other) const {
    return SharedReal(*value - *other.value);
  }

  /**
   * @brief Overloads the binary multiplication operator (`*`).
   *
   * @details Multiplies the integer value of the current object by the
   * integer value of `other`. This operation does not modify either
   * object; it returns a new `SharedReal` object containing the product.
   *
   * @param other The `SharedReal` object to multiply by.
   * @return A new `SharedReal` object containing the product of the two
   * values.
   */
  SharedReal operator*(const SharedReal& other) const {
    return SharedReal(*value * *other.value);
  }

  /**
   * @brief Overloads the binary division operator (`/`).
   *
   * @details Divides the integer value of the current object by the
   * integer value of `other`. This operation does not modify either
   * object; it returns a new `SharedReal` object containing the quotient.
   *
   * @param other The `SharedReal` object to divide by.
   * @return A new `SharedReal` object containing the quotient of the two
   * values.
   */
  SharedReal operator/(const SharedReal& other) const {
    return SharedReal(*value / *other.value);
  }

  // Arithmetic operators with primitives

  /**
   * @brief Overloads the binary addition operator (`+`) for an integer.
   *
   * @details Adds an integer value to the current object's value. This
   * operation does not modify the current object; it returns a new
   * `SharedReal` object containing the sum.
   *
   * @param other The integer value to add to the current object.
   * @return A new `SharedReal` object containing the sum.
   */
  SharedReal operator+(const int& other) const {
    return SharedReal(*value + other);
  }

  /**
   * @brief Overloads the binary subtraction operator (`-`) for an integer.
   *
   * @details Subtracts an integer value from the current object's value.
   * This operation does not modify the current object; it returns a new
   * `SharedReal` object containing the result.
   *
   * @param other The integer value to subtract from the current object.
   * @return A new `SharedReal` object containing the result.
   */
  SharedReal operator-(const int& other) const {
    return SharedReal(*value - other);
  }

  /**
   * @brief Overloads the binary multiplication operator (`*`) for an
   * integer.
   *
   * @details Multiplies the current object's value by an integer. This
   * operation does not modify the current object; it returns a new
   * `SharedReal` object containing the product.
   *
   * @param other The integer value to multiply the current object by.
   * @return A new `SharedReal` object containing the product.
   */
  SharedReal operator*(const int& other) const {
    return SharedReal(*value * other);
  }

  /**
   * @brief Overloads the binary division operator (`/`) for an integer.
   *
   * @details Divides the current object's value by an integer. This
   * operation does not modify the current object; it returns a new
   * `SharedReal` object containing the quotient.
   *
   * @param other The integer value to divide the current object by.
   * @return A new `SharedReal` object containing the quotient.
   */
  SharedReal operator/(const int& other) const {
    return SharedReal(*value / other);
  }

  // Compound assignment operators with SharedPrimitive

  /**
   * @brief Overloads the compound assignment operator (`+=`).
   *
   * @details Adds the integer value of `other` to the current object's
   * value, modifying the object in place.
   *
   * @param other The SharedReal object to add.
   * @return A reference to the current object (`*this`) after the
   * addition.
   */
  SharedReal& operator+=(const SharedReal& other) {
    *value += *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`-=`).
   *
   * @details Subtracts the integer value of `other` from the current
   * object's value, modifying the object in place.
   *
   * @param other The SharedReal object to subtract.
   * @return A reference to the current object (`*this`) after the
   * subtraction.
   */
  SharedReal& operator-=(const SharedReal& other) {
    *value -= *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`*=`).
   *
   * @details Multiplies the integer value of the current object by the
   * value of `other`, modifying the object in place.
   *
   * @param other The SharedReal object to multiply by.
   * @return A reference to the current object (`*this`) after the
   * multiplication.
   */
  SharedReal& operator*=(const SharedReal& other) {
    *value *= *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`/=`).
   *
   * @details Divides the integer value of the current object by the
   * value of `other`, modifying the object in place.
   *
   * @param other The SharedReal object to divide by.
   * @return A reference to the current object (`*this`) after the
   * division.
   */
  SharedReal& operator/=(const SharedReal& other) {
    *value /= *other.value;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`+=`) for an
   * integer.
   *
   * @details Adds an integer value to the current object's value,
   * modifying the object in place.
   *
   * @param other The integer value to add to the current object.
   * @return A reference to the current object (`*this`) after the
   * addition.
   */
  SharedReal& operator+=(const int& other) {
    *value += other;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`-=`) for an
   * integer.
   *
   * @details Subtracts an integer value from the current object's value,
   * modifying the object in place.
   *
   * @param other The integer value to subtract from the current object.
   * @return A reference to the current object (`*this`) after the
   * subtraction.
   */
  SharedReal& operator-=(const int& other) {
    *value -= other;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`*=`) for an
   * integer.
   *
   * @details Multiplies the integer value of the current object by the
   * integer value of `other`, modifying the object in place.
   *
   * @param other The integer value to multiply the current object by.
   * @return A reference to the current object (`*this`) after the
   * multiplication.
   */
  SharedReal& operator*=(const int& other) {
    *value *= other;
    return *this;
  }

  /**
   * @brief Overloads the compound assignment operator (`/=`) for an
   * integer.
   *
   * @details Divides the integer value of the current object by the
   * integer value of `other`, modifying the object in place.
   *
   * @param other The integer value to divide the current object by.
   * @return A reference to the current object (`*this`) after the
   * division.
   */
  SharedReal& operator/=(const int& other) {
    *value /= other;
    return *this;
  }

  // Relational operators

  /**
   * @brief Overloads the equality operator (`==`).
   *
   * @details Compares the integer value of the current object with the
   * integer value of `other` for equality. This is a non-mutating
   * operation.
   *
   * @param other The SharedReal object to compare against.
   * @return `true` if the values are equal, `false` otherwise.
   */
  bool operator==(const SharedReal& other) const {
    return *value == *other.value;
  }

  /**
   * @brief Overloads the inequality operator (`!=`).
   *
   * @details Compares the integer value of the current object with the
   * integer value of `other` for inequality. This is a non-mutating
   * operation.
   *
   * @param other The SharedReal object to compare against.
   * @return `true` if the values are not equal, `false` otherwise.
   */
  bool operator!=(const SharedReal& other) const {
    return *value != *other.value;
  }

  /**
   * @brief Overloads the less-than operator (`<`).
   *
   * @details Compares the integer value of the current object with the
   * integer value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedReal object to compare against.
   * @return `true` if the current object's value is less than `other`'s
   * value, `false` otherwise.
   */
  bool operator<(const SharedReal& other) const {
    return *value < *other.value;
  }

  /**
   * @brief Overloads the less-than-or-equal-to operator (`<=`).
   *
   * @details Compares the integer value of the current object with the
   * integer value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedReal object to compare against.
   * @return `true` if the current object's value is less than or equal
   * to `other`'s value, `false` otherwise.
   */
  bool operator<=(const SharedReal& other) const {
    return *value <= *other.value;
  }

  /**
   * @brief Overloads the greater-than operator (`>`).
   *
   * @details Compares the integer value of the current object with the
   * integer value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedReal object to compare against.
   * @return `true` if the current object's value is greater than `other`'s
   * value, `false` otherwise.
   */
  bool operator>(const SharedReal& other) const {
    return *value > *other.value;
  }

  /**
   * @brief Overloads the greater-than-or-equal-to operator (`>=`).
   *
   * @details Compares the integer value of the current object with the
   * integer value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedReal object to compare against.
   * @return `true` if the current object's value is greater than or
   * equal to `other`'s value, `false` otherwise.
   */
  bool operator>=(const SharedReal& other) const {
    return *value >= *other.value;
  }

  // Relational operators with primitives

  /**
   * @brief Overloads the equality operator (`==`) for an integer value.
   *
   * @details Compares the integer value of the current object with an
   * `int` for equality. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the values are equal, `false` otherwise.
   */
  bool operator==(const int& other) const { return *value == other; }

  /**
   * @brief Overloads the inequality operator (`!=`) for an integer.
   *
   * @details Compares the integer value of the current object with an
   * `int` for inequality. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the values are not equal, `false` otherwise.
   */
  bool operator!=(const int& other) const { return *value != other; }

  /**
   * @brief Overloads the less-than operator (`<`) for an integer.
   *
   * @details Compares the integer value of the current object with an
   * `int`. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is less than `other`,
   * `false` otherwise.
   */
  bool operator<(const int& other) const { return *value < other; }

  /**
   * @brief Overloads the less-than-or-equal-to operator (`<=`) for an
   * integer.
   *
   * @details Compares the integer value of the current object with an
   * `int`. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is less than or equal
   * to `other`, `false` otherwise.
   */
  bool operator<=(const int& other) const { return *value <= other; }

  /**
   * @brief Overloads the greater-than operator (`>`) for an integer.
   *
   * @details Compares the integer value of the current object with an
   * `int`. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is greater than
   * `other`, `false` otherwise.
   */
  bool operator>(const int& other) const { return *value > other; }

  /**
   * @brief Overloads the greater-than-or-equal-to operator (`>=`) for an
   * integer.
   *
   * @details Compares the integer value of the current object with an
   * `int`. This is a non-mutating operation.
   *
   * @param other The integer value to compare against.
   * @return `true` if the current object's value is greater than or
   * equal to `other`, `false` otherwise.
   */
  bool operator>=(const int& other) const { return *value >= other; }

  /**
   * @brief Overloads the stream insertion operator (`<<`) for SharedReal.
   *
   * @details Provides a way to print the integer value of a SharedReal object
   * to an output stream.
   *
   * @param os The output stream to write to.
   * @param sp The SharedReal object to print.
   * @return A reference to the output stream, allowing for chained insertions.
   */
  friend std::ostream& operator<<(std::ostream& os, const SharedReal& sp) {
    os << *sp.value;
    return os;
  }
};

/**
 * @brief Overloads the binary addition operator (`+`) for a double
 * left-hand side operand.
 *
 * @details Adds a double to a SharedReal's value, returning a new
 * SharedReal object with the result. This enables expressions
 * like `5.0 + my_shared_real`.
 *
 * @param lhs The double value on the left-hand side of the operator.
 * @param rhs The SharedReal object on the right-hand side.
 * @return A new SharedReal object containing the sum.
 */
SharedReal operator+(const double& lhs, const SharedReal& rhs) {
  return SharedReal(lhs + rhs.get());
}

/**
 * @brief Overloads the binary subtraction operator (`-`) for a double
 * left-hand side operand.
 *
 * @details Subtracts a SharedReal's value from a double, returning a
 * new SharedReal object with the result. This enables expressions
 * like `10.0 - my_shared_real`.
 *
 * @param lhs The double value on the left-hand side of the operator.
 * @param rhs The SharedReal object on the right-hand side.
 * @return A new SharedReal object containing the result of the
 * subtraction.
 */
SharedReal operator-(const double& lhs, const SharedReal& rhs) {
  return SharedReal(lhs - rhs.get());
}

/**
 * @brief Overloads the binary multiplication operator (`*`) for a
 * double left-hand side operand.
 *
 * @details Multiplies a double by a SharedReal's value, returning a
 * new SharedReal object with the product.
 *
 * @param lhs The double value on the left-hand side of the operator.
 * @param rhs The SharedReal object on the right-hand side.
 * @return A new SharedReal object containing the product.
 */
SharedReal operator*(const double& lhs, const SharedReal& rhs) {
  return SharedReal(lhs * rhs.get());
}

/**
 * @brief Overloads the binary division operator (`/`) for a double
 * left-hand side operand.
 *
 * @details Divides a double by a SharedReal's value, returning a new
 * SharedReal object with the quotient.
 *
 * @param lhs The double value on the left-hand side of the operator.
 * @param rhs The SharedReal object on the right-hand side.
 * @return A new SharedReal object containing the quotient.
 */
SharedReal operator/(const double& lhs, const SharedReal& rhs) {
  return SharedReal(lhs / rhs.get());
}

/**
 * @class SharedString
 * @brief A class that provides shared ownership of a string.
 *
 * @details This class wraps a `std::shared_ptr` to provide a reference-counted
 * string value. When a SharedString object is copied, it shares the same
 * underlying string with the original object. The string's memory is
 * automatically deallocated when the last SharedString object pointing to it
 * is destroyed.
 */
class SharedString {
 private:
  std::shared_ptr<std::string> value;

 public:
  /**
   * @brief Constructs a new SharedString object with a default, empty value.
   *
   * @details Initializes the object's internal shared pointer to manage a new
   * string that is initially empty.
   */
  SharedString() : value(std::make_shared<std::string>()) {}

  /**
   * @brief Constructs a new SharedString object from a string value.
   *
   * @details Initializes the object's internal shared pointer to manage a new
   * string with the value provided by the `val` parameter.
   *
   * @param val The string value to be stored in the shared pointer.
   */
  SharedString(std::string val) : value(std::make_shared<std::string>(val)) {}

  /**
   * @brief Copy constructs a new SharedString object.
   *
   * @details Initializes the object by creating a new shared pointer that
   * shares ownership of the string managed by another SharedString object.
   *
   * @param other The existing SharedString object to copy from.
   */
  SharedString(const SharedString& other) : value(other.value) {}

  /**
   * @brief Overloaded copy assignment operator.
   *
   * @details Assigns the shared state of `other` to the current object. If the
   * objects are not the same, the current object will release its old resource
   * and share ownership of the string value managed by `other`.
   *
   * @param other The SharedString object to assign from.
   * @return A reference to the current object (`*this`) to allow for chained
   * assignments.
   */
  SharedString& operator=(const SharedString& other) {
    if (this != &other) {
      value = other.value;
    }
    return *this;
  }

  /**
   * @brief Overloaded assignment operator for a string value.
   *
   * @details Assigns a new string value to the underlying shared string,
   * modifying the object in place.
   *
   * @param other The string value to assign.
   * @return A reference to the current object (`*this`) to allow for
   * chained assignments.
   */
  SharedString& operator=(const std::string& other) {
    *value = other;
    return *this;
  }

  /**
   * @brief Constructs a new SharedString object by moving resources.
   *
   * @details Transfers ownership of the shared string from `other` to
   * the new object. This is a non-throwing operation that is more
   * efficient than a copy.
   *
   * @param other The temporary SharedString object to move from. After
   * this call, `other` will no longer own the resource.
   */
  SharedString(SharedString&& other) noexcept : value(std::move(other.value)) {}

  /**
   * @brief Overloaded move assignment operator.
   *
   * @details Moves the shared state from `other` to the current object,
   * releasing any resources the current object might have held. This is a
   * non-throwing and more efficient operation than a copy assignment.
   *
   * @param other The temporary SharedString object to move from.
   * @return A reference to the current object (`*this`).
   */
  SharedString& operator=(SharedString&& other) noexcept {
    if (this != &other) {
      value = std::move(other.value);
    }
    return *this;
  }

  /**
   * @brief Retrieves the string value managed by the object.
   *
   * @details This is a getter function that returns a copy of the string
   * value. The function is `const` and can be called on a constant
   * instance of the class without modifying its state.
   *
   * @return The string value managed by the shared pointer.
   */
  std::string get() const { return *value; }

  /**
   * @brief Sets the string value of the object.
   *
   * @details Assigns a new string value to the underlying shared string.
   *
   * @param val The string value to set.
   */
  void set(std::string val) { *value = val; }

  /**
   * @brief User-defined conversion to `std::string`.
   *
   * @details This conversion operator allows the object to be implicitly
   * converted to a `std::string`. The returned value is the string
   * managed by the object's internal shared pointer.
   *
   * @return The string value held by the object.
   */
  operator std::string() { return this->get(); }

  /**
   * @brief Overloads the dereference operator (`*`).
   *
   * @details Provides direct access to the string value managed by the
   * internal shared pointer. This function is `const` and provides a
   * read-only view of the data.
   *
   * @return The string value held by the object.
   */
  std::string operator*() const { return *value; }

  /**
   * @brief Overloads the member access operator (`->`).
   *
   * @details This function provides a way to access the underlying
   * string value by returning a raw pointer to it. It returns the
   * result of the `std::shared_ptr::get()` method.
   *
   * @return A raw pointer (`std::string*`) to the string managed by
   * the shared pointer.
   */
  std::string* operator->() { return value.get(); }

  /**
   * @brief Overloads the member access operator (`->`) for const objects.
   *
   * @details This function provides a way to get a read-only raw pointer
   * to the underlying string. The `const` return type prevents
   * modification of the string value through this pointer. The function
   * itself is also `const`, ensuring it can be called on `const`
   * instances of the class.
   *
   * @return A `const` raw pointer to the string managed by the shared
   * pointer.
   */
  const std::string* operator->() const { return value.get(); }

  /**
   * @brief Overloads the stream insertion operator (`<<`) for
   * SharedString.
   *
   * @details Provides a way to print the string value of a SharedString
   * object to an output stream.
   *
   * @param os The output stream to write to.
   * @param sp The SharedString object to print.
   * @return A reference to the output stream, allowing for chained
   * insertions.
   */
  friend std::ostream& operator<<(std::ostream& os, const SharedString& sp) {
    os << *sp.value;
    return os;
  }
};

/**
 * @class SharedBoolean
 * @brief A class that provides shared ownership of a boolean value.
 *
 * @details This class wraps a `std::shared_ptr` to provide a reference-counted
 * boolean value. When a SharedBoolean object is copied, it shares the same
 * underlying boolean with the original object. The boolean's memory is
 * automatically deallocated when the last SharedBoolean object pointing to it
 * is destroyed.
 */
class SharedBoolean {
 private:
  std::shared_ptr<bool> value;

 public:
  /**
   * @brief Constructs a new SharedBoolean object with a default value.
   *
   * @details Initializes the object's internal shared pointer to manage a new
   * boolean with a default value of `false`.
   */
  SharedBoolean() : value(std::make_shared<bool>(0)) {}

  /**
   * @brief Constructs a new SharedBoolean object from a boolean value.
   *
   * @details Initializes the object's internal shared pointer to manage a new
   * boolean with the value provided by the `val` parameter.
   *
   * @param val The boolean value to be stored in the shared pointer.
   */
  SharedBoolean(bool val) : value(std::make_shared<bool>(val)) {}

  /**
   * @brief Copy constructs a new SharedBoolean object.
   *
   * @details Initializes the object by creating a new shared pointer that
   * shares ownership of the boolean managed by another SharedBoolean object.
   *
   * @param other The existing SharedBoolean object to copy from.
   */
  SharedBoolean(const SharedBoolean& other) : value(other.value) {}

  /**
   * @brief Overloaded copy assignment operator.
   *
   * @details Assigns the shared state of `other` to the current object. If the
   * objects are not the same, the current object will release its old resource
   * and share ownership of the boolean value managed by `other`.
   *
   * @param other The SharedBoolean object to assign from.
   * @return A reference to the current object (`*this`) to allow for chained
   * assignments.
   */
  SharedBoolean& operator=(const SharedBoolean& other) {
    if (this != &other) {
      value = other.value;
    }
    return *this;
  }

  /**
   * @brief Overloaded assignment operator for a boolean value.
   *
   * @details Assigns a new boolean value to the underlying shared boolean,
   * modifying the object in place.
   *
   * @param other The boolean value to assign.
   * @return A reference to the current object (`*this`) to allow for
   * chained assignments.
   */
  SharedBoolean& operator=(const bool& other) {
    *value = other;
    return *this;
  }

  /**
   * @brief Constructs a new SharedBoolean object by moving resources.
   *
   * @details Transfers ownership of the shared boolean from `other` to the new
   * object. This is a non-throwing operation that is more efficient than a
   * copy.
   *
   * @param other The temporary SharedBoolean object to move from. After this
   * call, `other` will no longer own the resource.
   */
  SharedBoolean(SharedBoolean&& other) noexcept
      : value(std::move(other.value)) {}

  /**
   * @brief Overloaded move assignment operator.
   *
   * @details Moves the shared state from `other` to the current object,
   * releasing any resources the current object might have held. This is a
   * non-throwing and more efficient operation than a copy assignment.
   *
   * @param other The temporary SharedBoolean object to move from.
   * @return A reference to the current object (`*this`).
   */
  SharedBoolean& operator=(SharedBoolean&& other) noexcept {
    if (this != &other) {
      value = std::move(other.value);
    }
    return *this;
  }

  /**
   * @brief Retrieves the boolean value managed by the object.
   *
   * @details This is a getter function that returns a copy of the boolean
   * value. Since the function is `const`, it can be safely called on a
   * constant instance of the class without modifying its state.
   *
   * @return The boolean value managed by the shared pointer.
   */
  bool get() const { return *value; }

  /**
   * @brief Sets the boolean value of the object.
   *
   * @details Assigns a new boolean value to the underlying shared boolean.
   *
   * @param val The boolean value to set.
   */
  void set(bool val) { *value = val; }

  /**
   * @brief User-defined conversion to `bool`.
   *
   * @details This conversion operator allows the object to be implicitly
   * converted to a `bool`. The returned value is the boolean managed by the
   * object's internal shared pointer.
   *
   * @return The boolean value held by the object.
   */
  operator bool() { return this->get(); }

  /**
   * @brief Overloads the dereference operator (`*`).
   *
   * @details Provides direct access to the boolean value managed by the
   * internal shared pointer. This function is `const` and provides a
   * read-only view of the data.
   *
   * @return The boolean value held by the object.
   */
  bool operator*() const { return *value; }

  /**
   * @brief Overloads the member access operator (`->`).
   *
   * @details This function provides a way to access the underlying boolean
   * value by returning a raw pointer to it. It returns the result of the
   * `std::shared_ptr::get()` method.
   *
   * @return A raw pointer (`bool*`) to the boolean managed by the
   * shared pointer.
   */
  bool* operator->() { return value.get(); }

  /**
   * @brief Overloads the member access operator (`->`) for const objects.
   *
   * @details This function provides a way to get a read-only raw pointer
   * to the underlying boolean. The `const` return type prevents
   * modification of the boolean value through this pointer. The function
   * itself is also `const`, ensuring it can be called on `const`
   * instances of the class.
   *
   * @return A `const` raw pointer to the boolean managed by the shared
   * pointer.
   */
  const bool* operator->() const { return value.get(); }

  /**
   * @brief Overloads the equality operator (`==`).
   *
   * @details Compares the boolean value of the current object with the
   * boolean value of `other` for equality. This is a non-mutating
   * operation.
   *
   * @param other The SharedBoolean object to compare against.
   * @return `true` if the values are equal, `false` otherwise.
   */
  bool operator==(const SharedBoolean& other) const {
    return *value == *other.value;
  }

  /**
   * @brief Overloads the inequality operator (`!=`).
   *
   * @details Compares the boolean value of the current object with the
   * boolean value of `other` for inequality. This is a non-mutating
   * operation.
   *
   * @param other The SharedBoolean object to compare against.
   * @return `true` if the values are not equal, `false` otherwise.
   */
  bool operator!=(const SharedBoolean& other) const {
    return *value != *other.value;
  }

  /**
   * @brief Overloads the less-than operator (`<`).
   *
   * @details Compares the boolean value of the current object with the
   * boolean value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedBoolean object to compare against.
   * @return `true` if the current object's value is less than `other`'s
   * value, `false` otherwise.
   */
  bool operator<(const SharedBoolean& other) const {
    return *value < *other.value;
  }

  /**
   * @brief Overloads the less-than-or-equal-to operator (`<=`).
   *
   * @details Compares the boolean value of the current object with the
   * boolean value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedBoolean object to compare against.
   * @return `true` if the current object's value is less than or equal
   * to `other`'s value, `false` otherwise.
   */
  bool operator<=(const SharedBoolean& other) const {
    return *value <= *other.value;
  }

  /**
   * @brief Overloads the greater-than operator (`>`).
   *
   * @details Compares the boolean value of the current object with the
   * boolean value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedBoolean object to compare against.
   * @return `true` if the current object's value is greater than `other`'s
   * value, `false` otherwise.
   */
  bool operator>(const SharedBoolean& other) const {
    return *value > *other.value;
  }

  /**
   * @brief Overloads the greater-than-or-equal-to operator (`>=`).
   *
   * @details Compares the boolean value of the current object with the
   * boolean value of `other`. This is a non-mutating operation.
   *
   * @param other The SharedBoolean object to compare against.
   * @return `true` if the current object's value is greater than or
   * equal to `other`'s value, `false` otherwise.
   */
  bool operator>=(const SharedBoolean& other) const {
    return *value >= *other.value;
  }

  /**
   * @brief Overloads the equality operator (`==`) for a boolean value.
   *
   * @details Compares the boolean value of the current object with a `bool`
   * for equality. This is a non-mutating operation.
   *
   * @param other The boolean value to compare against.
   * @return `true` if the values are equal, `false` otherwise.
   */
  bool operator==(const bool& other) const { return *value == other; }

  /**
   * @brief Overloads the inequality operator (`!=`) for a boolean value.
   *
   * @details Compares the boolean value of the current object with a `bool`
   * for inequality. This is a non-mutating operation.
   *
   * @param other The boolean value to compare against.
   * @return `true` if the values are not equal, `false` otherwise.
   */
  bool operator!=(const bool& other) const { return *value != other; }

  /**
   * @brief Overloads the less-than operator (`<`) for a boolean value.
   *
   * @details Compares the boolean value of the current object with a `bool`.
   * This is a non-mutating operation.
   *
   * @param other The boolean value to compare against.
   * @return `true` if the current object's value is less than `other`,
   * `false` otherwise.
   */
  bool operator<(const bool& other) const { return *value < other; }

  /**
   * @brief Overloads the less-than-or-equal-to operator (`<=`) for a boolean.
   *
   * @details Compares the boolean value of the current object with a `bool`.
   * This is a non-mutating operation.
   *
   * @param other The boolean value to compare against.
   * @return `true` if the current object's value is less than or equal
   * to `other`, `false` otherwise.
   */
  bool operator<=(const bool& other) const { return *value <= other; }

  /**
   * @brief Overloads the greater-than operator (`>`) for a boolean value.
   *
   * @details Compares the boolean value of the current object with a `bool`.
   * This is a non-mutating operation.
   *
   * @param other The boolean value to compare against.
   * @return `true` if the current object's value is greater than `other`,
   * `false` otherwise.
   */
  bool operator>(const bool& other) const { return *value > other; }

  /**
   * @brief Overloads the greater-than-or-equal-to operator (`>=`) for a
   * boolean.
   *
   * @details Compares the boolean value of the current object with a `bool`.
   * This is a non-mutating operation.
   *
   * @param other The boolean value to compare against.
   * @return `true` if the current object's value is greater than or
   * equal to `other`, `false` otherwise.
   */
  bool operator>=(const bool& other) const { return *value >= other; }

  /**
   * @brief Overloads the stream insertion operator (`<<`) for
   * SharedBoolean.
   *
   * @details Provides a way to print the boolean value of a SharedBoolean
   * object to an output stream.
   *
   * @param os The output stream to write to.
   * @param sp The SharedBoolean object to print.
   * @return A reference to the output stream, allowing for chained insertions.
   */
  friend std::ostream& operator<<(std::ostream& os, const SharedBoolean& sp) {
    os << *sp.value;
    return os;
  }
};

/**
 * @brief Overloads the less-than operator (`<`) for a boolean
 * left-hand side operand.
 *
 * @details Compares a boolean to a SharedBoolean's value.
 *
 * @param lhs The boolean value on the left-hand side of the operator.
 * @param rhs The SharedBoolean object on the right-hand side.
 * @return `true` if the boolean is less than the SharedBoolean's
 * value, `false` otherwise.
 */
bool operator<(const bool& lhs, const SharedBoolean& rhs) {
  return (lhs < rhs.get());
}

/**
 * @brief Overloads the less-than-or-equal-to operator (`<=`) for a
 * boolean left-hand side operand.
 *
 * @details Compares a boolean to a SharedBoolean's value.
 *
 * @param lhs The boolean value on the left-hand side of the operator.
 * @param rhs The SharedBoolean object on the right-hand side.
 * @return `true` if the boolean is less than or equal to the
 * SharedBoolean's value, `false` otherwise.
 */
bool operator<=(const bool& lhs, const SharedBoolean& rhs) {
  return (lhs <= rhs.get());
}

/**
 * @brief Overloads the greater-than operator (`>`) for a boolean
 * left-hand side operand.
 *
 * @details Compares a boolean to a SharedBoolean's value.
 *
 * @param lhs The boolean value on the left-hand side of the operator.
 * @param rhs The SharedBoolean object on the right-hand side.
 * @return `true` if the boolean is greater than the SharedBoolean's
 * value, `false` otherwise.
 */
bool operator>(const bool& lhs, const SharedBoolean& rhs) {
  return (lhs > rhs.get());
}

/**
 * @brief Overloads the greater-than-or-equal-to operator (`>=`) for a
 * boolean left-hand side operand.
 *
 * @details Compares a boolean to a SharedBoolean's value.
 *
 * @param lhs The boolean value on the left-hand side of the operator.
 * @param rhs The SharedBoolean object on the right-hand side.
 * @return `true` if the boolean is greater than or equal to the
 * SharedBoolean's value, `false` otherwise.
 */
bool operator>=(const bool& lhs, const SharedBoolean& rhs) {
  return (lhs >= rhs.get());
}

/**
 * @typedef fims_int
 * @brief An alias for the SharedInt class.
 *
 * @details This alias is used to conform to the fims naming convention,
 * representing a reference-counted integer.
 */
typedef SharedInt fims_int;
/**
 * @typedef fims_double
 * @brief An alias for the SharedReal class.
 *
 * @details This alias is used to conform to the fims naming convention,
 * representing a reference-counted real number (integer).
 */
typedef SharedReal fims_double;
/**
 * @typedef fims_string
 * @brief An alias for the SharedString class.
 *
 * @details This alias is used to conform to the fims naming convention,
 * representing a reference-counted string.
 */
typedef SharedString fims_string;
/**
 * @typedef fims_bool
 * @brief An alias for the SharedBoolean class.
 *
 * @details This alias is used to conform to the fims naming convention,
 * representing a reference-counted boolean.
 */
typedef SharedBoolean fims_bool;

#endif
