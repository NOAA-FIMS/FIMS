/**
 * @file population_registry.hpp
 * @brief Native singleton registry for population backend objects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_POPULATION_REGISTRY_HPP
#define FIMS_INTERFACE_CALL_POPULATION_REGISTRY_HPP

#include <map>
#include <memory>
#include <set>

#if !defined(TMB_MODEL) && !defined(STD_LIB)
#define STD_LIB
#endif

#include "../../common/information.hpp"
#include "../../population_dynamics/population/population.hpp"

template <typename Type>
class NativePopulationRegistry
{
public:
    static std::shared_ptr<NativePopulationRegistry<Type>> GetInstance()
    {
        if (registry_ == nullptr)
        {
            registry_ = std::make_shared<NativePopulationRegistry<Type>>();
        }
        return registry_;
    }

    std::shared_ptr<fims_popdy::Population<Type>> CreatePopulation(uint32_t id)
    {
        auto population = std::make_shared<fims_popdy::Population<Type>>();
        population->id = id;
        live_objects_[id] = population;
        info_->populations[id] = population;
        return population;
    }

    std::shared_ptr<fims_popdy::Population<Type>> GetPopulation(uint32_t id)
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
    NativePopulationRegistry()
        : info_(fims_info::Information<Type>::GetInstance()) {}

private:
    static std::shared_ptr<NativePopulationRegistry<Type>> registry_;
    std::shared_ptr<fims_info::Information<Type>> info_;
    std::map<uint32_t, std::shared_ptr<fims_popdy::Population<Type>>>
        live_objects_;
};

template <typename Type>
std::shared_ptr<NativePopulationRegistry<Type>>
    NativePopulationRegistry<Type>::registry_ = nullptr;

#endif // FIMS_INTERFACE_CALL_POPULATION_REGISTRY_HPP
