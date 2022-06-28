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
#include "../population_dynamics/growth/growth.hpp"
#include "../population_dynamics/recruitment/recruitment.hpp"
#include "../population_dynamics/selectivity/selectivity.hpp"
#include "model_object.hpp"

namespace fims {

/**
 * Contains all objects and data pre-model construction
 */
template <typename T>
class Information {
 public:
  static std::shared_ptr<Information<T> > fims_information;
  std::vector<T*> parameters;  // list of all estimated parameters
  std::vector<T*>
      random_effects_parameters;  // list of all random effects parameters
  std::vector<T*>
      fixed_effects_parameters;  // list of all fixed effects parameters

  std::map<uint32_t, std::shared_ptr<fims::RecruitmentBase<T> > >
      recruitment_models;
  std::map<uint32_t, std::shared_ptr<fims::SelectivityBase<T> > >
      selectivity_models;
  std::map<uint32_t, std::shared_ptr<fims::GrowthBase<T> > > growth_models;

  /**
   * Returns a single Information object for type T.
   *
   * @return singleton for type T
   */
  static std::shared_ptr<Information<T> > GetInstance() {
    if (Information<T>::fims_information == nullptr) {
      Information<T>::fims_information =
          std::make_shared<fims::Information<T> >();
    }
    return Information<T>::fims_information;
  }

  /**
   * Register a parameter as estimable.
   *
   * @param p
   */
  void RegisterParameter(T& p) { this->parameters.push_back(&p); }

  /**
   * Register a random effect as estimable.
   *
   * @param p
   */
  void RegisterRandomEffect(T& re) {
    this->random_effects_parameters.push_back(&re);
  }

  /**
   * Create the generalized stock assessment model that will evaluate the
   * objective function.
   *
   * @return
   */
  bool CreateModel() {
    std::cout << "Information::CreateModel(): Not yet implemented.\n";
    return false;
  }
};

template <typename T>
std::shared_ptr<Information<T> > Information<T>::fims_information =
    nullptr;  // singleton instance

}  // namespace fims

#endif /* FIMS_COMMON_INFORMATION_HPP */
