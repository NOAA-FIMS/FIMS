/*
* This File is part of the NOAA, National Marine Fisheries Service
* Fisheries Integrated Modeling System project. See LICENSE in the 
* source folder for reuse information.
*
*
* {{{ module_type }}} module file
* The purpose of this file is to include any .hpp files within the 
* subfolders so that only this file needs to included in the model.hpp file.
* 
* DEFINE guards for {{{ module_type }}} module outline to define the 
* module_type_base hpp file if not already defined.
*/
#ifndef POPULATION_DYNAMICS_{{{ module_type }}}_BASE_HPP
#define POPULATION_DYNAMICS_{{{ module_type }}}_BASE_HPP

#include "../../../common/model_object.hpp"

namespace fims {

/* @brief Base class for all {{{ module_type }}} functors.
 *
 * @tparam T The type of the {{{ module_type }}} functor.
 * */
template<typename T>
struct {{{ module_type }}}Base : public FIMSObject<T> {
    
    // id_g is the ID of the instance of the {{{ module_type }}}Base class.
    // this is like a memory tracker. 
    // Assigning each one its own ID is a way to keep track of
    // all the instances of the {{{ module_type }}}Base class.
    static uint32_t id_g;
    
    /* @brief Constructor.
    */
    {{{ module_type }}}Base() {
        this->id =  {{{ module_type }}}Base::id_g++;
    }

    /*
    * @brief Calculates the {{{ module_type }}} at the independent variable value.
    * @param x The independent variable in the logistic function (e.g., age or size in  {{{ module_type }}}).
    */
    virtual const T evaluate(const T& x) = 0;

};

template<typename T>
uint32_t {{{ module_type }}}Base<T>::id_g = 0;

}

#endif /* POPULATION_DYNAMICS_{{{ module_type }}}_BASE_HPP */