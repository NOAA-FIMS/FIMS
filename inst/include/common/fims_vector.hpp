#ifndef FIMS_VECTOR_HPP
#define FIMS_VECTOR_HPP

#include "../interface/interface.hpp"

namespace  fims {


/**
 * Wrapper class for vector types.
 */
template<typename Type>
class Vector{
    std::vector<Type> vec_m;
public:
    //Member Types
    typedef typename std::vector<Type>::value_type value_type;
    typedef typename std::vector<Type>::allocator_type allocator_type;
    typedef typename std::vector<Type>::size_type size_type;
    typedef typename std::vector<Type>::difference_type difference_type;
    typedef typename std::vector<Type>::reference reference;
    typedef typename std::vector<Type>::const_reference const_reference;
    typedef typename std::vector<Type>::pointer pointer;
    typedef typename std::vector<Type>::const_pointer const_pointer;
    typedef typename std::vector<Type>::iterator iterator;
    typedef typename std::vector<Type>::const_iterator const_iterator;
    typedef typename std::vector<Type>::reverse_iterator reverse_iterator;
    typedef typename std::vector<Type>::const_reverse_iterator const_reverse_iterator;
    
    
    Vector(){
    }
    
    Vector(size_t size){
        this->vec_m.resize(size);
    }
    
    Vector(const Vector<Type>& other){
        this->vec_m.resize(other.size());
        for(int i =0; i < this->vec_m.size(); i++){
            this->vec_m[i] = other[i];
        }
    }
    
    inline Type& operator[](size_t n){
        return this->vec_m[n];
    }
    
    inline const Type& operator[](size_t n) const{
        return this->vec_m[n];
    }
    
    inline Type& at(size_t n) {
        return this->vec_m.at(n);
    }
    
    inline const Type& at(size_t n) const{
        return this->vec_m.at(n);
    }
    
    reference front() {
        return this->vec_m.front();
    }
    
    const_reference front() const {
        return this->vec_m.front();
    }
    
    reference back() {
        return this->vec_m.back();
    }
    
    const_reference back() const {
        return this->vec_m.back();
    }
    
    pointer data() {
        return this->vec_m.data();
    }
    
    const_pointer data() const {
        return this->vec_m.data();
    }
    
    //iterators
    iterator begin() {
        return this->vec_m.begin();
    }
    
    iterator end() {
        return this->vec_m.end();
    }
    
    const_iterator begin() const {
        return this->vec_m.begin();
    }
    
    const_iterator end() const {
        return this->vec_m.end();
    }
    
    reverse_iterator rbegin() {
        return this->vec_m.rbegin();
    }
    
    reverse_iterator rend() {
        return this->vec_m.rend();
    }
    
    const_reverse_iterator rbegin() const {
        return this->vec_m.rbegin();
    }
    
    const_reverse_iterator rend() const {
        return this->vec_m.rend();
    }
    
    //capacity
    
    bool empty() {
        return this->vec_m.empty();
    }
    
    size_type size() const {
        return this->vec_m.size();
    }
    
    size_type max_size() const {
        return this->vec_m.max_size();
    }
    
    void reserve(size_type cap) {
        this->vec_m.reserve(cap);
    }
    
    size_type capacity() {
        return this->vec_m.capacity();
    }
    
    void shrink_to_fit() {
        this->vec_m.shrink_to_fit();
    }
    
    //modifiers
    
    void clear() {
        this->vec_m.clear();
    }
    
    iterator insert(const_iterator pos, const Type& value) {
        return this->vec_m.insert(pos, value);
    }
    
    iterator insert(const_iterator pos, size_type count, const Type& value) {
        return this->vec_m.insert(pos, count, value);
    }
    
    template< class InputIt >
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        return this->vec_m.insert(pos, first, last);
    }
    
    iterator insert(const_iterator pos, std::initializer_list<Type> ilist) {
        return this->vec_m.insert(pos, ilist);
    }
    
    template< class... Args >
    iterator emplace(const_iterator pos, Args&&... args) {
        return this->vec_m.emplace(pos, std::forward<Args>(args)...);
    }
    
    iterator erase(iterator pos) {
        return this->vec_m.erase(pos);
    }
    
    iterator erase(iterator first, iterator last) {
        return this->vec_m.erase(first, last);
    }
    
    void push_back(const Type& value) {
        this->vec_m.push_back(value);
    }
    
    void push_back(const Type&& value) {
        this->vec_m.push_back(value);
    }
    
    template< class... Args >
    void emplace_back(Args&&... args) {
        this->vec_m.emplace_back(std::forward<Args>(args)...);
    }
    
    void pop_back() {
        this->vec_m.pop_back();
    }
    
    void resize(size_t s) {
        this->vec_m.resize(s);
    }
    
    void swap( Vector& other ){
        this->vec_m.swap(other.vec_m);
    }
    
    //conversion operatrors
    
    inline operator std::vector<Type>(){
        return this->vec_m;
    }
    
#ifdef TMB_MODEL
    
    inline operator CppAD::vector<Type>() const{
        return this->get_cppad_vector();
    }
    
    
    inline operator tmbutils::vector<Type>()const{
        return this->get_tmb_vector();
    }
    
private:
    
    
    CppAD::vector<Type> get_cppad_vector() const{
        CppAD::vector<Type> ret;
        ret.resize(this->vec_m.size());
        for(int i =0; i < this->vec_m.size(); i++){
            ret[i] = this->vec_m[i];
        }
        return ret;
    }
    
    tmbutils::vector<Type> get_tmb_vector() const{
        tmbutils::vector<Type> ret;
        ret.resize(this->vec_m.size());
        for(int i =0; i < this->vec_m.size(); i++){
            ret[i] = this->vec_m[i];
        }
        return ret;
    }
    
    
#endif
    
    
};// end fims::Vector class


} //end fims namespace

#endif
