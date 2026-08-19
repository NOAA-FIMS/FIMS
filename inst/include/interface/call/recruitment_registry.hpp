/**
 * @file recruitment_registry.hpp
 * @brief Native singleton registry for recruitment backend objects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_CALL_RECRUITMENT_REGISTRY_HPP
#define FIMS_INTERFACE_CALL_RECRUITMENT_REGISTRY_HPP

#include <map>
#include <memory>
#include <set>

#if !defined(TMB_MODEL) && !defined(STD_LIB)
#define STD_LIB
#endif

#include "../../common/information.hpp"
#include "../../population_dynamics/recruitment/recruitment.hpp"

template <typename Type>
class NativeRecruitmentRegistry
{
public:
    static std::shared_ptr<NativeRecruitmentRegistry<Type>> GetInstance()
    {
        if (registry_ == nullptr)
        {
            registry_ = std::make_shared<NativeRecruitmentRegistry<Type>>();
        }
        return registry_;
    }

    std::shared_ptr<fims_popdy::SRBevertonHolt<Type>>
    CreateBevertonHoltRecruitment(uint32_t id)
    {
        auto recruitment = std::make_shared<fims_popdy::SRBevertonHolt<Type>>();
        recruitment->id = id;
        recruitment->logit_steep.resize(1);
        recruitment->log_rzero.resize(1);
        beverton_holt_objects_[id] = recruitment;
        info_->recruitment_models[id] = recruitment;
        return recruitment;
    }

    std::shared_ptr<fims_popdy::LogDevs<Type>> CreateLogDevsProcess()
    {
        return std::make_shared<fims_popdy::LogDevs<Type>>();
    }

    std::shared_ptr<fims_popdy::SRBevertonHolt<Type>>
    GetBevertonHoltRecruitment(uint32_t id)
    {
        auto it = beverton_holt_objects_.find(id);
        if (it == beverton_holt_objects_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    std::shared_ptr<fims_info::Information<Type>> GetInformation()
    {
        return info_;
    }

    void Clear() { beverton_holt_objects_.clear(); }

private:
public:
    NativeRecruitmentRegistry()
        : info_(fims_info::Information<Type>::GetInstance()) {}

private:
    static std::shared_ptr<NativeRecruitmentRegistry<Type>> registry_;
    std::shared_ptr<fims_info::Information<Type>> info_;
    std::map<uint32_t, std::shared_ptr<fims_popdy::SRBevertonHolt<Type>>>
        beverton_holt_objects_;
};

template <typename Type>
std::shared_ptr<NativeRecruitmentRegistry<Type>>
    NativeRecruitmentRegistry<Type>::registry_ = nullptr;

#endif // FIMS_INTERFACE_CALL_RECRUITMENT_REGISTRY_HPP
