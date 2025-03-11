#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_SHARED_PRIMITIVE_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_SHARED_PRIMITIVE_HPP

#include <memory>

/**
 * @brief Rcpp interface for Shared Int.
 *
 */
class SharedInt {
private:
    /**
     * @brief Shared pointer to value.
     *
     */
    std::shared_ptr<int> value;

public:

    /**
     * @brief Construct a new Shared Int object.
     *
     */
    SharedInt() : value(std::make_shared<int>(0)) {
    }
    /**
     * @brief Construct a new Shared Int object.
     *
     * @param val
     */
    SharedInt(int val) : value(std::make_shared<int>(val)) {
    }

    /**
     * @brief Copy construct a new Shared Int object.
     *
     * @param other
     */
    SharedInt(const SharedInt& other) : value(other.value) {
    }

    /**
     * @brief An assignment operator for Shared Int object.
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator=(const SharedInt& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }

    /**
     * @brief TODO: Document.
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator=(const int& other) {

        *value = other;

        return *this;
    }

    /**
     * @brief Construct a new Shared Int object. Moving the constructor.
     *
     * @param other
     */
    SharedInt(SharedInt&& other) noexcept : value(std::move(other.value)) {
    }

    /**
     * @brief Move the assignment operator for Shared Int.
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator=(SharedInt&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }

    // Access the value

    /**
     * @brief
     *
     * @return int
     */
    int get() const {
        return *value;
    }

    /**
     * @brief
     *
     * @param val
     */
    void set(int val) {
        *value = val;
    }

    /**
     * @brief
     *
     * @return int
     */
    operator int() {
        return this->get();
    }

    // Overloaded operators for SharedPrimitive

    /**
     * @brief
     *
     * @return int
     */
    int operator*() const {
        return *value;
    } // Dereference

    /**
     * @brief
     *
     * @return int*
     */
    int* operator->() {
        return value.get();
    } // Arrow operator

    /**
     * @brief
     *
     * @return const int*
     */
    const int* operator->() const {
        return value.get();
    } // Const arrow operator

    // Unary operators

    /**
     * @brief
     *
     * @return SharedInt&
     */
    SharedInt& operator++() {
        ++(*value);
        return *this;
    } // Prefix increment

    /**
     * @brief
     *
     * @return SharedInt
     */
    SharedInt operator++(int) {
        SharedInt temp(*this);
        ++(*value);
        return temp;
    } // Postfix increment

    /**
     * @brief
     *
     * @return SharedInt&
     */
    SharedInt& operator--() {
        --(*value);
        return *this;
    } // Prefix decrement

    /**
     * @brief
     *
     * @return SharedInt
     */
    SharedInt operator--(int) {
        SharedInt temp(*this);
        --(*value);
        return temp;
    } // Postfix decrement

