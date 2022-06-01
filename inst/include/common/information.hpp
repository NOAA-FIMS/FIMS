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
class Information {
  public:
  static std::shared_ptr<Information<T> > fims_information;
  // typedef fims::FIMSTraits<T>::variable_t variable_t;
  // typedef fims::FIMSTraits<T>::real_t real_t;
  std::vector<T*> parameters;  // list of all estimated parameters
  std::vector<T*>
      random_effects_parameters;  // list of all random effects parameters
  std::vector<T*>
      fixed_effects_parameters;  // list of all fixed effects parameters
  
  std::map<uint32_t, std::shared_ptr<fims::RecruitmentBase<T> > > recruitment_models;

static std::shared_ptr<Information<T> > get_instance(){
   if(Information<T>::fims_information == nullptr){
       Information<T>::fims_information =  std::make_shared<fims::Information<T> >();
   }  
   return Information<T>::fims_information;

}

void register_parameter( T& p){
    this->parameters.push_back(&p);
}

void register_random_effect( T& re){
    this->random_effects_parameters.push_back(&re);
}

 bool creat_model() {
     std::cout<<"Information::creat_model(): Not yet implemented.\n";
     return false;
 }
 
};

template<typename T>
std::shared_ptr<Information<T> > Information<T>::fims_information = nullptr;

}  // namespace fims

#endif /* FIMS_COMMON_INFORMATION_HPP */
