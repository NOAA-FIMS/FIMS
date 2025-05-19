#ifndef FIMS_MODELS_FISHERY_MODEL_BASE_HPP
#define FIMS_MODELS_FISHERY_MODEL_BASE_HPP

#include "../../common/model_object.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "../../population_dynamics/population/population.hpp"

namespace fims_popdy {

    template<typename Type>
    class FisheryModelBase : public fims_model_object::FIMSObject<Type> {
        static uint32_t id_g;
        uint32_t id;
    public:

        std::set<uint32_t> population_ids;
        std::vector<std::shared_ptr<fims_popdy::Population<Type> > > populations;

        FisheryModelBase() :
        id(FisheryModelBase::id_g++) {

        }

        void ShowPopulation(std::shared_ptr<fims_popdy::Population<double> >& p) {


            std::cout << "\n\nCAA Model Derived Quantities:\n";
            typename fims_popdy::Population<double>::derived_quantities_iterator it;
            for (it = p->derived_quantities.begin(); it != p->derived_quantities.end(); it++) {
                fims::Vector<double>& dq = (*it).second;
                std::cout << (*it).first << ":" << std::endl;
                for (int i = 0; i < dq.size(); i++) {
                    std::cout << dq[i] << " ";
                }
                std::cout << std::endl << std::endl;
            }


        }

        void Show() {
            for (size_t p = 0; p < this->populations.size(); p++) {
                this->ShowPopulation(this->populations[p]);
            }
        }

        virtual void Initialize() {

        }

        virtual void Prepare() {
        }

        virtual void ResetVector(fims::Vector<Type>& v, Type value = 0.0) {
            std::fill(v.begin(), v.end(), value);
        }

        virtual void Evaluate() {
            FIMS_WARNING_LOG("Not yet implemented.");
        }

        uint32_t GetId() {
            return this->id;
        }

    };

    template<typename Type>
    uint32_t FisheryModelBase<Type>::id_g = 0;

}
#endif