    // Arithmetic operators with SharedPrimitive

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator+(const SharedInt& other) const {
        return SharedInt(*value + *other.value);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator-(const SharedInt& other) const {
        return SharedInt(*value - *other.value);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator*(const SharedInt& other) const {
        return SharedInt(*value * *other.value);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator/(const SharedInt& other) const {
        return SharedInt(*value / *other.value);
    }

    // Arithmetic operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator+(const int& other) const {
        return SharedInt(*value + other);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator-(const int& other) const {
        return SharedInt(*value - other);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator*(const int& other) const {
        return SharedInt(*value * other);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt
     */
    SharedInt operator/(const int& other) const {
        return SharedInt(*value / other);
    }

    // Compound assignment operators with SharedPrimitive

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator+=(const SharedInt& other) {
        *value += *other.value;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator-=(const SharedInt& other) {
        *value -= *other.value;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator*=(const SharedInt& other) {
        *value *= *other.value;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator/=(const SharedInt& other) {
        *value /= *other.value;
        return *this;
    }

    // Compound assignment operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator+=(const int& other) {
        *value += other;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator-=(const int& other) {
        *value -= other;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator*=(const int& other) {
        *value *= other;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedInt&
     */
    SharedInt& operator/=(const int& other) {
        *value /= other;
        return *this;
    }

    // Relational operators

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator==(const SharedInt& other) const {
        return *value == *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator!=(const SharedInt& other) const {
        return *value != *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<(const SharedInt& other) const {
        return *value < *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<=(const SharedInt& other) const {
        return *value <= *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>(const SharedInt& other) const {
        return *value > *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>=(const SharedInt& other) const {
        return *value >= *other.value;
    }

    // Relational operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator==(const int& other) const {
        return *value == other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator!=(const int& other) const {
        return *value != other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<(const int& other) const {
        return *value < other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<=(const int& other) const {
        return *value <= other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>(const int& other) const {
        return *value > other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>=(const int& other) const {
        return *value >= other;
    }

    /**
     * @brief
     *
     * @param os
     * @param sp
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, const SharedInt& sp) {
        os << *sp.value;
        return os;
    }
};


// Non-member operators for primitive + SharedPrimitive

SharedInt operator+(const int& lhs, const SharedInt& rhs) {
    return SharedInt(lhs + rhs.get());
}

SharedInt operator-(const int& lhs, const SharedInt& rhs) {
    return SharedInt(lhs - rhs.get());
}

SharedInt operator*(const int& lhs, const SharedInt& rhs) {
    return SharedInt(lhs * rhs.get());
}

SharedInt operator/(const int& lhs, const SharedInt& rhs) {
    return SharedInt(lhs / rhs.get());
}

bool operator<(const int& lhs, const SharedInt& rhs) {
    return (lhs < rhs.get());
}

bool operator<=(const int& lhs, const SharedInt& rhs) {
    return (lhs <= rhs.get());
}

bool operator>(const int& lhs, const SharedInt& rhs) {
    return (lhs > rhs.get());
}

bool operator>=(const int& lhs, const SharedInt& rhs) {
    return (lhs >= rhs.get());
}

/**
 * @brief TODO: Document.
 *
 */
class SharedReal {
private:
    std::shared_ptr<int> value;

public:
    // Constructor

    SharedReal() : value(std::make_shared<int>(0)) {
    }

    /**
     * @brief Construct a new Shared Real object
     *
     * @param val
     */
    SharedReal(int val) : value(std::make_shared<int>(val)) {
    }

    // Copy Constructor

    /**
     * @brief Construct a new Shared Real object
     *
     * @param other
     */
    SharedReal(const SharedReal& other) : value(other.value) {
    }

    // Assignment Operator

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator=(const SharedReal& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator=(const int& other) {

        *value = other;

        return *this;
    }

    // Move Constructor

    /**
     * @brief Construct a new Shared Real object
     *
     * @param other
     */
    SharedReal(SharedReal&& other) noexcept : value(std::move(other.value)) {
    }

    // Move Assignment Operator

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator=(SharedReal&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }

    // Access the value

    /**
     * @brief
     *
     * @return int
     */
    int get() const {
        return *value;
    }

    /**
     * @brief
     *
     * @param val
     */
    void set(int val) {
        *value = val;
    }

    /**
     * @brief
     *
     * @return int
     */
    operator int() {
        return this->get();
    }

    // Overloaded operators for SharedPrimitive

    /**
     * @brief
     *
     * @return int
     */
    int operator*() const {
        return *value;
    } // Dereference

    /**
     * @brief
     *
     * @return int*
     */
    int* operator->() {
        return value.get();
    } // Arrow operator

    /**
     * @brief
     *
     * @return const int*
     */
    const int* operator->() const {
        return value.get();
    } // Const arrow operator

    // Unary operators

    /**
     * @brief
     *
     * @return SharedReal&
     */
    SharedReal& operator++() {
        ++(*value);
        return *this;
    } // Prefix increment

    /**
     * @brief
     *
     * @return SharedReal
     */
    SharedReal operator++(int) {
        SharedReal temp(*this);
        ++(*value);
        return temp;
    } // Postfix increment

    /**
     * @brief
     *
     * @return SharedReal&
     */
    SharedReal& operator--() {
        --(*value);
        return *this;
    } // Prefix decrement

    /**
     * @brief
     *
     * @return SharedReal
     */
    SharedReal operator--(int) {
        SharedReal temp(*this);
        --(*value);
        return temp;
    } // Postfix decrement

    // Arithmetic operators with SharedPrimitive

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator+(const SharedReal& other) const {
        return SharedReal(*value + *other.value);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator-(const SharedReal& other) const {
        return SharedReal(*value - *other.value);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator*(const SharedReal& other) const {
        return SharedReal(*value * *other.value);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator/(const SharedReal& other) const {
        return SharedReal(*value / *other.value);
    }

    // Arithmetic operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator+(const int& other) const {
        return SharedReal(*value + other);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator-(const int& other) const {
        return SharedReal(*value - other);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator*(const int& other) const {
        return SharedReal(*value * other);
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal
     */
    SharedReal operator/(const int& other) const {
        return SharedReal(*value / other);
    }

    // Compound assignment operators with SharedPrimitive

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator+=(const SharedReal& other) {
        *value += *other.value;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator-=(const SharedReal& other) {
        *value -= *other.value;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator*=(const SharedReal& other) {
        *value *= *other.value;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator/=(const SharedReal& other) {
        *value /= *other.value;
        return *this;
    }

    // Compound assignment operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator+=(const int& other) {
        *value += other;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator-=(const int& other) {
        *value -= other;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator*=(const int& other) {
        *value *= other;
        return *this;
    }

    /**
     * @brief
     *
     * @param other
     * @return SharedReal&
     */
    SharedReal& operator/=(const int& other) {
        *value /= other;
        return *this;
    }

    // Relational operators

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator==(const SharedReal& other) const {
        return *value == *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator!=(const SharedReal& other) const {
        return *value != *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<(const SharedReal& other) const {
        return *value < *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<=(const SharedReal& other) const {
        return *value <= *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>(const SharedReal& other) const {
        return *value > *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>=(const SharedReal& other) const {
        return *value >= *other.value;
    }

    // Relational operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator==(const int& other) const {
        return *value == other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator!=(const int& other) const {
        return *value != other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<(const int& other) const {
        return *value < other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<=(const int& other) const {
        return *value <= other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>(const int& other) const {
        return *value > other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>=(const int& other) const {
        return *value >= other;
    }

    /**
     * @brief
     *
     * @param os
     * @param sp
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, const SharedReal& sp) {
        os << *sp.value;
        return os;
    }
};

SharedReal operator+(const double& lhs, const SharedReal& rhs) {
    return SharedReal(lhs + rhs.get());
}

SharedReal operator-(const double& lhs, const SharedReal& rhs) {
    return SharedReal(lhs - rhs.get());
}

SharedReal operator*(const double& lhs, const SharedReal& rhs) {
    return SharedReal(lhs * rhs.get());
}

SharedReal operator/(const double& lhs, const SharedReal& rhs) {
    return SharedReal(lhs / rhs.get());
}


/**
 * @brief TODO: Document.
 *
 */
class SharedString {
private:
    std::shared_ptr<std::string> value;

public:
    /**
     * @brief Construct a new Shared String object.
     *
     */
    SharedString() : value(std::make_shared<std::string>()) {
    }

    /**
     * @brief Construct a new Shared String object.
     *
     * @param val
     */
    SharedString(std::string val) : value(std::make_shared<std::string>(val)) {
    }

    /**
     * @brief Construct a new Shared String object. Copy constructor.
     *
     * @param other
     */
    SharedString(const SharedString& other) : value(other.value) {
    }

    /**
     * @brief Assignment operator.
     *
     * @param other
     * @return SharedString&
     */
    SharedString& operator=(const SharedString& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }

    /**
     * @brief TODO: Document.
     *
     * @param other
     * @return SharedString&
     */
    SharedString& operator=(const std::string& other) {

        *value = other;

        return *this;
    }

    /**
     * @brief Construct a new Shared String object. Move the constructor.
     *
     * @param other
     */
    SharedString(SharedString&& other) noexcept : value(std::move(other.value)) {
    }

    /**
     * @brief Move the assignment operator.
     *
     * @param other
     * @return SharedString&
     */
    SharedString& operator=(SharedString&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }

    /**
     * @brief Access the value.
     *
     * @return std::string
     */
    std::string get() const {
        return *value;
    }

    /**
     * @brief TODO: Document.
     *
     * @param val
     */
    void set(std::string val) {
        *value = val;
    }

    /**
     * @brief TODO: Document.
     *
     * @return std::string
     */
    operator std::string() {
        return this->get();
    }

    /**
     * @brief Overloaded operators for SharedPrimitive
     *
     * @return std::string
     */
    std::string operator*() const {
        return *value;
    } // Dereference

    /**
     * @brief  TODO: Document.
     *
     * @return std::string*
     */
    std::string* operator->() {
        return value.get();
    } // Arrow operator

    /**
     * @brief TODO: Document.
     *
     * @return const std::string*
     */
    const std::string* operator->() const {
        return value.get();
    } // Const arrow operator

  
    /**
     * @brief TODO: Document.
     *
     * @param os
     * @param sp
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, const SharedString& sp) {
        os << *sp.value;
        return os;
    }
};


/**
 * @brief TODO: Document.
 *
 */
class SharedBoolean {
private:
    std::shared_ptr<bool> value;

public:
    /**
     * @brief Construct a new Shared Boolean object.
     *
     */
    SharedBoolean() : value(std::make_shared<bool>(0)) {
    }

    /**
     * @brief Construct a new Shared Boolean object.
     *
     * @param val
     */
    SharedBoolean(bool val) : value(std::make_shared<bool>(val)) {
    }

    /**
     * @brief Construct a new Shared Boolean object. A copy constructor.
     *
     * @param other
     */
    SharedBoolean(const SharedBoolean& other) : value(other.value) {
    }

    /**
     * @brief Assignment operator.
     *
     * @param other
     * @return SharedBoolean&
     */
    SharedBoolean& operator=(const SharedBoolean& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }

    /**
     * @brief TODO: Document.
     *
     * @param other
     * @return SharedBoolean&
     */
    SharedBoolean& operator=(const bool& other) {

        *value = other;

        return *this;
    }

    /**
     * @brief Construct a new Shared Boolean object. Move constructor.
     *
     * @param other
     */
    SharedBoolean(SharedBoolean&& other) noexcept : value(std::move(other.value)) {
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other
     * @return SharedBoolean&
     */
    SharedBoolean& operator=(SharedBoolean&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }

    /**
     * @brief Access the value.
     *
     * @return true
     * @return false
     */
    bool get() const {
        return *value;
    }

    /**
     * @brief TODO: Document.
     *
     * @param val
     */
    void set(bool val) {
        *value = val;
    }

    /**
     * @brief TODO: Document.
     *
     * @return true
     * @return false
     */
    operator bool() {
        return this->get();
    }

    /**
     * @brief Overloaded operators for SharedPrimitive.
     *
     * @return true
     * @return false
     */
    bool operator*() const {
        return *value;
    } // Dereference

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool* operator->() {
        return value.get();
    } // Arrow operator

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    const bool* operator->() const {
        return value.get();
    } // Const arrow operator

   

    // Relational operators
    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator==(const SharedBoolean& other) const {
        return *value == *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator!=(const SharedBoolean& other) const {
        return *value != *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<(const SharedBoolean& other) const {
        return *value < *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<=(const SharedBoolean& other) const {
        return *value <= *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>(const SharedBoolean& other) const {
        return *value > *other.value;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>=(const SharedBoolean& other) const {
        return *value >= *other.value;
    }

    // Relational operators with primitives

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator==(const bool& other) const {
        return *value == other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator!=(const bool& other) const {
        return *value != other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<(const bool& other) const {
        return *value < other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator<=(const bool& other) const {
        return *value <= other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>(const bool& other) const {
        return *value > other;
    }

    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool operator>=(const bool& other) const {
        return *value >= other;
    }

    /**
     * @brief
     *
     * @param os
     * @param sp
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, const SharedBoolean& sp) {
        os << *sp.value;
        return os;
    }
};



bool operator<(const bool& lhs, const SharedBoolean& rhs) {
    return (lhs < rhs.get());
}

bool operator<=(const bool& lhs, const SharedBoolean& rhs) {
    return (lhs <= rhs.get());
}

bool operator>(const bool& lhs, const SharedBoolean& rhs) {
    return (lhs > rhs.get());
}

bool operator>=(const bool& lhs, const SharedBoolean& rhs) {
    return (lhs >= rhs.get());
}


typedef SharedInt fims_int;
typedef SharedReal fims_double;
typedef SharedString fims_string;
typedef SharedBoolean fims_bool;

#endif
