/**
 * @file selectivity_registry.hpp
 * @brief Native singleton registry for selectivity backend objects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_SELECTIVITY_REGISTRY_HPP
#define FIMS_INTERFACE_CALL_SELECTIVITY_REGISTRY_HPP

#include <map>
#include <memory>
#include <set>

#if !defined(TMB_MODEL) && !defined(STD_LIB)
#define STD_LIB
#endif

#include "../../common/information.hpp"
#include "../../population_dynamics/selectivity/selectivity.hpp"

template <typename Type>
class NativeSelectivityRegistry
{
public:
    static std::shared_ptr<NativeSelectivityRegistry<Type>> GetInstance()
    {
        if (registry_ == nullptr)
        {
            registry_ = std::make_shared<NativeSelectivityRegistry<Type>>();
        }
        return registry_;
    }

    std::shared_ptr<fims_popdy::LogisticSelectivity<Type>>
    CreateLogisticSelectivity(uint32_t id)
    {
        auto selectivity =
            std::make_shared<fims_popdy::LogisticSelectivity<Type>>();
        selectivity->id = id;
        selectivity->inflection_point.resize(1);
        selectivity->slope.resize(1);
        live_objects_[id] = selectivity;
        info_->selectivity_models[id] = selectivity;
        return selectivity;
    }

    std::shared_ptr<fims_popdy::LogisticSelectivity<Type>>
    GetLogisticSelectivity(uint32_t id)
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

    std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>>
    CreateDoubleLogisticSelectivity(uint32_t id)
    {
        auto selectivity =
            std::make_shared<fims_popdy::DoubleLogisticSelectivity<Type>>();
        selectivity->id = id;
        selectivity->inflection_point_asc.resize(1);
        selectivity->slope_asc.resize(1);
        selectivity->inflection_point_desc.resize(1);
        selectivity->slope_desc.resize(1);
        double_logistic_objects_[id] = selectivity;
        info_->selectivity_models[id] = selectivity;
        return selectivity;
    }

    std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>>
    GetDoubleLogisticSelectivity(uint32_t id)
    {
        auto it = double_logistic_objects_.find(id);
        if (it == double_logistic_objects_.end())
        {
            return nullptr;
        }
        return it->second;
    }

private:
public:
    NativeSelectivityRegistry()
        : info_(fims_info::Information<Type>::GetInstance()) {}

private:
    static std::shared_ptr<NativeSelectivityRegistry<Type>> registry_;
    std::shared_ptr<fims_info::Information<Type>> info_;
    std::map<uint32_t, std::shared_ptr<fims_popdy::LogisticSelectivity<Type>>>
        live_objects_;
    std::map<uint32_t,
             std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>>>
        double_logistic_objects_;
};

template <typename Type>
std::shared_ptr<NativeSelectivityRegistry<Type>>
    NativeSelectivityRegistry<Type>::registry_ = nullptr;

#endif // FIMS_INTERFACE_CALL_SELECTIVITY_REGISTRY_HPP