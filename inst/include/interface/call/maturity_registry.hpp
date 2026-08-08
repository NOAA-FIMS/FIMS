/**
 * @file maturity_registry.hpp
 * @brief Native singleton registry for maturity backend objects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_MATURITY_REGISTRY_HPP
#define FIMS_INTERFACE_CALL_MATURITY_REGISTRY_HPP

#include <map>
#include <memory>
#include <set>

#if !defined(TMB_MODEL) && !defined(STD_LIB)
#define STD_LIB
#endif

#include "../../common/information.hpp"
#include "../../population_dynamics/maturity/maturity.hpp"

template <typename Type>
class NativeMaturityRegistry
{
public:
    static std::shared_ptr<NativeMaturityRegistry<Type>> GetInstance()
    {
        if (registry_ == nullptr)
        {
            registry_ = std::make_shared<NativeMaturityRegistry<Type>>();
        }
        return registry_;
    }

    std::shared_ptr<fims_popdy::LogisticMaturity<Type>>
    CreateLogisticMaturity(uint32_t id)
    {
        auto maturity = std::make_shared<fims_popdy::LogisticMaturity<Type>>();
        maturity->id = id;
        maturity->inflection_point.resize(1);
        maturity->slope.resize(1);
        live_objects_[id] = maturity;
        info_->maturity_models[id] = maturity;
        return maturity;
    }

    std::shared_ptr<fims_popdy::LogisticMaturity<Type>>
    GetLogisticMaturity(uint32_t id)
    {
        auto it = live_objects_.find(id);
        if (it == live_objects_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    std::shared_ptr<fims_info::Information<Type>> GetInformation()
    {
        return info_;
    }

private:
public:
    NativeMaturityRegistry() : info_(fims_info::Information<Type>::GetInstance()) {}

private:
    static std::shared_ptr<NativeMaturityRegistry<Type>> registry_;
    std::shared_ptr<fims_info::Information<Type>> info_;
    std::map<uint32_t, std::shared_ptr<fims_popdy::LogisticMaturity<Type>>>
        live_objects_;
};

template <typename Type>
std::shared_ptr<NativeMaturityRegistry<Type>>
    NativeMaturityRegistry<Type>::registry_ = nullptr;

#endif // FIMS_INTERFACE_CALL_MATURITY_REGISTRY_HPP