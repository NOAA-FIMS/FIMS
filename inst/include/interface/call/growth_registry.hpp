/**
 * @file growth_registry.hpp
 * @brief Native singleton registry for growth backend objects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_GROWTH_REGISTRY_HPP
#define FIMS_INTERFACE_CALL_GROWTH_REGISTRY_HPP

#include <map>
#include <memory>
#include <set>

#if !defined(TMB_MODEL) && !defined(STD_LIB)
#define STD_LIB
#endif

#include "../../common/information.hpp"
#include "../../population_dynamics/growth/growth.hpp"

template <typename Type>
class NativeGrowthRegistry
{
public:
    static std::shared_ptr<NativeGrowthRegistry<Type>> GetInstance()
    {
        if (registry_ == nullptr)
        {
            registry_ = std::make_shared<NativeGrowthRegistry<Type>>();
        }
        return registry_;
    }

    std::shared_ptr<fims_popdy::EWAAGrowth<Type>> CreateEWAAGrowth(uint32_t id)
    {
        auto growth = std::make_shared<fims_popdy::EWAAGrowth<Type>>();
        growth->id = id;
        live_objects_[id] = growth;
        info_->growth_models[id] = growth;
        return growth;
    }

    std::shared_ptr<fims_popdy::EWAAGrowth<Type>> GetEWAAGrowth(uint32_t id)
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

    void Clear() { live_objects_.clear(); }

private:
public:
    NativeGrowthRegistry() : info_(fims_info::Information<Type>::GetInstance()) {}

private:
    static std::shared_ptr<NativeGrowthRegistry<Type>> registry_;
    std::shared_ptr<fims_info::Information<Type>> info_;
    std::map<uint32_t, std::shared_ptr<fims_popdy::EWAAGrowth<Type>>> live_objects_;
};

template <typename Type>
std::shared_ptr<NativeGrowthRegistry<Type>> NativeGrowthRegistry<Type>::registry_ =
    nullptr;

#endif // FIMS_INTERFACE_CALL_GROWTH_REGISTRY_HPP
