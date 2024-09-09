/** \file information.hpp
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
#include "fims_vector.hpp"
#include "model_object.hpp"

namespace fims_info {

/**
 * @brief Stores FIMS model information and creates model. Contains all objects
 * and data pre-model construction
 */
template <typename Type>
class Information {
 public:
  size_t nyears;       /**< number of years >*/
  size_t nseasons = 1; /**< number of seasons >*/
  size_t nages;        /**< number of ages>*/

  static std::shared_ptr<Information<Type> >
      fims_information;          /**< singleton instance >*/
  std::vector<Type*> parameters; /**< list of all estimated parameters >*/
  std::vector<Type*>
      random_effects_parameters; /**< list of all random effects parameters >*/
  std::vector<Type*>
      fixed_effects_parameters; /**< list of all fixed effects parameters >*/
  std::vector<std::string>
      parameter_names; /**< list of all parameter names estimated in the model */

  // data objects
  std::map<uint32_t, std::shared_ptr<fims_data_object::DataObject<Type> > >
      data_objects; /**< map that holds data objects >*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_data_object::DataObject<Type> > >::iterator
      data_iterator; /**< iterator for the data objects */

  // life history modules
  std::map<uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type> > >
      recruitment_models; /**<hash map to link each object to its shared
                             location in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type> > >::iterator
      recruitment_models_iterator;
  /**< iterator for recruitment objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::SelectivityBase<Type> > >
      selectivity_models; /**<hash map to link each object to its shared
                             location in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::SelectivityBase<Type> > >::iterator
      selectivity_models_iterator;
  /**< iterator for selectivity objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::GrowthBase<Type> > >
      growth_models; /**<hash map to link each object to its shared location in
                        memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::GrowthBase<Type> > >::iterator
      growth_models_iterator;
  /**< iterator for growth objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::MaturityBase<Type> > >
      maturity_models; /**<hash map to link each object to its shared location
                          in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::MaturityBase<Type> > >::iterator
      maturity_models_iterator;
  /**< iterator for maturity objects>*/

  // fleet modules
  std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type> > >
      fleets; /**<hash map to link each object to its shared location in
                 memory*/
  typedef
      typename std::map<uint32_t,
                        std::shared_ptr<fims_popdy::Fleet<Type> > >::iterator
          fleet_iterator;
  /**< iterator for fleet objects>*/

  // populations
  std::map<uint32_t, std::shared_ptr<fims_popdy::Population<Type> > >
      populations; /**<hash map to link each object to its shared location in
                      memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::Population<Type> > >::iterator
      population_iterator;
  /**< iterator for population objects>*/

  // distributions
  std::map<uint32_t,
           std::shared_ptr<fims_distributions::DensityComponentBase<Type> > >
      density_components; /**<hash map to link each object to its shared
                              location in memory*/
  typedef typename std::map<
      uint32_t,
      std::shared_ptr<fims_distributions::DensityComponentBase<Type> > >::iterator
      density_components_iterator;
  /**< iterator for distribution objects>*/

  std::unordered_map<uint32_t, fims::Vector<Type>* >
      variable_map; /**<hash map to link a parameter, derived value, or observation
                        to its shared location in memory */
  typedef typename std::unordered_map<uint32_t, fims::Vector<Type>* >::iterator
      variable_map_iterator; /**< iterator for variable map>*/

  Information() {}

  virtual ~Information() {}

  /**
   * Returns a single Information object for type T.
   *
   * @return singleton for type T
   */
  static std::shared_ptr<Information<Type> > GetInstance() {
    if (Information<Type>::fims_information == nullptr) {
      Information<Type>::fims_information =
          std::make_shared<fims_info::Information<Type> >();
    }
    return Information<Type>::fims_information;
  }

  /**
   * Register a parameter as estimable.
   *
   * @param p
   */
  void RegisterParameter(Type& p) {
    this->fixed_effects_parameters.push_back(&p);
  }

  /**
   * Register a random effect as estimable.
   *
   * @param re
   */
  void RegisterRandomEffect(Type& re) {
    this->random_effects_parameters.push_back(&re);
  }

  /**
   * Register a parameter name.
   *
   * @param p_name
   */
  void RegisterParameterName(std::string p_name) {
    this->parameter_names.push_back(p_name);
  }

