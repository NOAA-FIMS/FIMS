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
        uint32_t observed_index_data_id;
        std::shared_ptr<fims::DataObject<double> observed_index_data;

        uint32_t observed_agecomp_data_id;
        std::shared_ptr<fims::DataObject<double> observed_agecomp_data;

        //likelihood components
        uint32_t index_likelihood_id;
        std::shared_ptr<fims::LikelihoodBase> index_likelihood;
        
        uint32_t agecomp_likelihood_id;
        std::shared_ptr<fims::LikelihoodBase> agecomp_likelihood;
        
        //selectivity
        uint32_t selectivity_id;
        std::shared_ptr<fims::SelectivityBase> selectivity;
        
        //derived quantities
        std::vector<T> catch_at_age;
        std::vector<T> catch_index;
        std::vector<T> age_composition;
        

        /** @brief Constructor.
         */
        Fleet() {
            this->id = Fleet::id_g++;
        }
        
 

//        void SetNormalIndexLikelihood(T mean, T sd) {
//            this->index_likelihood& = fims::Dnorm<T>();
//            this->index_likelihood.mean = mean;
//            this->index_likelihood.sd = sd;
//        }
//
//        void SetMultinomAgeCompLikelihood(ModelTraits<T>::EigenVector x,
//                ModelTraits<T>::EigenVector p) {
//            this->agecomp_likelihood& = fims::Dmultinom<T>();
//            this->agecomp_likelihood.x = x;
//            this->agecomp_likelihood.p = p;
//        }

        T likelihood() {
            return this->index_likelihood->evaluate(do_log = true)
                    + this->agecomp_likelihood->evaluate(do_log = true);

        }

    };

    template <class T>
    uint32_t Fleet<T>::id_g = 0;

} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
