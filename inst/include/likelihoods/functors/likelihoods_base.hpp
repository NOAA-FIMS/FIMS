/*! \file likelihoods_base.hpp
* This File is part of the NOAA, National Marine Fisheries Service
* Fisheries Integrated Modeling System project. See LICENSE in the
* source folder for reuse information.
*
*
* likelihoods_base file
* The purpose of this file is to declare the LikelihoodsBase class
* which is the base class for all likelihood functors.
*
* DEFINE guards for likelihoods module outline to define the
* likelihoods_base hpp file if not already defined.
*/
  #ifndef LIKELIHOODS_BASE_HPP
  #define LIKELIHOODS_BASE_HPP

  #include "../../common/model_object.hpp"
  #include "../../interface/interface.hpp"

  namespace fims {

    /** @brief Base class for all module_name functors.
     *
     * @tparam T The type of the module_name functor.
     *
     */
      template<typename T>
      struct LikelihoodsBase : public FIMSObject<T> {

        // id_g is the ID of the instance of the LikelihoodsBase class.
        // this is like a memory tracker.
        // Assigning each one its own ID is a way to keep track of
        // all the instances of the LikelihoodsBase class.
        static uint32_t id_g;

        /** @brief Constructor.
         */
          LikelihoodsBase() {
            this->id = LikelihoodsBase::id_g++;
          }

        /**
         * @brief Generic probability density function. Calculates the likelihood at the independent variable value.
         * @param do_log Boolean; if true, log-likelihoods are returned
         */
          virtual const T evaluate(const bool& do_log) = 0;
      };

    /** @brief Default id of the singleton likelihood class
     */
    template<typename T>
      uint32_t LikelihoodsBase<T>::id_g = 0;

  }

#endif /* LIKELIHOODS_BASE_HPP */
