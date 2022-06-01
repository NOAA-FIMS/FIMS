/*
 * File:   information.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */

#ifndef FIMS_COMMON_INFORMATION_HPP
#define FIMS_COMMON_INFORMATION_HPP

#include <map>
#include <memory>
#include <vector>

#include "def.hpp"
// #include "../interface/interface.hpp"
#include "model_object.hpp"

namespace fims {

/**
 * Contains all objects and data pre-model construction
 */
template <typename T>
class information {
  public:
  static std::shared_ptr<information<T> > fims_information;
  // typedef fims::FIMSTraits<T>::variable_t variable_t;
  // typedef fims::FIMSTraits<T>::real_t real_t;
  std::vector<T*> parameters;  // list of all estimated parameters
  std::vector<T*>
      random_effects_parameters;  // list of all random effects parameters
  std::vector<T*>
      fixed_effects_parameters;  // list of all fixed effects parameters

static std::shared_ptr<information<T> > get_instance(){
   if(information<T>::fims_information == nullptr){
       information<T>::fims_information =  std::make_shared<fims::information<T> >();
   }  
   return information<T>::fims_information;

}
 bool creat_model() {
     std::cout<<"information::creat_model(): Not yet implemented.\n";
     return false;
 }
 
};

template<typename T>
std::shared_ptr<information<T> > information<T>::fims_information = nullptr;

}  // namespace fims

#endif /* FIMS_COMMON_INFORMATION_HPP */
