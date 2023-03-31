/*! \file information.hpp
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

#include "../distributions/distributions.hpp"
#include "../population_dynamics/fleet/fleet.hpp"
#include "../population_dynamics/growth/growth.hpp"
#include "../population_dynamics/population/population.hpp"
#include "../population_dynamics/recruitment/recruitment.hpp"
#include "../population_dynamics/selectivity/selectivity.hpp"
#include "def.hpp"
#include "model_object.hpp"

namespace fims {

/**
 * @brief Stores FIMS model information and creates model. Contains all objects
 * and data pre-model construction
 */
template <typename T>
class Information {
 public:
  size_t nyears;       /**< number of years >*/
  size_t nseasons = 1; /**< number of seasons >*/
  size_t nages;        /**< number of ages>*/

  static std::shared_ptr<Information<T> >
      fims_information;       /**< singleton instance >*/
  std::vector<T*> parameters; /**< list of all estimated parameters >*/
  std::vector<T*>
      random_effects_parameters; /**< list of all random effects parameters >*/
  std::vector<T*>
      fixed_effects_parameters; /**< list of all fixed effects parameters >*/
  std::vector<double> ages;     /**< ages in model >*/

  // data objects
  std::map<uint32_t, std::shared_ptr<fims::DataObject<T> > >
      data_objects; /*!< map that holds data objects >*/
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims::DataObject<T> > >::iterator
      data_iterator; /**< iterator for the data objects */

  // life history modules
  std::map<uint32_t, std::shared_ptr<fims::RecruitmentBase<T> > >
      recruitment_models; /*!<hash map to link each object to its shared
                             location in memory*/
  typedef
      typename std::map<uint32_t,
                        std::shared_ptr<fims::RecruitmentBase<T> > >::iterator
          recruitment_models_iterator;
  /**< iterator for recruitment objects>*/

  std::map<uint32_t, std::shared_ptr<fims::SelectivityBase<T> > >
      selectivity_models; /*!<hash map to link each object to its shared
                             location in memory*/
  typedef
      typename std::map<uint32_t,
                        std::shared_ptr<fims::SelectivityBase<T> > >::iterator
          selectivity_models_iterator;
  /**< iterator for selectivity objects>*/

  std::map<uint32_t, std::shared_ptr<fims::GrowthBase<T> > >
      growth_models; /*!<hash map to link each object to its shared location in
                        memory*/
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims::GrowthBase<T> > >::iterator
      growth_models_iterator;
  /**< iterator for growth objects>*/

  std::map<uint32_t, std::shared_ptr<fims::MaturityBase<T> > >
      maturity_models; /*!<hash map to link each object to its shared location
                          in memory*/
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims::MaturityBase<T> > >::iterator
      maturity_models_iterator;
  /**< iterator for maturity objects>*/

  // fleet modules
  std::map<uint32_t, std::shared_ptr<fims::Fleet<T> > >
      fleets; /*!<hash map to link each object to its shared location in
                 memory*/
  typedef
      typename std::map<uint32_t, std::shared_ptr<fims::Fleet<T> > >::iterator
          fleet_iterator;
  /**< iterator for fleet objects>*/

  // populations
  std::map<uint32_t, std::shared_ptr<fims::Population<T> > >
      populations; /*!<hash map to link each object to its shared location in
                      memory*/
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims::Population<T> > >::iterator
      population_iterator;
  /**< iterator for population objects>*/

  // distributions
  std::map<uint32_t, std::shared_ptr<fims::DistributionsBase<T> > >
      distribution_models; /*!<hash map to link each object to its shared
                              location in memory*/
  typedef
      typename std::map<uint32_t,
                        std::shared_ptr<fims::DistributionsBase<T> > >::iterator
          distribution_models_iterator;
  /**< iterator for distribution objects>*/

  Information() {}

  virtual ~Information() {}

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
  void RegisterParameter(T& p) { this->fixed_effects_parameters.push_back(&p); }

  /**
   * Register a random effect as estimable.
   *
   * @param re
   */
  void RegisterRandomEffect(T& re) {
    this->random_effects_parameters.push_back(&re);
  }

