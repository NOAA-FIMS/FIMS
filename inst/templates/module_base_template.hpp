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
* Change {{{ module_type }}} to ALL CAPS in the define guards below
*/
#ifndef POPULATION_DYNAMICS_{{{ module_type }}}_BASE_HPP
#define POPULATION_DYNAMICS_{{{ module_type }}}_BASE_HPP

#include "../../../common/model_object.hpp"

namespace fims {

/* @brief Base class for all {{{ module_type }}} functors.
 *
 * @tparam Type The type of the {{{ module_type }}} functor.
 * */
template<typename Type>
// Change {{{ module_type }}} to CamelCase in the class definition below. 
struct {{{ module_type }}}Base : public FIMSObject<Type> {
    
    // id_g is the ID of the instance of the {{{ module_type }}}Base class.
    // this is like a memory tracker. 
    // Assigning each one its own ID is a way to keep track of
    // all the instances of the {{{ module_type }}}Base class.
    static uint32_t id_g;
    
    /* @brief Constructor.
    */
    // CamelCase {{{ module_type }}} below
    {{{ module_type }}}Base() {
        this->id =  {{{ module_type }}}Base::id_g++;
    }

    /*
    * @brief Calculates the {{{ module_type }}} at the independent variable value.
    * @param x The independent variable in the logistic function (e.g., age or size in  {{{ module_type }}}).
    */
    virtual const Type evaluate(const Type& x) = 0;

};

template<typename Type>
// {{{module_type}}} should be CamelCase below
uint32_t {{{ module_type }}}Base<Type>::id_g = 0;

}

// Change {{{module_type}}} to ALL CAPS below.
#endif /* POPULATION_DYNAMICS_{{{ module_type }}}_BASE_HPP */