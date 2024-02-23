

/* 
 * File:   area.hpp
 * Author: mattadmin
 *
 * Created on February 22, 2024, 9:05 AM
 */

#ifndef FIMS_POPULATION_DYNAMICS_AREA_HPP
#define FIMS_POPULATION_DYNAMICS_AREA_HPP
#include <map>
#include "../recruitment/recruitment.hpp"
#include "../growth/growth.hpp"
#include "../population/subpopulation.hpp"

template<typename Type>
class Area {
    
public:
    std::map<uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type> > area_specific_recruitment;
    typedef typename std::map<uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type> >::iterator recruitment_iterator;
    std::map<uint32_t, std::shared_ptr<fims_popdy::GrowthBase<Type> > area_specific_growth;
    typedef typename std::map<uint32_t, std::shared_ptr<fims_popdy::GrowthBase<Type> >::iterator growth_iterator;

    void EvluateRecruitment(std::shared_ptr<fims::popdy::Subpopulation>& subpop) {
        recruitment_iterator it = this->area_specific_recruitment.find(subpop->id_m);
        if (it != this->area_specific_recruitment.end()) {
            //area specific recruitment
        } else {
            //population specific recruitment
        }
    }

    void EvluateGrowth(std::shared_ptr<fims::popdy::Subpopulation>& subpop) {
        growth_iterator it = this->area_specific_growth.find(subpop->id_m);
        if (it != this->area_specific_growth.end()) {
            //area specific growth
        } else {
            //population specific growth
        }
    }


};



#endif /* FIMS_POPULATION_DYNAMICS_ */

