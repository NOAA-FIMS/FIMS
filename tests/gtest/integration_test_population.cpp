#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/integration/integration_class.hpp"

namespace
{
    TEST(integration_test, mcp_c0_om1_works)
    {
        std::ofstream integration_test_log("log_integration_test.txt");
        // Declare IntegrationTest object
        IntegrationTest t(1, 1);
        std::stringstream ss;
        typename rapidjson::Document::MemberIterator it;

        bool good = true;

        // Read in input and output json files
        rapidjson::Document input;
        rapidjson::Document output;

        // Read inputs
        ss.str("");
        // GoogleTest operates in the folder with executables "build/tests/gtest"
        // so we have to go up three directories to get into FIMS folder
        ss << "../../../tests/integration/inputs/FIMS-deterministic/om_input" << 1 << ".json";
        t.ReadJson(ss.str(), input);
        ss.str("");

        // Read in outputs
        ss << "../../../tests/integration/inputs/FIMS-deterministic/om_output" << 1 << ".json";
        t.ReadJson(ss.str(), output);

        // Declare singleton of population class
        fims::Population<double> pop;

        // ConfigurePopulationModel, RunModelLoop, and CheckModelOutput
        // methods are in integration_class.hpp
        good = t.ConfigurePopulationModel(pop, input, output);

        pop.numbers_at_age = t.RunModelLoop(pop, input);
        good = t.CheckModelOutput(pop, output);

        // declare unfished numbers at age 1, unfished spawning bimoass,
        // and unfished biomass
        std::vector<double> expected_unfished_numbers_at_age1(pop.nyears, 0.0);
        std::vector<double> expected_unfished_spawning_biomass(pop.nyears, 0.0);
        std::vector<double> expected_unfished_biomass(pop.nyears, 0.0);

        // declare vector of doubles to hold
        // biomass, spawning biomass, unfished spawning biomass,
        // expected catch in weight, expected index
        std::vector<double> expected_biomass(pop.nyears, 0.0);
        std::vector<double> expected_spawning_biomass(pop.nyears, 0.0);
        std::vector<double> expected_catch(pop.nyears, 0.0);
        std::vector<double> expected_index(pop.nyears, 0.0);

        // declare vector of doubles to hold dimension folded
        // numbers at age,
        // fishing mortality at age, and total mortality at age
        std::vector<double> expected_numbers_at_age(pop.nages * pop.nyears, 0.0);
        std::vector<double> expected_mortality_F(pop.nages * pop.nyears, 0.0);
        std::vector<double> expected_mortality_Z(pop.nages * pop.nyears, 0.0);

        // Test unfished numbers at age, unfished spawning biomass,
        // and unfished biomass
        it = input.FindMember("median_R0");
        rapidjson::Value &R_0 = (*it).value;
        double rzero = R_0[0].GetDouble();

        it = input.FindMember("Phi.0");
        rapidjson::Value &Phi0 = (*it).value;
        double phi_0 = Phi0[0].GetDouble();

        for (int year = 0; year < pop.nyears; year++)
        {
            for (int age = 0; age < pop.nages; age++)
            {
                int index_ya = year * pop.nages + age;

                // Expect FIMS value is greater than 0.0
                EXPECT_GT(pop.unfished_numbers_at_age[index_ya], 0.0)
                    << "differ at index " << index_ya << "; year " << year << "; age" << age;
                EXPECT_LE(pop.unfished_numbers_at_age[index_ya], rzero)
                    << "differ at index " << index_ya << "; year " << year << "; age" << age;

                EXPECT_GT(pop.unfished_spawning_biomass[year], 0.0)
                    << "differ at index " << index_ya << "; year " << year << "; age" << age;
                EXPECT_LE(pop.unfished_spawning_biomass[year], rzero * phi_0)
                    << "differ at index " << index_ya << "; year " << year << "; age" << age;

                EXPECT_GT(pop.unfished_biomass[year], 0.0)
                    << "differ at index " << index_ya << "; year " << year << "; age" << age;
            }
        }

        for (int age = 0; age < pop.nages; age++)
        {
            int index_ya = pop.nyears * pop.nages + age;
            EXPECT_GT(pop.unfished_numbers_at_age[index_ya], 0.0)
                << "differ at index " << index_ya << "; year " << pop.nyears + 1 << "; age" << age;
        }

        EXPECT_GT(pop.unfished_spawning_biomass[pop.nyears], 0.0)
            << "differ at year " << pop.nyears + 1;

        EXPECT_GT(pop.unfished_biomass[pop.nyears], 0.0)
            << "differ at year " << pop.nyears + 1;

        // Test spawning biomass
        // find the OM json member called "SSB"
        it = output.FindMember("SSB");

        if (it != output.MemberEnd())
        {
            rapidjson::Value &e = (*it).value;
            for (int year = 0; year < pop.nyears; year++)
            {
                expected_spawning_biomass[year] = e[year].GetDouble();
                // Expect the difference between FIMS value and the
                // expected value from the MCP OM
                // is less than 1.0 mt.
                EXPECT_NEAR(pop.spawning_biomass[year], expected_spawning_biomass[year], 1.0)
                    << "year " << year;
                // Expect the difference between FIMS value and the
                // expected value from the MCP OM
                // is less than 1.0% of the expected value.
                EXPECT_LE(std::abs(pop.spawning_biomass[year] - expected_spawning_biomass[year]) /
                              expected_spawning_biomass[year] * 100,
                          1.0)
                    << "year " << year;
                // Expect FIMS value is greater than 0.0
                EXPECT_GT(pop.spawning_biomass[year], 0.0)
                    << "year " << year;
            }
        }
        EXPECT_GT(pop.spawning_biomass[pop.nyears], 0.0)
            << "year " << pop.nyears + 1;

        // Test biomass
        // find the OM json member called "Biomass"
        it = output.FindMember("biomass.mt");

        if (it != output.MemberEnd())
        {
            rapidjson::Value &e = (*it).value;
            for (int year = 0; year < pop.nyears; year++)
            {
                expected_biomass[year] = e[year].GetDouble();

                EXPECT_NEAR(pop.biomass[year], expected_biomass[year], 2)
                    << "year " << year;
                // Expect the difference between FIMS value and the
                // expected value from the MCP OM
                // is less than 1.0% of the expected value.
                EXPECT_LE(std::abs(pop.biomass[year] - expected_biomass[year]) /
                              expected_biomass[year] * 100,
                          1.0)
                    << "year " << year;
                // Expect FIMS value is greater than 0.0
                EXPECT_GT(pop.biomass[year], 0.0)
                    << "year " << year;
            }
        }
        EXPECT_GT(pop.biomass[pop.nyears], 0.0)
            << "year " << pop.nyears + 1;

        // Test expected catch
        it = output.FindMember("L.mt");

        if (it != output.MemberEnd())
        {
            typename rapidjson::Document::MemberIterator fleet1;
            fleet1 = it->value.FindMember("fleet1");
            rapidjson::Value &fleet_catch = (*fleet1).value;
            for (int year = 0; year < pop.nyears; year++)
            {
                expected_catch[year] = fleet_catch[year].GetDouble();
                // Expect the difference between FIMS and OM is less than 1 mt
                EXPECT_NEAR(pop.fleets[0]->expected_catch[year], expected_catch[year], 1)
                    << "year " << year;
                // Expect the difference between FIMS value and the
                // expected value from the MCP OM
                // is less than 1.0% of the expected value.
                EXPECT_LE(std::abs(pop.fleets[0]->expected_catch[year] - expected_catch[year]) /
                              expected_catch[year] * 100,
                          1.0)
                    << "year " << year;
                // Expect FIMS value is greater than 0.0
                EXPECT_GT(pop.fleets[0]->expected_catch[year], 0.0)
                    << "year " << year;
                // Expect FIMS value = 0.0
                EXPECT_EQ(pop.fleets[1]->expected_catch[year], 0.0)
                    << "year " << year;
            }
        }

        // Test expected index
        it = output.FindMember("survey_q");
        typename rapidjson::Document::MemberIterator fleet2_q;
        fleet2_q = it->value.FindMember("survey1");
        rapidjson::Value &fleet_q = (*fleet2_q).value;

        it = output.FindMember("survey_index_biomass");

        if (it != output.MemberEnd())
        {
            typename rapidjson::Document::MemberIterator fleet2_index;
            fleet2_index = it->value.FindMember("survey1");
            rapidjson::Value &fleet_index = (*fleet2_index).value;

            for (int year = 0; year < pop.nyears; year++)
            {
                // Expect catchability of the fishing fleet = 1.0
                EXPECT_EQ(pop.fleets[0]->q[year], 1.0)
                    << "year " << year;
                // Expect expected index of the fishing fleet to be
                // greater than 0.0
                EXPECT_GT(pop.fleets[0]->expected_index[year], 0.0)
                    << "year " << year;

                expected_index[year] = fleet_index[year].GetDouble();
                
                EXPECT_NEAR(pop.fleets[1]->expected_index[year], expected_index[year], 0.0001)
                    << "year " << year;
                // Expect the difference between FIMS value and the
                // expected value from the MCP OM
                // is less than 5.0% of the expected value.
                EXPECT_LE(std::abs(pop.fleets[1]->expected_index[year] - expected_index[year]) /
                              expected_index[year] * 100,
                          5.0)
                    << "year " << year;
                
                // Do not use EXPECT_EQ to compare floats or doubles
                // Use EXPECT_NEAR here
                EXPECT_NEAR(pop.fleets[1]->q[year], fleet_q[0].GetDouble(), 1.0e-07)
                    << "year " << year;
                // Expect FIMS value is greater than 0.0
                EXPECT_GT(pop.fleets[1]->expected_index[year], 0.0)
                    << "year " << year;
            }
        }

        // Test numbers at age
        // find the OM json member called "N.age"
        it = output.FindMember("N.age");

        if (it != output.MemberEnd())
        {
            rapidjson::Value &e = (*it).value;
            for (int year = 0; year < pop.nyears; year++)
            {
                for (int age = 0; age < pop.nages; age++)
                {
                    int index_ya = year * pop.nages + age;
                    expected_numbers_at_age[index_ya] = e[year][age].GetDouble();
                    // Expect the difference between FIMS value and the
                    // expected value from the MCP OM
                    // is less than 1.0% of the expected value.
                    EXPECT_LE(std::abs(pop.numbers_at_age[index_ya] - expected_numbers_at_age[index_ya]) /
                                  expected_numbers_at_age[index_ya] * 100,
                              1.0)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                    // Expect the difference between FIMS value and the
                    // expected value from the MCP OM
                    // is less than 65 fish.
                    EXPECT_LE(std::abs(pop.numbers_at_age[index_ya] - expected_numbers_at_age[index_ya]),
                              65)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                    // Expect FIMS value is greater than 0.0
                    EXPECT_GT(pop.numbers_at_age[index_ya], 0.0)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                }
            }
        }

        // Test numbers at age in year pop.nyear+1
        for (int age = 0; age < pop.nages; age++)
        {
            int index_ya = pop.nyears * pop.nages + age;
            EXPECT_GT(pop.numbers_at_age[index_ya], 0.0)
                << "differ at index " << index_ya << "; year " << pop.nyears + 1 << "; age" << age;
        }

        // Test fishing mortality at age
        it = output.FindMember("FAA");

        if (it != output.MemberEnd())
        {
            rapidjson::Value &e = (*it).value;
            for (int year = 0; year < pop.nyears; year++)
            {
                for (int age = 0; age < pop.nages; age++)
                {
                    int index_ya = year * pop.nages + age;
                    expected_mortality_F[index_ya] = e[year][age].GetDouble();
                    // Expect the difference between FIMS value and the
                    // expected value from the MCP OM
                    // is less than 0.0001.
                    EXPECT_NEAR(pop.mortality_F[index_ya], expected_mortality_F[index_ya],
                                0.0001)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                    // Expect FIMS value >= 0.0
                    EXPECT_GE(pop.mortality_F[index_ya], 0.0)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                }
            }
        }

        // Test total mortality at age
        it = input.FindMember("M.age");
        // integration_test_log <<"test"<<std::endl;
        if (it != input.MemberEnd())
        {
            rapidjson::Value &e = (*it).value;
            for (int year = 0; year < pop.nyears; year++)
            {
                for (int age = 0; age < pop.nages; age++)
                {
                    int index_ya = year * pop.nages + age;
                    int index_ya2 = (year - 1) * pop.nages + (age - 1);
                    expected_mortality_Z[index_ya] = expected_mortality_F[index_ya] + e[age].GetDouble();

                    // Check numbers at age mismatch issue
                    // FIMS output
                    if (age < (pop.nages - 1)) // Ignore plus group
                    {
                        integration_test_log << "year " << year << " age " << age << " index_ya " << index_ya << " FIMS: " << pop.numbers_at_age[index_ya2] << "*exp(-" << pop.mortality_Z[index_ya2] << ")=" << pop.numbers_at_age[index_ya2] * exp(-pop.mortality_Z[index_ya2]) << " pop.numbers_at_age[index_ya] " << pop.numbers_at_age[index_ya] << " MCP OM: " << expected_numbers_at_age[index_ya2] << "*exp(-" << expected_mortality_Z[index_ya2] << ")=" << expected_numbers_at_age[index_ya2] * exp(-expected_mortality_Z[index_ya2]) << " expected_numbers_at_age[index_ya] " << expected_numbers_at_age[index_ya] << std::endl;
                    }

                    // R code to print MCP OM output
                    // ZAA <- matrix(NA, nrow = om_input$nyr, ncol = om_input$nages)
                    // for (year in 1 : om_input$nyr)
                    // {
                    //     for (age in 1 : om_input$nages)
                    //     {
                    //         ZAA[year, age] < -om_output$FAA[year, age] + om_input$M.age[age] if (year > 1 & age<om_input$nages & age> 1)
                    //         {
                    //             cat("Year", year - 1, "Age", age - 1,
                    //                 om_output$N.age[year - 1, age - 1], "x exp(-", ZAA[year - 1, age - 1], ")=",
                    //                 om_output$N.age[year - 1, age - 1] * exp(-ZAA[year - 1, age - 1]),
                    //                 "OM NAA:", om_output$N.age[year, age], "\n")
                    //         }
                    //     }
                    // }

                    // Expect the difference between FIMS value and the
                    // expected value from the MCP OM
                    // is less than 0.0001.
                    EXPECT_NEAR(pop.mortality_Z[index_ya], expected_mortality_Z[index_ya],
                                0.0001)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                    // Expect FIMS value is greater than 0.0
                    EXPECT_GT(pop.mortality_Z[index_ya], 0.0)
                        << "differ at index " << index_ya << "; year " << year << "; age" << age;
                }
            }
        }
    }
}