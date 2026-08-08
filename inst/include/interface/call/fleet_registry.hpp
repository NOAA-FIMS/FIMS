/**
 * @file fleet_registry.hpp
 * @brief Native singleton registry for fleet backend objects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_FLEET_REGISTRY_HPP
#define FIMS_INTERFACE_CALL_FLEET_REGISTRY_HPP

#include <map>
#include <memory>
#include <set>

#if !defined(TMB_MODEL) && !defined(STD_LIB)
#define STD_LIB
#endif

#include "../../common/information.hpp"
#include "../../population_dynamics/fleet/fleet.hpp"

template <typename Type>
class NativeFleetRegistry
{
public:
    static std::shared_ptr<NativeFleetRegistry<Type>> GetInstance()
    {
        if (registry_ == nullptr)
        {
            registry_ = std::make_shared<NativeFleetRegistry<Type>>();
        }
        return registry_;
    }

    std::shared_ptr<fims_popdy::Fleet<Type>> CreateFleet(uint32_t id)
    {
        auto fleet = std::make_shared<fims_popdy::Fleet<Type>>();
        fleet->id = id;
        live_objects_[id] = fleet;
        info_->fleets[id] = fleet;
        return fleet;
    }

    std::shared_ptr<fims_popdy::Fleet<Type>> GetFleet(uint32_t id)
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
    NativeFleetRegistry() : info_(fims_info::Information<Type>::GetInstance()) {}

private:
    static std::shared_ptr<NativeFleetRegistry<Type>> registry_;
    std::shared_ptr<fims_info::Information<Type>> info_;
    std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type>>> live_objects_;
};

template <typename Type>
std::shared_ptr<NativeFleetRegistry<Type>> NativeFleetRegistry<Type>::registry_ =
    nullptr;

#endif // FIMS_INTERFACE_CALL_FLEET_REGISTRY_HPP