/**
 * @file information.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_COMMON_INFORMATION_HPP
#define FIMS_COMMON_INFORMATION_HPP

#include <map>
#include <memory>
#include <vector>
#include <algorithm>

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
        size_t nyears = 0; /**< number of years >*/
        size_t nseasons = 1; /**< number of seasons >*/
        size_t nages = 0; /**< number of ages>*/

        static std::shared_ptr<Information<Type> >
        fims_information; /**< singleton instance >*/
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

        Information() {
        }

        virtual ~Information() {
        }
        
        /**
         * @brief Clears all containers.
         * 
         */
        void Clear(){
            this->data_objects.clear();
            this->density_components.clear();
            this->fixed_effects_parameters.clear();
            this->fleets.clear();
            this->growth_models.clear();
            this->maturity_models.clear();
            this->parameter_names.clear();
            this->parameters.clear();
            this->random_effects_parameters.clear();
            this->recruitment_models.clear();
            this->selectivity_models.clear();
            this->variable_map.clear();
            this->nyears = 0;
            this->nseasons = 0;
            this->nages = 0;
        }

        /**
         * @brief Returns a singleton Information object for type T.
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
         * @brief Register a parameter as estimable.
         *
         * @param p
         */
        void RegisterParameter(Type& p) {
            this->fixed_effects_parameters.push_back(&p);
        }

        /**
         * @brief Register a random effect as estimable.
         *
         * @param re
         */
        void RegisterRandomEffect(Type& re) {
            this->random_effects_parameters.push_back(&re);
        }

        /**
         * @brief Register a parameter name.
         *
         * @param p_name
         */
        void RegisterParameterName(std::string p_name) {
            this->parameter_names.push_back(p_name);
        }

        /**
         * @brief Loop over distributions and set links to distribution x value if distribution is a prior type.
         */
        void SetupPriors() {
            for (density_components_iterator it = density_components.begin(); it != density_components.end(); ++it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
                if (d->input_type == "prior") {
                    FIMS_INFO_LOG("Setup prior for distribution " + fims::to_string(d->id));
                    variable_map_iterator vmit;
                    FIMS_INFO_LOG("Link prior from distribution " + fims::to_string(d->id) + " to parameter " + fims::to_string(d->key[0]));
                    vmit = this->variable_map.find(d->key[0]);
                    d->x = *(*vmit).second;
                    for (size_t i = 1; i < d->key.size(); i++) {
                        FIMS_INFO_LOG("Link prior from distribution " + fims::to_string(d->id)
                                + " to parameter " + fims::to_string(d->key[0]));
                        vmit = this->variable_map.find(d->key[i]);
                        d->x.insert(std::end(d->x),
                                std::begin(*(*vmit).second), std::end(*(*vmit).second));
                    }
                    FIMS_INFO_LOG("Prior size for distribution " + fims::to_string(d->id) + "is: " + fims::to_string(d->x.size()));
                }
            }
        }

        /**
         * @brief Loop over distributions and set links to distribution x value if distribution is a random effects type.
         */
        void SetupRandomEffects() {
            for (density_components_iterator it = this->density_components.begin(); it != this->density_components.end(); ++it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
                if (d->input_type == "random_effects") {
                    FIMS_INFO_LOG("Setup random effects for distribution " + fims::to_string(d->id));
                    variable_map_iterator vmit;
                    FIMS_INFO_LOG("Link random effects from distribution "
                            + fims::to_string(d->id) + " to derived value " + fims::to_string(d->key[0]));
                    vmit = this->variable_map.find(d->key[0]);
                    d->x = *(*vmit).second;
                    for (size_t i = 1; i < d->key.size(); i++) {
                        FIMS_INFO_LOG("Link random effects from distribution " + fims::to_string(d->id)
                                + " to derived value " + fims::to_string(d->key[0]));
                        vmit = this->variable_map.find(d->key[i]);
                        d->x.insert(std::end(d->x),
                                std::begin(*(*vmit).second), std::end(*(*vmit).second));
                    }
                    FIMS_INFO_LOG("Random effect size for distribution " + fims::to_string(d->id) + " is: " + fims::to_string(d->x.size()));
                }
            }
        }

        /**
         * @brief Loop over distributions and set links to distribution expected value if distribution is a data type.
         */
        void SetupData() {
            for (density_components_iterator it = this->density_components.begin(); it != this->density_components.end(); ++it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;
                if (d->input_type == "data") {
                    FIMS_INFO_LOG("Setup expected value for data distribution " + fims::to_string(d->id));
                    variable_map_iterator vmit;
                    FIMS_INFO_LOG("Link expected value from distribution " + fims::to_string(d->id)
                            + " to derived value " + fims::to_string(d->key[0]));
                    vmit = this->variable_map.find(d->key[0]);
                    d->expected_values = *(*vmit).second;

                    for (size_t i = 1; i < d->key.size(); i++) {
                        vmit = this->variable_map.find(d->key[i]);
                        FIMS_INFO_LOG("Link expected value from distribution "
                                + fims::to_string(d->id) + " to derived value " + fims::to_string(d->key[i]));
                        d->expected_values.insert(std::end(d->expected_values),
                                std::begin(*(*vmit).second), std::end(*(*vmit).second));
                    }
                    FIMS_INFO_LOG("Expected value size for distribution " + fims::to_string(d->id)
                            + " is: " + fims::to_string(d->expected_values.size()));
                }
            }
        }

        /**
         * @brief Set pointers to index data in the fleet module.
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param f shared pointer to fleet module
         */
        void SetFleetIndexData(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Fleet<Type> > f) {
            if (f->fleet_observed_index_data_id_m != -999) {
                uint32_t observed_index_id =
                        static_cast<uint32_t> (f->fleet_observed_index_data_id_m);
                data_iterator it = this->data_objects.find(observed_index_id);
                if (it != this->data_objects.end()) {
                    f->observed_index_data = (*it).second;
                    FIMS_INFO_LOG("Index data for fleet "
                            + fims::to_string(f->id) + " successfully set to "
                            + fims::to_string(f->observed_index_data->at(1)));
                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected index data not defined for fleet "
                            + fims::to_string(f->id) + ", index "
                            + fims::to_string(observed_index_id));
                }
            } else {
                valid_model = false;
                // TODO: explore why index data is required because it should not be
                FIMS_ERROR_LOG("No index data observed for fleet "
                        + fims::to_string(f->id) + ". FIMS requires index data for all fleets.");
            }
        }

        /**
         * @brief Set pointers to age composition data in the fleet module.
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param f shared pointer to fleet module
         */
        void SetAgeCompositionData(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Fleet<Type> > f) {
            if (f->fleet_observed_agecomp_data_id_m != -999) {
                uint32_t observed_agecomp_id =
                        static_cast<uint32_t> (f->fleet_observed_agecomp_data_id_m);
                data_iterator it = this->data_objects.find(observed_agecomp_id);
                if (it != this->data_objects.end()) {
                    f->observed_agecomp_data = (*it).second;
                    FIMS_INFO_LOG("Observed input age-composition data for fleet "
                            + fims::to_string(f->id) + " successfully set to "
                            + fims::to_string(f->observed_agecomp_data->at(1)));
                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected age-composition observations not defined for fleet "
                            + fims::to_string(f->id));
                }
            }
        }

        /**
         * @brief Set pointers to length composition data in the fleet module.
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param f shared pointer to fleet module
         */
        void SetLengthCompositionData(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Fleet<Type> > f) {
            if (f->fleet_observed_lengthcomp_data_id_m != -999) {
                uint32_t observed_lengthcomp_id =
                        static_cast<uint32_t> (f->fleet_observed_lengthcomp_data_id_m);
                data_iterator it = this->data_objects.find(observed_lengthcomp_id);
                if (it != this->data_objects.end()) {
                    f->observed_lengthcomp_data = (*it).second;
                    FIMS_INFO_LOG("Observed input length-composition data for fleet "
                            + fims::to_string(f->id) + " successfully set to "
                            + fims::to_string(f->observed_lengthcomp_data->at(1)));
                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected length-composition observations not defined for fleet "
                            + fims::to_string(f->id));
                }
            }
        }

        /**
         * @brief Set pointers to the selectivity module referenced in the fleet module.
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param f shared pointer to fleet module
         */
        void SetFleetSelectivityModel(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Fleet<Type> > f) {
            if (f->fleet_selectivity_id_m != -999) {
                uint32_t sel_id = static_cast<uint32_t> (
                        f->fleet_selectivity_id_m); // cast as unsigned integer
                selectivity_models_iterator it = this->selectivity_models.find(
                        sel_id); // if find, set it, otherwise invalid

                if (it != this->selectivity_models.end()) {
                    f->selectivity = (*it).second; // elements in container held in pair
                    FIMS_INFO_LOG("Selectivity model "
                            + fims::to_string(f->fleet_selectivity_id_m)
                            + " successfully set to fleet " + fims::to_string(f->id));

                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected selectivity pattern not defined for fleet "
                            + fims::to_string(f->id) + ", selectivity pattern " + fims::to_string(sel_id));
                }

            } else {
                valid_model = false;
                FIMS_ERROR_LOG("Error: No selectivity pattern defined for fleet " + fims::to_string(f->id)
                        + ". FIMS requires selectivity be defined for all fleets.");
            }
        }

        /**
         * @brief Set pointers to the recruitment module referened in the population module. 
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param p shared pointer to population module
         */
        void SetRecruitment(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Population<Type> > p) {
            if (p->recruitment_id != -999) {
                uint32_t recruitment_uint = static_cast<uint32_t> (p->recruitment_id);
                recruitment_models_iterator it =
                        this->recruitment_models.find(recruitment_uint);

                if (it != this->recruitment_models.end()) {
                    p->recruitment =
                            (*it).second; // recruitment defined in population.hpp
                    FIMS_INFO_LOG("Recruitment model "
                            + fims::to_string(recruitment_uint)
                            + " successfully set to population "
                            + fims::to_string(p->id));
                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected recruitment function not defined for "
                            "population "
                            + fims::to_string(p->id) + ", recruitment function "
                            + fims::to_string(recruitment_uint));
                }

            } else {
                valid_model = false;
                FIMS_ERROR_LOG("No recruitment function defined for population "
                        + fims::to_string(p->id)
                        + ". FIMS requires recruitment functions be defined for all "
                        "populations.");
            }
        }

        /**
         * @brief Set pointers to the growth module referened in the population module. 
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param p shared pointer to population module
         */
        void SetGrowth(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Population<Type> > p) {
            if (p->growth_id != -999) {
                uint32_t growth_uint = static_cast<uint32_t> (p->growth_id);
                growth_models_iterator it = this->growth_models.find(
                        growth_uint); // growth_models is specified in information.hpp
                // and used in rcpp
                // at the head of information.hpp; are the
                // dimensions of ages defined in rcpp or where?
                if (it != this->growth_models.end()) {
                    p->growth =
                            (*it).second; // growth defined in population.hpp (the object
                    // is called p, growth is within p)
                    FIMS_INFO_LOG("Growth model "
                            + fims::to_string(growth_uint)
                            + " successfully set to population " + fims::to_string(p->id));
                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected growth function not defined for population "
                            + fims::to_string(p->id) + ", growth function "
                            + fims::to_string(growth_uint));
                }

            } else {
                valid_model = false;
                FIMS_ERROR_LOG("No growth function defined for population "
                        + fims::to_string(p->id)
                        + ". FIMS requires growth functions be defined for all "
                        "populations.");
            }
        }

        /**
         * @brief Set pointers to the maturity module referened in the population module. 
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         * @param p shared pointer to population module
         */
        void SetMaturity(
                bool &valid_model,
                std::shared_ptr<fims_popdy::Population<Type> > p) {
            if (p->maturity_id != -999) {
                uint32_t maturity_uint = static_cast<uint32_t> (p->maturity_id);
                maturity_models_iterator it = this->maturity_models.find(
                        maturity_uint); // >maturity_models is specified in
                // information.hpp and used in rcpp
                if (it != this->maturity_models.end()) {
                    p->maturity = (*it).second; // >maturity defined in population.hpp
                    FIMS_INFO_LOG("Maturity model "
                            + fims::to_string(maturity_uint)
                            + " successfully set to population " + fims::to_string(p->id));
                } else {
                    valid_model = false;
                    FIMS_ERROR_LOG("Expected maturity function not defined for population "
                            + fims::to_string(p->id) + ", maturity function "
                            + fims::to_string(maturity_uint));
                }
            } else {

                valid_model = false;
                FIMS_ERROR_LOG("No maturity function defined for population "
                        + fims::to_string(p->id)
                        + ". FIMS requires maturity functions be defined for all "
                        "populations.");
            }
        }

        /**
         * @brief Loop over all fleets and set pointers to fleet objects
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         */
        void CreateFleetObjects(bool &valid_model) {
            for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end();
                    ++it) {

                std::shared_ptr<fims_popdy::Fleet<Type> > f = (*it).second;
                FIMS_INFO_LOG("Initializing fleet " + fims::to_string(f->id));

                f->Initialize(f->nyears, f->nages, f->nlengths);

                SetFleetIndexData(valid_model, f);

                SetAgeCompositionData(valid_model, f);

                SetLengthCompositionData(valid_model, f);

                SetFleetSelectivityModel(valid_model, f);
            }
        }

        /**
         * @brief Loop over all density components and set pointers to data objects
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         */
        void SetDataObjects(bool &valid_model) {
            for (density_components_iterator it = this->density_components.begin();
                    it != this->density_components.end(); ++it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*it).second;

                //set data objects if distribution is a data type
                if (d->input_type == "data") {
                    if (d->observed_data_id_m != -999) {
                        uint32_t observed_data_id = static_cast<uint32_t> (d->observed_data_id_m);
                        data_iterator it = this->data_objects.find(observed_data_id);

                        if (it != this->data_objects.end()) {
                            d->observed_values = (*it).second;
                            FIMS_INFO_LOG("Observed data "
                                    + fims::to_string(observed_data_id)
                                    + " successfully set to density component " + fims::to_string(d->id));
                        } else {
                            valid_model = false;
                            FIMS_ERROR_LOG("Expected data observations not defined for density component "
                                    + fims::to_string(d->id) + ", observed data " + fims::to_string(observed_data_id));
                        }

                    } else {
                        valid_model = false;
                        FIMS_ERROR_LOG("No data input for density component" + fims::to_string(d->id));
                    }
                }
            }

        }

        /**
         * @brief Loop over all populations and set pointers to population objects
         * 
         * @param &valid_model reference to true/false boolean indicating whether model is valid.
         */
        void CreatePopulationObjects(bool &valid_model) {
            for (population_iterator it = this->populations.begin();
                    it != this->populations.end(); ++it) {
                std::shared_ptr<fims_popdy::Population<Type> > p = (*it).second;

                FIMS_INFO_LOG("Initializing population " + fims::to_string(p->id));
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
                }

                p->Initialize(p->nyears, p->nseasons, p->nages);

                //set information dimensions 
                this->nyears = std::max(this->nyears, p->nyears);
                this->nages = std::max(this->nages, p->nages);
                this->nseasons = std::max(this->nseasons, p->nseasons);

                SetRecruitment(valid_model, p);

                SetGrowth(valid_model, p);

                SetMaturity(valid_model, p);

            }
        }

        /**
         * @brief Create the generalized stock assessment model that will evaluate the
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

            CreateFleetObjects(valid_model);

            SetDataObjects(valid_model);

            CreatePopulationObjects(valid_model);

            //setup priors, random effect, and data density components
            SetupPriors();

            return valid_model;
        }

        /**
         * @brief Get the Nages object
         *
         * @return size_t
         */
        size_t GetNages() const {

            return nages;
        }

        /**
         * @brief Set the Nages object
         *
         * @param nages
         */
        void SetNages(size_t nages) {

            this->nages = nages;
        }

        /**
         * @brief Get the Nseasons object
         *
         * @return size_t
         */
        size_t GetNseasons() const {

            return nseasons;
        }

        /**
         * @brief Set the Nseasons object
         *
         * @param nseasons
         */
        void SetNseasons(size_t nseasons) {

            this->nseasons = nseasons;
        }

        /**
         * @brief Get the Nyears object
         *
         * @return size_t
         */
        size_t GetNyears() const {

            return nyears;
        }

        /**
         * @brief Set the Nyears object
         *
         * @param nyears
         */
        void SetNyears(size_t nyears) {

            this->nyears = nyears;
        }

        /**
         * @brief Get the Parameters object
         *
         * @return std::vector<Type*>&
         */
        std::vector<Type*>& GetParameters() {

            return parameters;
        }

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
    nullptr; // singleton instance

} // namespace fims_info

#endif /* FIMS_COMMON_INFORMATION_HPP */
