/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the module_name functor class
 * which is the base class for all module_name functors.
 */
#ifndef POPULATION_DYNAMICS_{{{module_name}}}_{{{sub_module}}}_HPP
#define POPULATION_DYNAMICS_{{{module_name}}}_{{{sub_module}}}_HPP


//#include "../../../interface/interface.hpp"
// The fims_math.hpp file is only needed if you are using a function 
// programmed in fims_math
#include "../../../common/fims_math.hpp"
#include "{{{module_name}}}_base.hpp"

namespace fims {

 /**
  *  @brief {{{sub_module}}}{{{module_name}}} class that returns the sub_module function value
 * from fims_math.
 */ 
template<typename T>
struct {{{sub_module}}}{{{module_name}}} : public {{{module_name}}}Base<T> {
    //add submodule class members here
    //these include parameters of the submodule

    {{{sub_module}}}{{{module_name}}}(): {{{module_name}}}Base<T>(){

    }

    /**
    * @brief 
    * 
    * @param x  description
    */
    virtual const T evaluate(const T& x) {
        //you will need to add class members as arguments to the function below
        return fims::{{{sub_module}}}<T>(x);
    }
};

}

#endif /* POPULATION_DYNAMICS_{{{module_name}}}_{{{sub_module}}}_HPP */