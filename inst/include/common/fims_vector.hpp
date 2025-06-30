/**
 * @file fims_vector.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_VECTOR_HPP
#define FIMS_VECTOR_HPP

#include "../interface/interface.hpp"
#include <ostream>
namespace fims {

/**
 * Wrapper class for std::vector types. If this file is compiled with
 * -DTMB_MODEL, conversion operators are defined for TMB vector types.
 *
 * All std::vector functions are copied over from the std library. While some of
 * these may not be called explicitly in FIMS, they may be required to run other
 * std library functions.
 *
 */
template <typename Type>
class Vector {
  std::vector<Type> vec_m;

  /**
   * @brief friend comparison operator. Allows the operartor to see private
   * members of fims::Vector<Type>.
   */
  template <typename T>
  friend bool operator==(const fims::Vector<T>& lhs,
                         const fims::Vector<T>& rhs);

 public:
  // Member Types

  typedef
      typename std::vector<Type>::value_type value_type; /*!<Member type Type>*/
  typedef typename std::vector<Type>::allocator_type
      allocator_type; /*!<Allocator for type Type>*/
  typedef typename std::vector<Type>::size_type size_type; /*!<Size type>*/
  typedef typename std::vector<Type>::difference_type
      difference_type; /*!<Difference type>*/
  typedef typename std::vector<Type>::reference
      reference; /*!<Reference type &Type>*/
  typedef typename std::vector<Type>::const_reference
      const_reference; /*!<Constant eference type const &Type>*/
  typedef typename std::vector<Type>::pointer pointer; /*!<Pointer type Type*>*/
  typedef typename std::vector<Type>::const_pointer
      const_pointer; /*!<Constant ointer type const Type*>*/
  typedef typename std::vector<Type>::iterator iterator; /*!<Iterator>*/
  typedef typename std::vector<Type>::const_iterator
      const_iterator; /*!<Constant iterator>*/
  typedef typename std::vector<Type>::reverse_iterator
      reverse_iterator; /*!<Reverse iterator>*/
  typedef typename std::vector<Type>::const_reverse_iterator
      const_reverse_iterator; /*!<Constant reverse iterator>*/

  // Constructors

  /**
   * Default constructor.
   */
  Vector() {}

  /**
   * @brief Constructs a Vector of length "size" and sets the elements with the
   * value from input "value".
   */
  Vector(size_t size, const Type& value = Type()) {
    this->vec_m.resize(size, value);
  }

  /**
   * @brief Copy constructor.
   */
  Vector(const Vector<Type>& other) {
    this->vec_m.resize(other.size());
    for (size_t i = 0; i < this->vec_m.size(); i++) {
      this->vec_m[i] = other[i];
    }
  }

  /**
   * @brief Initialization constructor from std::vector<Type> type.
   */
  Vector(const std::vector<Type>& other) { this->vec_m = other; }

  // TMB specific constructor
#ifdef TMB_MODEL

  /**
   * @brief Initialization constructor from tmbutils::vector<Type> type.
   */
  Vector(const tmbutils::vector<Type>& other) {
    this->vec_m.resize(other.size());
    for (size_t i = 0; i < this->vec_m.size(); i++) {
      this->vec_m[i] = other[i];
    }
  }

#endif

  /**
   * The following are std::vector functions copied over from the standard
   * library. While some of these may not be called explicitly in FIMS, they may
   * be required to run other std library functions.
   */

  /**
   * @brief Returns a reference to the element at specified location pos. No
   * bounds checking is performed.
   */
  inline Type& operator[](size_t pos) { return this->vec_m[pos]; }

  /**
   * @brief Returns a constant  reference to the element at specified location
   * pos. No bounds checking is performed.
   */
  inline const Type& operator[](size_t n) const { return this->vec_m[n]; }

  /**
   * @brief Returns a reference to the element at specified location pos. Bounds
   * checking is performed.
   */
  inline Type& at(size_t n) { return this->vec_m.at(n); }

  /**
   * @brief Returns a constant reference to the element at specified location
   * pos. Bounds checking is performed.
   */
  inline const Type& at(size_t n) const { return this->vec_m.at(n); }

  /**
   *  @brief  If this vector is size 1 and pos is greater than zero,
   * the first index is returned. If this vector has size
   * greater than 1 and pos is greater than size, a invalid_argument
   * exception is thrown. Otherwise, the value at index pos is returned.
   *
   * @param pos
   * @return a constant reference to the element at specified location
   */
  inline Type& get_force_scalar(size_t pos) {
    if (this->size() == 1 && pos > 0) {
      return this->at(0);
    } else if (this->size() > 1 && pos >= this->size()) {
      throw std::invalid_argument(
          "force_get fims::Vector index out of bounds.");
    } else {
      return this->at(pos);
    }
  }

  /**
   * @brief  Returns a reference to the first element in the container.
   */
  inline reference front() { return this->vec_m.front(); }

  /**
   * @brief  Returns a constant reference to the first element in the container.
   */
  inline const_reference front() const { return this->vec_m.front(); }

  /**
   * @brief  Returns a reference to the last element in the container.
   */
  inline reference back() { return this->vec_m.back(); }

  /**
   * @brief  Returns a constant reference to the last element in the container.
   */
  inline const_reference back() const { return this->vec_m.back(); }

  /**
   * @brief Returns a pointer to the underlying data array.
   */
  inline pointer data() { return this->vec_m.data(); }

  /**
   * @brief Returns a constant pointer to the underlying data array.
   */
  inline const_pointer data() const { return this->vec_m.data(); }

  // iterators

  /**
   * @brief Returns an iterator to the first element of the vector.
   */
  inline iterator begin() { return this->vec_m.begin(); }

