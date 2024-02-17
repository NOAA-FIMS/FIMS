/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the module_name functor class
 * which is the base class for all module_name functors.
 */
 // Change test_module_type and test_module_name to ALL CAPS below
#ifndef POPULATION_DYNAMICS_test_module_type_test_module_name_HPP
#define POPULATION_DYNAMICS_test_module_type_test_module_name_HPP


//#include "../../../interface/interface.hpp"
// The fims_math.hpp file is only needed if you are using a function 
// programmed in fims_math
// Leave test_module_type lowercase below
#include "../../../common/fims_math.hpp"
#include "test_module_type_base.hpp"

namespace fims_popdy {

 /**
  *  @brief test_module_nametest_module_type class that returns the sub_module function value
 * from fims_math.
 */ 
 // test_module_name and test_module_type should be CamelCase below
template<typename Type>
struct test_module_nametest_module_type : public test_module_typeBase<Type> {
    //add submodule class members here
    //these include parameters of the submodule

    test_module_nametest_module_type(): test_module_typeBase<Type>(){

    }

    /**
    * @brief 
    * 
    * @param x  description
    */
    virtual const Type evaluate(const Type& x) {
        //you will need to add class members as arguments to the function below
        return fims_popdy::test_module_name<Type>(x);
    }
};

}
//Change test_module_type and test_module_name to ALL CAPS below
#endif /* POPULATION_DYNAMICS_test_module_type_test_module_name_HPP */
