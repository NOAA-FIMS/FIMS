/*
 * This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the module_name functor class
 * which is the base class for all module_name functors.
 */
 // Change {{{module_type}}} and {{{module_name}}} to ALL CAPS below
#ifndef POPULATION_DYNAMICS_{{{module_type}}}_{{{module_name}}}_HPP
#define POPULATION_DYNAMICS_{{{module_type}}}_{{{module_name}}}_HPP


//#include "../../../interface/interface.hpp"
// The fims_math.hpp file is only needed if you are using a function 
// programmed in fims_math
// Leave {{{module_type}}} lowercase below
#include "../../../common/fims_math.hpp"
#include "{{{module_type}}}_base.hpp"

namespace fims_popdy {

 /**
  *  @brief {{{module_name}}}{{{module_type}}} class that returns the sub_module function value
 * from fims_math.
 */ 
 // {{{module_name}}} and {{{module_type}}} should be CamelCase below
template<typename Type>
struct {{{module_name}}}{{{module_type}}} : public {{{module_type}}}Base<Type> {
    //add submodule class members here
    //these include parameters of the submodule

    {{{module_name}}}{{{module_type}}}(): {{{module_type}}}Base<Type>(){

    }

    /**
    * @brief 
    * 
    * @param x  description
    */
    virtual const Type evaluate(const Type& x) {
        //you will need to add class members as arguments to the function below
        return fims_popdy::{{{module_name}}}<Type>(x);
    }
};

}
//Change {{{module_type}}} and {{{module_name}}} to ALL CAPS below
#endif /* POPULATION_DYNAMICS_{{{module_type}}}_{{{module_name}}}_HPP */