#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"
#include "../../tests/integration/integration_class.hpp"
#include <filesystem>

namespace
{
    TEST(Integrated_test_works, run_all)
    {

    //Declare IntegrationTest object
    IntegrationTest t(1, 1);
    std::stringstream ss;
    typename rapidjson::Document::MemberIterator it;

    bool good = true;

    //Read in input and output json files
    rapidjson::Document input;
    rapidjson::Document output;
    
    //Read inputs
    ss.str("");
    //GoogleTest operates in the folder with executables "build/tests/gtest"
    //so we have to go up three directories to get into FIMS folder
    ss << "../../../tests/integration/inputs/C" << 0 << "/om_input" << 1 << ".json";
    t.ReadJson(ss.str(), input);
    ss.str("");
    
    //Read in outputs
    ss << "../../../tests/integration/inputs/C" << 0 << "/om_output" << 1 << ".json";
    t.ReadJson(ss.str(), output);

    //Declare singleton of population class
    fims::Population<double> pop;

    //ConfigurePopulationModel, RunModelLoop, and CheckModelOutput 
    // methods are in integration_class.hpp
    good = t.ConfigurePopulationModel(pop, input);
    pop.numbers_at_age = t.RunModelLoop(pop, input);
    good = t.CheckModelOutput(pop, output);

    //declare vector of doubles to hold
    //dimension folded numbers at age vector
    //resize to nyears * nages
    std::vector<double> test_numbers_at_age;
    test_numbers_at_age.resize(pop.nages*pop.nyears);

    //find the json member called "N.age"
    it = output.FindMember("N.age");

    if (it != output.MemberEnd()) {
    rapidjson::Value &e = (*it).value;
    for (int year = 0; year < e.Size(); year++) {
        for(int age = 0; age < e[year].Size(); age++){
            int index_ya = year * pop.nages + age;
            test_numbers_at_age[index_ya] = e[year][age].GetDouble();    
            EXPECT_EQ(pop.numbers_at_age[index_ya], test_numbers_at_age[index_ya]);
        }
    }
    } else {
            std::cout << "N.age not found in output\n";
    }
    }

    // TEST_F(PopulationPrepareTestFixture, CalculateNumbersAA_forloop_works)
    // {

    //     for (size_t year = 0; year < nyears; year++)
    //     {
    //         for (size_t age = 0; age < nages; age++)
    //         {

    //             int index_ya = year * population.nages + age;
    //             // Calculate mortality starts from year = 0 and age = 0
    //             population.CalculateMortality(index_ya, year, age);

    //             // loop over fleets to calculate mortality and set up
    //             // true values for the test
    //             std::vector<double> mortality_F(nyears * nages, 0.0);

    //             for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
    //             {
    //                 size_t index_yf = year * population.nfleets + fleet_index;
    //                 mortality_F[index_ya] += population.fleets[fleet_index]->Fmort[index_yf] *
    //                                          population.fleets[fleet_index]->selectivity->evaluate(age);
    //             }

    //             std::vector<double> mortality_Z(nyears * nages, 0.0);
    //             mortality_Z[index_ya] = fims::exp(population.log_M[index_ya]) +
    //                                     mortality_F[index_ya];
    //             EXPECT_EQ(population.mortality_Z[index_ya], mortality_Z[index_ya]);

    //             std::vector<double> test_numbers_at_age((nyears + 1) * nages, 0.0);
    //             if (year == 0)
    //             {
    //                 // Calculate initial numbers at age when year = 0
    //                 population.CalculateInitialNumbersAA(index_ya, age);
    //                 // Set up true values for the test
    //                 test_numbers_at_age[index_ya] = population.naa[age];
    //                 EXPECT_EQ(fims::exp(population.log_naa[age]), test_numbers_at_age[index_ya]);
    //             }

    //             if (year > 0 && age == 0)
    //             {
    //                 population.CalculateRecruitment(index_ya, year);
    //                 // The test will get complicated if we calculate
    //                 // test_numbers_at_age by calling recruitment module
    //             }

    //             if (year > 0 && age > 0)
    //             {
    //                 int index_ya2 = (year - 1) * population.nages + age - 1;
    //                 // this needs to be after we calculate mortality
    //                 population.CalculateNumbersAA(index_ya, index_ya2);
    //                 test_numbers_at_age[index_ya] = test_numbers_at_age[index_ya2] * fims::exp(-mortality_Z[index_ya2]);
    //             }

    //             EXPECT_EQ(population.numbers_at_age[index_ya], test_numbers_at_age[index_ya]);
    //         }
    //     }
    // }

}