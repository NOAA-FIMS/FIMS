/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 * 
 * The purpose of this file is to declare the growth functor class
 * which is the base class for all growth functors.
 */
#ifndef FIMS_POPULATION_DYNAMICS_FLEET_HPP
#define FIMS_POPULATION_DYNAMICS_FLEET_HPP

#include "../../common/model_object.hpp"
#include "../../common/data_object.hpp"
//#include "../../common/likelihood.hpp"
#include "../selectivity/selectivity.hpp"

namespace fims {

    /* @brief Base class for all fleets.
     *
     * @tparam T The type of the fleet object.
     * */
    template<typename T>
    struct Fleet : public FIMSObject<T> {
        static uint32_t id_g; /*!< reference id for fleet object*/


        //data objects
        int observed_index_data_id = -999;
        std::shared_ptr<fims::DataObject<double> observed_index_data;

        int observed_agecomp_data_id = -999;
        std::shared_ptr<fims::DataObject<double> observed_agecomp_data;

        //likelihood components
        int index_likelihood_id = -999;
        std::shared_ptr<fims::LikelihoodBase<T> > index_likelihood;

        int agecomp_likelihood_id = -999;
        std::shared_ptr<fims::LikelihoodBase<T> > agecomp_likelihood;

        //selectivity
        int selectivity_id = -999;
        std::shared_ptr<fims::SelectivityBase<T> > selectivity;

        //derived quantities
        std::vector<T> catch_at_age;
        std::vector<T> catch_index;
        std::vector<T> age_composition;

        /** 
         * @brief Constructor.
         */
        Fleet() {
            this->id = Fleet::id_g++;
        }

        const T likelihood() {
#warning this needs review
            return this->index_likelihood->evaluate(do_log = true)
                    + this->agecomp_likelihood->evaluate(do_log = true);
        }

    };
    template <class T>
    uint32_t Fleet<T>::id_g = 0;

} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
