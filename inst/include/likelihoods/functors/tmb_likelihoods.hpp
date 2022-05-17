/*
  * This File is part of the NOAA, National Marine Fisheries Service
* Fisheries Integrated Modeling System project.
* Refer to the LICENSE file for reuse information.
*
  * The purpose of this file is to declare the module_name functor class
* which is the base class for all module_name functors.
*/
  #ifndef LIKELIHOODS_TMB_LIKELIHOODS_HPP
  #define LIKELIHOODS_TMB_LIKELIHOODS_HPP


  //#include "../../../interface/interface.hpp"
  #include "../../../common/fims_math.hpp"
  #include "likelihoods_base.hpp"

  namespace fims {

    /**
      *  @brief sub_modulemodule_name class that returns the sub_module function value
    * from fims_math.
    */
      template<typename T>
      struct sub_modulemodule_name : public module_nameBase<T> {
        //add submodule class members here
        //these include parameters of the submodule

        sub_modulemodule_name(): module_nameBase<T>(){

        }

        /**
          * @brief
        *
          * @param x  description
        */
          virtual const T evaluate(const T& x) {
            //you will need to add class members as arguments to the function below
            return fims::sub_module<T>(x);
          }
      };

  }

#endif /* LIKELIHOODS_TMB_LIKELIHOODS_HPP */
