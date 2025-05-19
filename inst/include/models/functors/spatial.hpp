#ifndef FIMS_MODELS_SPATIAL_HPP
#define FIMS_MODELS_SPATIAL_HPP


#include "fishery_model_base.hpp"

namespace fims_popdy {

    template<typename Type>
    class Spatial : public FisheryModelBase<Type> {
    public:

        virtual void Evaluate() {
        }
    };


}


#endif
