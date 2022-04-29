/*
* This File is part of the NOAA, National Marine Fisheries Service
* Fisheries Integrated Modeling System project. See LICENSE in the 
* source folder for reuse information.
*
*
* {{{ module_name }}} module_base file
* The purpose of this file is to include any .hpp files within the 
* subfolders so that only this file needs to included in the model.hpp file.
* 
* DEFINE guards for {{{ module_name }}} module outline to define the 
* module_name_base hpp file if not already defined.
*/
#ifndef POPULATION_DYNAMICS_ {{{ module_name }}}_BASE_HPP
#define POPULATION_DYNAMICS_ {{{ module_name }}}_BASE_HPP

#include "../../../common/model_object.hpp"

namespace fims {

/* @brief Base class for all  {{{ module_name }}} functors.
 *
 * @tparam T The type of the  {{{ module_name }}} functor.
 * */
template<typename T>
struct  {{{ module_name }}}Base : public FIMSObject<T> {
    
    // id_g is the ID of the instance of the  {{{ module_name }}}Base class.
    // this is like a memory tracker. 
    // Assigning each one its own ID is a way to keep track of
    // all the instances of the  {{{ module_name }}}Base class.
    static uint32_t id_g;
    
    /* @brief Constructor.
    */
    {{{ module_name }}}Base() {
        this->id =  {{{ module_name }}}Base::id_g++;
    }

    /*
    * @brief Calculates the  {{{ module_name }}} at the independent variable value.
    * @param x The independent variable in the logistic function (e.g., age or size in  {{{ module_name }}}).
    */
    virtual const T evaluate(const T& x) = 0;

};

template<typename T>
uint32_t  {{{ module_name }}}Base<T>::id_g = 0;

}

#endif /* POPULATION_DYNAMICS_ {{{ module_name }}}_BASE_HPP */