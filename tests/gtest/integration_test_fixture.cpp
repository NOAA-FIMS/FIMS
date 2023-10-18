#include <sstream>
#include <fstream>
#include <iostream>
#include "../integration/third_party/rapidjson/document.h"
#include "../integration/integration_class.hpp"

#include "population_dynamics/population/population.hpp"

namespace {

// Use test fixture to reuse the same configuration of objects for
// several different tests. To use a test fixture, derive a class
// from testing::Test.
class PopulationIntegrationTestFixture : public testing::Test {
  // Make members protected and they can be accessed from
  // sub-classes.
 protected:
  // Use SetUp function to prepare the objects for each test.
  // Use override in C++11 to make sure SetUp (e.g., not Setup with
  // a lowercase u) is spelled
  // correctly.
  void SetUp() override {
      std::stringstream ss;
        for (uint32_t i = 0; i < this->ncases_m; i++) {
            for (uint32_t j = 0; j < this->ninput_files_m; j++) {
                
                rapidjson::Document input;
                rapidjson::Document output;
                this->ReadJson(ss.str(), input);

                fims::Population<double> pop;



                if (!this->ConfigurePopulationModel(pop, input)) {
                    good = false;
                }

                if (!this->RunModelLoop(pop, input)) {
                    good = false;
                }
                if (!this->CheckModelOutput(pop, output)) {
                    good = false;
                }
            }
  }

  // Virtual void TearDown() will be called after each test is
  // run. It needs to be defined if there is clearup work to
  // do. Otherwise, it does not need to be provided.
  virtual void TearDown() {}

  fims::Population<double> population;

  // Use default values from the Li et al., 2021
  // https://github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison/blob/master/R/save_initial_input.R
  int id_g = 0;
  int nyears = 30;
  int nseasons = 1;
  int nages = 12;
  int nfleets = 2; // one survey fleet and one fishing fleet
};