  /**
   * Create the generalized stock assessment model that will evaluate the
   * objective function. Does error checking to make sure the program has
   * all necessary components for the model and that they're in the right
   * dimensions.
   *
   * @return
   */
  bool CreateModel() {
    bool valid_model = true;

    std::cout << "Information: Initializing fleet objects.\n";
    for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end();
         ++it) {
      // Initialize fleet object
      std::shared_ptr<fims::Fleet<T> > f = (*it).second;

      f->Initialize(nyears, nages);

      // set index data
      if (f->observed_index_data_id != -999) {
        uint32_t index_id = static_cast<uint32_t>(f->observed_index_data_id);
        data_iterator it = this->data_objects.find(index_id);

        if (it != this->data_objects.end()) {
          f->observed_index_data = (*it).second;
        } else {
          valid_model = false;
          // log error
          FIMS_LOG << "Error: observed index data not defined for fleet"
                   << f->id << std::endl;
        }

      } else {
        valid_model = false;
        // log error
      }

      // set age composition data
      if (f->observed_agecomp_data_id != -999) {
        uint32_t agecomp_id =
            static_cast<uint32_t>(f->observed_agecomp_data_id);
        data_iterator it = this->data_objects.find(agecomp_id);

        if (it != this->data_objects.end()) {
          f->observed_agecomp_data = (*it).second;
        } else {
          valid_model = false;
          // log error
        }

      } else {
        valid_model = false;
        // log error
      }

      // set selectivity model
      if (f->selectivity_id != -999) {
        uint32_t sel_id = static_cast<uint32_t>(
            f->selectivity_id);  // cast as unsigned integer
        //   selectivity_models_iterator it = this->selectivity_models.find(
        //     sel_id);  // if find, set it, otherwise invalid

        std::cout << "Information: Initializing fleet objects.\n";
        for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end();
             ++it) {
          // Initialize fleet object
          std::shared_ptr<fims::Fleet<T> > f =
              (*it).second;  // fleet object pointer initialized to second field
                             // in map

          f->Initialize(nyears, nages);

          // set catch data
          // if (f->observed_catch_data_id != -999) {
          //   uint32_t catch_id =
          //       static_cast<uint32_t>(f->observed_catch_data_id);
          //   data_iterator it = this->data_objects.find(catch_id);
          //
          //   if (it != this->data_objects.end()) {
          //     f->observed_catch_data = (*it).second;
          //   } else {
          //     valid_model = false;
          //     // log error
          //     FIMS_LOG << "Error: observed catch data not defined for fleet"
          //              << f->id << std::endl;
          //   }
          //
          // } else {
          //   valid_model = false;
          //   // log error
          // }

          // set index data
          if (f->observed_index_data_id != -999) {
            uint32_t index_id =
                static_cast<uint32_t>(f->observed_index_data_id);
            data_iterator it = this->data_objects.find(index_id);

            if (it != this->data_objects.end()) {
              f->observed_index_data = (*it).second;
            } else {
              valid_model = false;
              // log error
              FIMS_LOG << "Error: observed index data not defined for fleet"
                       << f->id << std::endl;
            }

          } else {
            valid_model = false;
            // log error
          }

          // set age composition data
          if (f->observed_agecomp_data_id != -999) {
            uint32_t agecomp_id =
                static_cast<uint32_t>(f->observed_agecomp_data_id);
            data_iterator it = this->data_objects.find(agecomp_id);

            if (it != this->data_objects.end()) {
              f->observed_agecomp_data = (*it).second;
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }

          // set selectivity model
          if (f->selectivity_id != -999) {
            uint32_t sel_id = static_cast<uint32_t>(
                f->selectivity_id);  // cast as unsigned integer
            selectivity_models_iterator it = this->selectivity_models.find(
                sel_id);  // if find, set it, otherwise invalid

            if (it != this->selectivity_models.end()) {
              f->selectivity =
                  (*it).second;  // elements in container held in pair (first is
                                 // id, second is object - shared pointer to
                                 // distribution)
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }

          // set index likelihood
          if (f->index_likelihood_id != -999) {
            uint32_t ind_like_id = static_cast<uint32_t>(
                f->index_likelihood_id);  // cast as unsigned integer
            distribution_models_iterator it = this->distribution_models.find(
                ind_like_id);  // if find, set it, otherwise invalid

            if (it != this->distribution_models.end()) {
              f->index_likelihood =
                  (*it).second;  // elements in container held in pair (first is
                                 // id, second is object - shared pointer to
                                 // distribution)
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }

          // set catch likelihood
          // if (f->catch_likelihood_id != -999) {
          //  uint32_t catch_like_id = static_cast<uint32_t>(
          //      f->catch_likelihood_id);  // cast as unsigned integer
          //  distribution_models_iterator it = this->distribution_models.find(
          //      catch_like_id);  // if find, set it, otherwise invalid
          //
          //  if (it != this->distribution_models.end()) {
          //    f->catch_likelihood =
          //        (*it).second;  // elements in container held in pair (first
          //        is
          //                       // id, second is object - shared pointer to
          //                       // distribution)
          //  } else {
          //    valid_model = false;
          //    // log error
          //  }
          //
          //} else {
          //  valid_model = false;
          //  // log error
          //}

          // set agecomp likelihood
          if (f->agecomp_likelihood_id != -999) {
            uint32_t ac_like_id = static_cast<uint32_t>(
                f->agecomp_likelihood_id);  // cast as unsigned integer
            distribution_models_iterator it = this->distribution_models.find(
                ac_like_id);  // if find, set it, otherwise invalid

            if (it != this->distribution_models.end()) {
              f->agecomp_likelihood =
                  (*it).second;  // elements in container held in pair (first is
                                 // id, second is object - shared pointer to
                                 // distribution)
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }
        }

        std::cout << "Information: Initializing population objects.\n";
        for (population_iterator it = this->populations.begin();
             it != this->populations.end(); ++it) {
          std::shared_ptr<fims::Population<T> > p = (*it).second;
          // error check and set population elements here

          p->Initialize(nyears, nseasons, nages);

          // set recruitment
          if (p->recruitment_id != -999) {
            uint32_t recruitment_uint =
                static_cast<uint32_t>(p->recruitment_id);
            recruitment_models_iterator it =
                this->recruitment_models.find(recruitment_uint);

            if (it != this->recruitment_models.end()) {
              p->recruitment =
                  (*it).second;  // recruitment defined in population.hpp
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }
          // set growth
          if (p->growth_id != -999) {
            uint32_t growth_uint = static_cast<uint32_t>(p->growth_id);
            growth_models_iterator it = this->growth_models.find(
                growth_uint);  // growth_models is specified in information.hpp
                               // and used in rcpp
            p->ages =
                this->ages;  // check me re dims. ages defined as an std::vector
                             // at the head of information.hpp; are the
                             // dimensions of ages defined in rcpp or where?
            if (it != this->growth_models.end()) {
              p->growth =
                  (*it).second;  // growth defined in population.hpp (the object
                                 // is called p, growth is within p)
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }

          // set maturity
          if (p->maturity_id != -999) {
            uint32_t maturity_uint = static_cast<uint32_t>(p->maturity_id);
            maturity_models_iterator it = this->maturity_models.find(
                maturity_uint);  // >maturity_models is specified in
                                 // information.hpp and used in rcpp

            if (it != this->maturity_models.end()) {
              p->maturity =
                  (*it).second;  // >maturity defined in population.hpp
            } else {
              valid_model = false;
              // log error
            }

          } else {
            valid_model = false;
            // log error
          }

          // check me - add another fleet iterator to push information from
          // population to the individual fleets This is to pass catch at age
          // from population to fleets?
          for (fleet_iterator it = this->fleets.begin();
               it != this->fleets.end(); ++it) {
            // Initialize fleet object
            std::shared_ptr<fims::Fleet<T> > f =
                (*it).second;  // fleet object pointer initialized to second
                               // field in map
            // any shared member in p (population is pushed into fleets)
            p->fleets.push_back(f);
          }
        }
        return valid_model;

      } else {
        valid_model = false;
        // log error
      }

      // set agecomp likelihood
      if (f->agecomp_likelihood_id != -999) {
        uint32_t ac_like_id = static_cast<uint32_t>(
            f->agecomp_likelihood_id);  // cast as unsigned integer
        distribution_models_iterator it = this->distribution_models.find(
            ac_like_id);  // if find, set it, otherwise invalid

        if (it != this->distribution_models.end()) {
          f->agecomp_likelihood =
              (*it).second;  // elements in container held in pair (first is id,
                             // second is object - shared pointer to
                             // distribution)
        } else {
          valid_model = false;
          // log error
        }

      } else {
        valid_model = false;
        // log error
      }
    }

    std::cout << "Information: Initializing population objects.\n";
    for (population_iterator it = this->populations.begin();
         it != this->populations.end(); ++it) {
      std::shared_ptr<fims::Population<T> > p = (*it).second;
      // error check and set population elements here
    }
    return valid_model;
  }

  /**
   * @brief Get the Nages object
   *
   * @return size_t
   */
  size_t GetNages() const { return nages; }

  /**
   * @brief Set the Nages object
   *
   * @param nages
   */
  void SetNages(size_t nages) { this->nages = nages; }

  /**
   * @brief Get the Nseasons object
   *
   * @return size_t
   */
  size_t GetNseasons() const { return nseasons; }

  /**
   * @brief Set the Nseasons object
   *
   * @param nseasons
   */
  void SetNseasons(size_t nseasons) { this->nseasons = nseasons; }

  /**
   * @brief Get the Nyears object
   *
   * @return size_t
   */
  size_t GetNyears() const { return nyears; }

  /**
   * @brief Set the Nyears object
   *
   * @param nyears
   */
  void SetNyears(size_t nyears) { this->nyears = nyears; }

  /**
   * @brief Get the Parameters object
   *
   * @return std::vector<T*>&
   */
  std::vector<T*>& GetParameters() { return parameters; }

  /**
   * @brief Get the Fixed Effects Parameters object
   *
   * @return std::vector<T*>&
   */
  std::vector<T*>& GetFixedEffectsParameters() {
    return fixed_effects_parameters;
  }

  /**
   * @brief Get the Random Effects Parameters object
   *
   * @return std::vector<T*>&
   */
  std::vector<T*>& GetRandomEffectsParameters() {
    return random_effects_parameters;
  }
};

template <typename T>
std::shared_ptr<Information<T> > Information<T>::fims_information =
    nullptr;  // singleton instance

}  // namespace fims

#endif /* FIMS_COMMON_INFORMATION_HPP */
