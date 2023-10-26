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
    typedef typename std::vector<Type>::iterator iterator;
    typedef Eigen::Array<Type, -1,1,0> eigen_vector;

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
    
    iterator begin(){
        return this->vec_m.begin();
    }
    
    iterator end(){
        return this->vec_m.end();
    }
    
    std::vector<Type>& get_data(){
        return this->vec_m;
    }
    
    operator std::vector<Type>(){
        return this->vec_m;
    }
    
#ifdef TMB_MODEL
    
    template<typename T>
    operator CppAD::vector<T>() const{
        CppAD::vector<T> ret;
        ret.resize(this->vec_m.size());
        for(int i =0; i < this->vec_m.size(); i++){
            ret[i] = this->vec_m[i];
        }
        return ret;
    }
    

    inline operator tmbutils::vector<Type>()const{
//        tmbutils::vector<Type> ret;
//        ret.resize(this->vec_m.size());
//        for(int i =0; i < this->vec_m.size(); i++){
//            ret[i] = this->vec_m[i];
//        }
//        return ret;
        return this->get_tmb_vector();
    }
    
    tmbutils::vector<Type> get_tmb_vector() const{
        tmbutils::vector<Type> ret;
        ret.resize(this->vec_m.size());
        for(int i =0; i < this->vec_m.size(); i++){
            ret[i] = this->vec_m[i];
        }
        return ret;
    }
    
    
//    operator eigen_vector(){
//        eigen_vector ret;
//        ret.resize(this->vec_m.size());
//        for(int i =0; i < this->vec_m.size(); i++){
//            ret[i] = this->vec_m[i];
//        }
//        return ret;
//    }
    
#endif
    
    size_t size() const{
        return this->vec_m.size();
    }
    
    void resize(size_t s){
        this->vec_m.resize(s);
    }
    
};


}

#endif