  /**
   * @brief Returns an iterator to the element following the last element of the
   * vector.
   */
  inline iterator end() { return this->vec_m.end(); }

  /**
   * @brief Returns a reverse iterator to the first element of the reversed
   * vector. It corresponds to the last element of the non-reversed vector.
   */
  inline reverse_iterator rbegin() { return this->vec_m.rbegin(); }

  /**
   * @brief Returns a reverse iterator to the element following the last element
   * of the reversed vector. It corresponds to the element preceding the first
   * element of the non-reversed vector.
   */
  inline reverse_iterator rend() { return this->vec_m.rend(); }

  /**
   * @brief Returns a constant reverse iterator to the first element of the
   * reversed vector. It corresponds to the last element of the non-reversed
   * vector.
   */
  inline const_reverse_iterator rbegin() const { return this->vec_m.rbegin(); }

  /**
   * @brief Returns a constant reverse iterator to the element following the
   * last element of the reversed vector. It corresponds to the element
   * preceding the first element of the non-reversed vector.
   */
  inline const_reverse_iterator rend() const { return this->vec_m.rend(); }

  // capacity

  /**
   * @brief Checks whether the container is empty.
   */
  inline bool empty() { return this->vec_m.empty(); }

  /**
   * @brief Returns the number of elements.
   */
  inline size_type size() const { return this->vec_m.size(); }

  /**
   * @brief Returns the maximum possible number of elements.
   */
  inline size_type max_size() const { return this->vec_m.max_size(); }

  /**
   * @brief Reserves storage.
   */
  inline void reserve(size_type cap) { this->vec_m.reserve(cap); }

  /**
   * @brief Returns the number of elements that can be held in currently
   * allocated storage.
   */
  inline size_type capacity() { return this->vec_m.capacity(); }

  /**
   *  @brief Reduces memory usage by freeing unused memory.
   */
  inline void shrink_to_fit() { this->vec_m.shrink_to_fit(); }

  // modifiers

  /**
   * @brief Clears the contents.
   */
  inline void clear() { this->vec_m.clear(); }

  /**
   * @brief Inserts value before pos.
   */
  inline iterator insert(const_iterator pos, const Type& value) {
    return this->vec_m.insert(pos, value);
  }

  /**
   * @brief Inserts count copies of the value before pos.
   */
  inline iterator insert(const_iterator pos, size_type count,
                         const Type& value) {
    return this->vec_m.insert(pos, count, value);
  }

  /**
   * @brief Inserts elements from range [first, last) before pos.
   */
  template <class InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    return this->vec_m.insert(pos, first, last);
  }

  /**
   * @brief Inserts elements from initializer list ilist before pos.
   */

  iterator insert(const_iterator pos, std::initializer_list<Type> ilist) {
    return this->vec_m.insert(pos, ilist);
  }

  /**
   * @brief Constructs element in-place.
   */
  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return this->vec_m.emplace(pos, std::forward<Args>(args)...);
  }

  /**
   * @brief Removes the element at pos.
   */
  inline iterator erase(iterator pos) { return this->vec_m.erase(pos); }

  /**
   * @brief Removes the elements in the range [first, last).
   */
  inline iterator erase(iterator first, iterator last) {
    return this->vec_m.erase(first, last);
  }

  /**
   * @brief Adds an element to the end.
   */
  inline void push_back(const Type&& value) { this->vec_m.push_back(value); }

  /**
   * @brief Constructs an element in-place at the end.
   */
  template <class... Args>
  void emplace_back(Args&&... args) {
    this->vec_m.emplace_back(std::forward<Args>(args)...);
  }

  /**
   * @brief Removes the last element.
   */
  inline void pop_back() { this->vec_m.pop_back(); }

  /**
   * @brief Changes the number of elements stored.
   */
  inline void resize(size_t s) { this->vec_m.resize(s); }

  /**
   * @brief Swaps the contents.
   */
  inline void swap(Vector& other) { this->vec_m.swap(other.vec_m); }

  // end std::vector functions

  /**
   * Conversion operators
   */

  /**
   * @brief Converts fims::Vector<Type> to std::vector<Type>
   */
  inline operator std::vector<Type>() { return this->vec_m; }

#ifdef TMB_MODEL

  /**
   * @brief Converts fims::Vector<Type> to tmbutils::vector<Type>const
   */
  operator tmbutils::vector<Type>() const {
    tmbutils::vector<Type> ret;
    ret.resize(this->vec_m.size());
    for (size_t i = 0; i < this->vec_m.size(); i++) {
      ret[i] = this->vec_m[i];
    }
    return ret;
  }

  /**
   * @brief Converts fims::Vector<Type> to tmbutils::vector<Type>
   */
  operator tmbutils::vector<Type>() {
    tmbutils::vector<Type> ret;
    ret.resize(this->vec_m.size());
    for (size_t i = 0; i < this->vec_m.size(); i++) {
      ret[i] = this->vec_m[i];
    }
    return ret;
  }

#endif

 private:
};  // end fims::Vector class

/**
 * @brief Comparison operator.
 */
template <class T>
bool operator==(const fims::Vector<T>& lhs, const fims::Vector<T>& rhs) {
  return lhs.vec_m == rhs.vec_m;
}

}  // namespace fims

/**
 * @brief Output for std::ostream& for a vector.
 *
 * @param out The stream.
 * @param v A vector.
 * @return std::ostream&
 */
template <typename Type>
std::ostream& operator<<(std::ostream& out, fims::Vector<Type>& v) {
  out << "[";

  if (v.size() == 0) {
    out << "]";
    return out;
  }
  for (size_t i = 0; i < v.size() - 1; i++) {
    out << v[i] << ",";
  }

  out << v[v.size() - 1] << "]";
  return out;
}

#endif
