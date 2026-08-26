// Instructions ----
// This test file verifies that the linkage between a distribution and a model
// parameter (specifically for random effects) is correctly established, both in
// the Rcpp interface layer and in the C++ backend.

#include "gtest/gtest.h"
#include "interface/rcpp/rcpp_objects/rcpp_distribution.hpp"
#include "interface/rcpp/rcpp_objects/rcpp_recruitment.hpp"
#include "common/information.hpp"
#include "interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"

// Test fixture for cleaning up singletons and static members
class DistributionLinkageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear singletons to ensure a clean state for each test
        fims_info::Information<double>::GetInstance()->Clear();
        fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance()->Clear();
        
        // Clear the static lists of live objects in the interfaces
        RecruitmentInterfaceBase::live_objects.clear();
        DistributionsInterfaceBase::live_objects.clear();
        FIMSRcppInterfaceBase::fims_interface_objects.clear();
        
        // Reset global ID counters
        Variable::id_g = 0;
        VariableVector::id_g = 0;
        RecruitmentInterfaceBase::id_g = 0;
        DistributionsInterfaceBase::id_g = 0;
    }

    void TearDown() override {
        // Cleanup is handled by SetUp of the next test, but good practice to have it.
        fims_info::Information<double>::GetInstance()->Clear();
        fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance()->Clear();
        RecruitmentInterfaceBase::live_objects.clear();
        DistributionsInterfaceBase::live_objects.clear();
        FIMSRcppInterfaceBase::fims_interface_objects.clear();
    }
};

TEST_F(DistributionLinkageTest, RandomEffectsLinkageIsCorrect) {
    // 1. Create the Rcpp interface objects.
    // Using `new` mimics R's object creation. The constructors will add
    // these objects to the static `fims_interface_objects` list.
    auto recruitment_interface = new BevertonHoltRecruitmentInterface();
    auto dnorm_interface = new DnormDistributionsInterface();

    // The log_devs vector in the recruitment interface gets a unique ID upon creation.
    uint32_t log_devs_id = recruitment_interface->log_devs.get_id();

    // 2. Link the distribution to the recruitment deviations (log_devs).
    std::vector<uint32_t> observed_ids = {log_devs_id};
    std::vector<uint32_t> expected_ids = {};
    uint32_t uncertainty_id = static_cast<uint32_t>(-999);
    dnorm_interface->set_distribution_links("random_effects", observed_ids, expected_ids, uncertainty_id);

    // 3. [Check 1: Rcpp layer] Verify the key is set correctly in the Rcpp interface object.
    ASSERT_EQ(dnorm_interface->key_m->observed_id.size(), 1);
    EXPECT_EQ(dnorm_interface->key_m->observed_id[0], log_devs_id);
    EXPECT_EQ(dnorm_interface->input_type_m.get(), "random_effects");

    // 4. Push the Rcpp interface objects to the TMB backend.
    recruitment_interface->add_to_fims_tmb();
    dnorm_interface->add_to_fims_tmb();

    // 5. [Check 2: C++ backend pre-setup] Verify the key was transferred correctly to the C++ distribution object.
    auto info = fims_info::Information<double>::GetInstance();
    auto it = info->density_components.find(dnorm_interface->get_id());
    ASSERT_NE(it, info->density_components.end()) << "Distribution component not found in Information map.";

    auto cpp_distribution = std::dynamic_pointer_cast<fims_distributions::NormalLPDF<double>>(it->second);
    ASSERT_TRUE(cpp_distribution) << "Could not cast to NormalLPDF.";

    ASSERT_EQ(cpp_distribution->key.observed_id.size(), 1);
    EXPECT_EQ(cpp_distribution->key.observed_id[0], log_devs_id);
    EXPECT_EQ(cpp_distribution->input_type, "random_effects");

    // 6. Run the setup function that resolves the links.
    info->SetupRandomEffects();

    // 7. [Check 3: C++ backend post-setup] Verify the pointer to the random effect vector was correctly linked.
    EXPECT_NE(cpp_distribution->re, nullptr) << "The random effect vector pointer (re) was not linked.";

    auto rec_it = info->recruitment_models.find(recruitment_interface->get_id());
    ASSERT_NE(rec_it, info->recruitment_models.end()) << "Recruitment model not found in Information map.";
    auto cpp_recruitment = std::dynamic_pointer_cast<fims_popdy::SRBevertonHolt<double>>(rec_it->second);
    ASSERT_TRUE(cpp_recruitment) << "Could not cast to SRBevertonHolt.";

    EXPECT_EQ(cpp_distribution->re, &(cpp_recruitment->log_recruit_devs));
}
