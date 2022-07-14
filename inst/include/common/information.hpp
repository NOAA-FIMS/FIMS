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

#include "def.hpp"
// #include "../interface/interface.hpp"
#include "../distributions/distributions.hpp"
#include "../population_dynamics/growth/growth.hpp"
#include "../population_dynamics/recruitment/recruitment.hpp"
#include "../population_dynamics/selectivity/selectivity.hpp"
#include "model_object.hpp"
#include "../population_dynamics/fleet/fleet.hpp"
#include "../population_dynamics/population/population.hpp"



namespace fims {

    /**
     * Contains all objects and data pre-model construction
     */
    template <typename T>
    class Information {
    public:

        size_t nyears;
        size_t nseasons = 1;
        size_t nages;

        static std::shared_ptr<Information<T> > fims_information;
        std::vector<T*> parameters; // list of all estimated parameters
        std::vector<T*> random_effects_parameters; // list of all random effects parameters
        std::vector<T*> fixed_effects_parameters; // list of all fixed effects parameters

        //data objects
        std::map<uint32_t, std::shared_ptr<fims::DataObject<double> > data_objects;
        typedef typename std::map<uint32_t, std::shared_ptr<fims::DataObject<double> >::iterator data_iterator;

        //life history modules
        std::map<uint32_t, std::shared_ptr<fims::RecruitmentBase<T> > >;//hash map to link each object to its shared location in memory
        typedef typename std::map<uint32_t, std::shared_ptr<fims::RecruitmentBase<T> > >::iterator recruitment_model_iterator;

        std::map<uint32_t, std::shared_ptr<fims::SelectivityBase<T> > > selectivity_models;
        typedef typename std::map<uint32_t, std::shared_ptr<fims::SelectivityBase<T> > >::iterator selectivity_models_iterator;

        std::map<uint32_t, std::shared_ptr<fims::GrowthBase<T> > > growth_models;
        typedef typename std::map<uint32_t, std::shared_ptr<fims::GrowthBase<T> > >::iterator growth_models_iterator;

        //fleet modules
        std::map<uint32_t, std::shared_ptr<fims::Fleet<T> > > fleets;
        typedef typename std::map<uint32_t, std::shared_ptr<fims::Fleet<T> > >::iterator fleet_iterator;

        //populations
        std::map<uint32_t, std::shared_ptr<fims::Population<T> > > populations;
        typedef typename std::map<uint32_t, std::shared_ptr<fims::Population<T> > >::iterator population_iterator;

        //distributions
        std::map<uint32_t, std::shared_ptr<fims::DistributionsBase> > distribution_models;
        typedef typename std::map<uint32_t, std::shared_ptr<fims::DistributionsBase> >::iterator distribution_models_iterator;

        Information() {

        }

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
        void RegisterParameter(T& p) {
            this->parameters.push_back(&p);
        }

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

            bool valid_model = true;

            std::cout << "Information: Initializing fleet objects.\n";
            for (fleet_iterator it = this->fleets.begin();
                    it != this->fleets.end(); ++it) {

                //Initialize fleet object 
                std::shared_ptr<fims::Fleet<T> > f = (*it).second;

                //set index data
                if (f->observed_index_data_id != -999) {

                    uint32_t index_id = static_cast<uint32_t> (f->observed_index_data_id);
                    data_iterator it = this->data_objects.find(index_id);

                    if (it != this->data_objects.end()) {
                        f->observed_index_data = (*it).second;
                    } else {
                        valid_model = false;
                        //log error
                    }

                } else {
                    valid_model = false;
                    //log error
                }

                //set age composition data
                if (f->observed_agecomp_data_id != -999) {

                    uint32_t agecomp_id = static_cast<uint32_t> (f->observed_agecomp_data_id);
                    data_iterator it = this->data_objects.find(agecomp_id);

                    if (it != this->data_objects.end()) {
                        f->observed_agecomp_data = (*it).second;
                    } else {
                        valid_model = false;
                        //log error
                    }

                } else {
                    valid_model = false;
                    //log error
                }

                //set selectivity model
                if (f->selectivity_id != -999) {

                    uint32_t sel_id = static_cast<uint32_t> (f->selectivity_id); //cast as unsigned integer
                    selectivity_models_iterator it = this->selectivity_models.find(sel_id); //if find, set it, otherwise invalid

                    if (it != this->selectivity_models.end()) {
                        f->selectivity = (*it).second; //elements in container held in pair (first is id, second is object - shared pointer to distribution)
                    } else {
                        valid_model = false;
                        //log error
                    }

                } else {
                    valid_model = false;
                    //log error
                }

                //set index likelihood 
                if (f->index_likelihood_id != -999) {

                    uint32_t ind_like_id = static_cast<uint32_t> (f->index_likelihood_id); //cast as unsigned integer
                    distribution_models_iterator it = this->distribution_models.find(ind_like_id); //if find, set it, otherwise invalid
                    
                    if (it != this->distribution_models.end()) {
                        f->index_likelihood = (*it).second; //elements in container held in pair (first is id, second is object - shared pointer to distribution)
                    } else {
                        valid_model = false;
                        //log error
                    }

                } else {
                    valid_model = false;
                    //log error
                }

                //set agecomp likelihood 
                if (f->agecomp_likelihood_id != -999) {

                    uint32_t ac_like_id = static_cast<uint32_t> (f->agecomp_likelihood_id); //cast as unsigned integer
                    distribution_models_iterator it = this->distribution_models.find(ac_like_id); //if find, set it, otherwise invalid

                    if (it != this->distribution_models.end()) {
                        f->agecomp_likelihood = (*it).second; //elements in container held in pair (first is id, second is object - shared pointer to distribution)
                    } else {
                        valid_model = false;
                        //log error
                    }

                } else {
                    valid_model = false;
                    //log error
                }

                //initialize derived quantities containers
                f->catch_at_age.resize(nyears * nseasons * nages);
                f->catch_at_age.resize(nyears * nseasons * nages);
                f->age_composition.resize(nyears * nseasons);
                //error check and set fleet elements here
            }

            std::cout << "Information: Initializing population objects.\n";
            for (population_iterator it = this->populations.begin();
                    it != this->populations.end(); ++it) {

                std::shared_ptr<fims::Population<T> > p = (*it).second;
                //error check and set population elements here
            }
            return valid_model;
        }

        size_t GetNages() const {
            return nages;
        }

        void SetNages(size_t nages) {
            this->nages = nages;
        }

        size_t GetNseasons() const {
            return nseasons;
        }

        void SetNseasons(size_t nseasons) {
            this->nseasons = nseasons;
        }

        size_t GetNyears() const {
            return nyears;
        }

        void SetNyears(size_t nyears) {
            this->nyears = nyears;
        }

        std::vector<T*>& GetParameters() {
            return parameters;
        }

        std::vector<T*>& GetFixedEffectsParameters() {
            return fixed_effects_parameters;
        }

        std::vector<T*>& GetRandomEffectsParameters() {
            return random_effects_parameters;
        }


    };

    template <typename T>
    std::shared_ptr<Information<T> > Information<T>::fims_information =
    nullptr; // singleton instance

} // namespace fims

#endif /* FIMS_COMMON_INFORMATION_HPP */
