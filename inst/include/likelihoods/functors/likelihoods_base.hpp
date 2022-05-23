/*
  * This File is part of the NOAA, National Marine Fisheries Service
* Fisheries Integrated Modeling System project. See LICENSE in the
* source folder for reuse information.
*
  *
  * likelihoods_base file
* The purpose of this file is to include any .hpp files within the
* subfolders so that only this file needs to included in the model.hpp file.
*
  * DEFINE guards for module_name module outline to define the
* module_name_base hpp file if not already defined.
*/
  #ifndef LIKELIHOODS_BASE_HPP
  #define LIKELIHOODS_BASE_HPP

  #include "../../common/model_object.hpp"

  namespace fims {

    /* @brief Base class for all module_name functors.
    *
      * @tparam T The type of the module_name functor.
    * */
      template<typename T>
      struct LikelihoodsBase : public FIMSObject<T> {

        // id_g is the ID of the instance of the LikelihoodsBase class.
        // this is like a memory tracker.
        // Assigning each one its own ID is a way to keep track of
        // all the instances of the LikelihoodsBase class.
        static uint32_t id_g;

        /* @brief Constructor.
        */
          LikelihoodsBase() {
            this->id = LikelihoodsBase::id_g++;
          }

        /*
          * @brief Calculates the likelihoods at the independent variable value.
        * @param x The observed value.
        */
          virtual const T evaluate(const T& observed, const T& expected, const bool& do_log) = 0;
      };

    template<typename T>
      uint32_t LikelihoodsBase<T>::id_g = 0;

  }

#endif /* LIKELIHOODS_BASE_HPP */