  /**
   * Loop over distributions and set links to distribution x value if distribution is a prior type.
   */
  void setup_priors(){
       FIMS_INFO_LOG("segment");
    for(density_components_iterator it = density_components.begin(); it!= density_components.end(); ++it){
      std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
      if(d->input_type == "prior"){
        INFO_LOG << "Setup prior for distribution " << d->id << std::endl;
        variable_map_iterator vmit;
        INFO_LOG << "Link prior from distribution " << d->id << " to parameter " << d->key[0] << std::endl;
        vmit = this->variable_map.find(d->key[0]);
        d->x = *(*vmit).second;
        for(size_t i=1; i<d->key.size(); i++){
          INFO_LOG << "Link prior from distribution " << d->id << " to parameter " << d->key[i] << std::endl;
          vmit = this->variable_map.find(d->key[i]);
          d->x.insert(std::end(d->x),
            std::begin(*(*vmit).second), std::end(*(*vmit).second));
        }
        INFO_LOG << "Prior size for distribution " << d->id << "is: " << d->x.size() << std::endl;
      }
    }
  }

  /**
   * Loop over distributions and set links to distribution x value if distribution is a random effects type.
   */
  void setup_random_effects(){
       FIMS_INFO_LOG("segment");
    for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
      std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
      if(d->input_type == "random_effects"){
        INFO_LOG << "Setup random effects for distribution " << d->id << std::endl;
        variable_map_iterator vmit;
        INFO_LOG << "Link random effects from distribution " << d->id << " to derived value " << d->key[0] << std::endl;
        vmit = this->variable_map.find(d->key[0]);
        d->x = *(*vmit).second;
        for(size_t i=1; i<d->key.size(); i++){
        INFO_LOG << "Link random effects from distribution " << d->id << " to derived value " << d->key[i] << std::endl;
          vmit = this->variable_map.find(d->key[i]);
          d->x.insert(std::end(d->x),
                                    std::begin(*(*vmit).second), std::end(*(*vmit).second));
        }
        INFO_LOG << "Random effect size for distribution " << d->id << "is: " << d->x.size() << std::endl;
      }
    }
  }

  /**
   * Loop over distributions and set links to distribution expected value if distribution is a data type.
   */
  void setup_data(){
       FIMS_INFO_LOG("segment");
    for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
      std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
      if(d->input_type == "data"){
        INFO_LOG << "Setup expected value for data distribution " << d->id << std::endl;
        variable_map_iterator vmit;
        INFO_LOG << "Link expected value from distribution " << d->id << " to derived value " << d->key[0] << std::endl;
        vmit = this->variable_map.find(d->key[0]);
        d->expected_values = *(*vmit).second;

        for(size_t i=1; i<d->key.size(); i++){
          vmit = this->variable_map.find(d->key[i]);
        INFO_LOG << "Link expected value from distribution " << d->id << " to derived value " << d->key[i] << std::endl;
          d->expected_values.insert(std::end(d->expected_values),
            std::begin(*(*vmit).second), std::end(*(*vmit).second));
        }
        INFO_LOG << "Expected value size for distribution " << d->id << "is: " << d->expected_values.size() << std::endl;
      }
    }
  }



  /**
   * Create the generalized stock assessment model that will evaluate the
   * objective function. Does error checking to make sure the program has
   * all necessary components for the model and that they're in the right
   * dimensions. This sets up pointers to all memory objects and initializes
   * fleet and population objects.
   *
   * @return True if valid model, False if invalid model, check fims.log for
   * errors.
   */
  bool CreateModel() {
    bool valid_model = true;
    INFO_LOG << "" << std::endl;
    INFO_LOG
        << "Beginning to create FIMS model in information.hpp CreateModel(). "
        << std::endl;
    INFO_LOG << "Initializing fleet objects for " << this->fleets.size()
             << " fleets." << std::endl;
    FIMS_INFO_LOG("segment");
    for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end();
         ++it) {

      std::shared_ptr<fims_popdy::Fleet<Type> > f = (*it).second;
      INFO_LOG << "Initializing fleet " << f->id << "." << std::endl;
 FIMS_INFO_LOG("segment");
      f->Initialize(f->nyears, f->nages);

      INFO_LOG << "Checking for available fleet selectivity pattern."
               << std::endl;
      // set selectivity model
      if (f->fleet_selectivity_id_m != -999) {
        uint32_t sel_id = static_cast<uint32_t>(
            f->fleet_selectivity_id_m);  // cast as unsigned integer
        selectivity_models_iterator it = this->selectivity_models.find(
            sel_id);  // if find, set it, otherwise invalid
        INFO_LOG << "Input fleet selectivity pattern id = " << sel_id << "."
                 << std::endl;
 FIMS_INFO_LOG("segment");
        if (it != this->selectivity_models.end()) {
          f->selectivity = (*it).second;  // elements in container held in pair
                                          // (first is id, second is object -
                                          // shared pointer to distribution)
          INFO_LOG << "Selectivity successfully set." << std::endl;
           FIMS_INFO_LOG("segment");
        } else {
          valid_model = false;
           FIMS_INFO_LOG("segment");
          ERROR_LOG
              << "Error: Expected selectivity pattern not defined for fleet "
              << f->id << ", selectivity pattern " << sel_id << std::endl;
        }

      } else {
        valid_model = false;
        ERROR_LOG << "Error: No selectivity pattern defined for fleet " << f->id
                  << ". FIMS requires selectivity be defined for all fleets."
                  << std::endl;
      }
      // end set selectivity
    }
    INFO_LOG << "Expecting to import " << this->data_objects.size()
               << " data objects." << std::endl;
     FIMS_INFO_LOG("segment");
    for(density_components_iterator it = this->density_components.begin();
        it!= this->density_components.end(); ++it){
      std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
      INFO_LOG << "Checking for available density components data objects."
          << std::endl;
       FIMS_INFO_LOG("segment");
      //set data objects if distribution is a data type
      if(d->input_type == "data"){
        if(d->observed_data_id_m != -999){
          uint32_t observed_data_id = static_cast<uint32_t>(d->observed_data_id_m);
          data_iterator it = this->data_objects.find(observed_data_id);
          INFO_LOG << "Input data id = " << observed_data_id << "." << std::endl;
 FIMS_INFO_LOG("segment");
          if (it != this->data_objects.end()) {
            d->observed_values = (*it).second;
            INFO_LOG << "Data for density component, " << d->id << " successfully set." << std::endl;
            DATA_LOG << "" << std::endl;
             FIMS_INFO_LOG("segment");
          } else {
            valid_model = false;
            ERROR_LOG << "Error: Expected data observations not defined for density component "
                      << d->id << ", observed data " << observed_data_id << std::endl;
             FIMS_INFO_LOG("segment");
          }

        } else {
          valid_model = false;
          ERROR_LOG << "Error: No data input for density " << d->id << std::endl;
        }
      }
      // end set data
    }

    INFO_LOG << "Initializing population objects for "
             << this->populations.size() << " populations." << std::endl;
    for (population_iterator it = this->populations.begin();
         it != this->populations.end(); ++it) {
         FIMS_INFO_LOG("segment");
      std::shared_ptr<fims_popdy::Population<Type> > p = (*it).second;

      INFO_LOG << "Setting up links from population " << p->id
               << " to fleets [ " << std::flush;
      // error check and set population elements
      // check me - add another fleet iterator to push information from
      for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end();
           ++it) {
        // Initialize fleet object
        std::shared_ptr<fims_popdy::Fleet<Type> > f = (*it).second;
        // population to the individual fleets This is to pass catch at age
        // from population to fleets?
        // any shared member in p (population is pushed into fleets)
        p->fleets.push_back(f);
         FIMS_INFO_LOG("segment");
        INFO_LOG << f->id << " " << std::flush;
      }
      INFO_LOG << "]" << std::endl;

      INFO_LOG << "Initializing population " << p->id << "." << std::endl;
      p->Initialize(p->nyears, p->nseasons, p->nages);

      INFO_LOG << "Checking for available recruitment function." << std::endl;
      // set recruitment
      if (p->recruitment_id != -999) {
           FIMS_INFO_LOG("segment");
        uint32_t recruitment_uint = static_cast<uint32_t>(p->recruitment_id);
        recruitment_models_iterator it =
            this->recruitment_models.find(recruitment_uint);
        INFO_LOG << "Input recruitment id = " << recruitment_uint << "."
                 << std::endl;
        if (it != this->recruitment_models.end()) {
          p->recruitment =
              (*it).second;  // recruitment defined in population.hpp
          INFO_LOG << "Recruitment function successfully set." << std::endl;
        } else {
          valid_model = false;
          ERROR_LOG << "Error: Expected recruitment function not defined for "
                       "population "
                    << p->id << ", recruitment function " << recruitment_uint
                    << std::endl;
        }

      } else {
        valid_model = false;
        ERROR_LOG << "Error: No recruitment function defined for population "
                  << p->id
                  << ". FIMS requires recruitment functions be defined for all "
                     "populations."
                  << std::endl;
      }

      INFO_LOG << "Checking for available growth function." << std::endl;
      // set growth
      if (p->growth_id != -999) {
        uint32_t growth_uint = static_cast<uint32_t>(p->growth_id);
        growth_models_iterator it = this->growth_models.find(
            growth_uint);  // growth_models is specified in information.hpp
         FIMS_INFO_LOG("segment");
        // and used in rcpp
        // at the head of information.hpp; are the
        // dimensions of ages defined in rcpp or where?
        INFO_LOG << "Input growth id = " << growth_uint << "." << std::endl;
        if (it != this->growth_models.end()) {
          p->growth =
              (*it).second;  // growth defined in population.hpp (the object
          // is called p, growth is within p)
          INFO_LOG << "Growth function successfully set." << std::endl;
        } else {
          valid_model = false;
           FIMS_INFO_LOG("segment");
          ERROR_LOG
              << "Error: Expected growth function not defined for population "
              << p->id << ", growth function " << growth_uint << std::endl;
        }

      } else {
        valid_model = false;
        ERROR_LOG << "Error: No growth function defined for population "
                  << p->id
                  << ". FIMS requires growth functions be defined for all "
                     "populations."
                  << std::endl;
      }

      INFO_LOG << "Checking for available maturity function." << std::endl;
      // set maturity
      if (p->maturity_id != -999) {
           FIMS_INFO_LOG("segment");
        uint32_t maturity_uint = static_cast<uint32_t>(p->maturity_id);
        maturity_models_iterator it = this->maturity_models.find(
            maturity_uint);  // >maturity_models is specified in
        // information.hpp and used in rcpp
        INFO_LOG << "Input maturity id = " << maturity_uint << "." << std::endl;
        if (it != this->maturity_models.end()) {
          p->maturity = (*it).second;  // >maturity defined in population.hpp
          INFO_LOG << "Maturity function successfully set." << std::endl;
        } else {
          valid_model = false;
          ERROR_LOG
              << "Error: Expected maturity function not defined for population "
              << p->id << ", maturity function " << maturity_uint << std::endl;
        }
 FIMS_INFO_LOG("segment");
      } else {
        valid_model = false;
        ERROR_LOG << "Error: No maturity function defined for population "
                  << p->id
                  << ". FIMS requires maturity functions be defined for all "
                     "populations."
                  << std::endl;
      }
       FIMS_INFO_LOG("segment");
      INFO_LOG << "Completed initialization for population " << p->id << "."
               << std::endl;
    }
    INFO_LOG << "Completed initialization of all populations." << std::endl;
 FIMS_INFO_LOG("segment");
    //setup priors, random effect, and data density components
    setup_priors();

    INFO_LOG << "Completed FIMS model creation." << std::endl;
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
   * @return std::vector<Type*>&
   */
  std::vector<Type*>& GetParameters() { return parameters; }

  /**
   * @brief Get the Fixed Effects Parameters object
   *
   * @return std::vector<Type*>&
   */
  std::vector<Type*>& GetFixedEffectsParameters() {
    return fixed_effects_parameters;
  }

  /**
   * @brief Get the Random Effects Parameters object
   *
   * @return std::vector<Type*>&
   */
  std::vector<Type*>& GetRandomEffectsParameters() {
    return random_effects_parameters;
  }
};

template <typename Type>
std::shared_ptr<Information<Type> > Information<Type>::fims_information =
    nullptr;  // singleton instance

}  // namespace fims_info

#endif /* FIMS_COMMON_INFORMATION_HPP */
