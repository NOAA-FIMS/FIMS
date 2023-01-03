
#include <sstream>
#include <fstream>
#include <iostream>
#include "../../inst/include/population_dynamics/population/population.hpp"
#include "third_party/rapidjson/document.h"

std::ofstream test_log("integration_test.log");

class IntegrationTest {
public:

    uint32_t ncases_m = 10;
    uint32_t ninput_files_m = 160;

    IntegrationTest() {
    }

    IntegrationTest(uint32_t ncases, uint32_t ninput_files)
    : ncases_m(ncases), ninput_files_m(ninput_files) {
    }

    bool Run() {

        bool good = true;
        std::stringstream ss;
        for (uint32_t i = 0; i < this->ncases_m; i++) {
            for (uint32_t j = 0; j < this->ninput_files_m; j++) {
                ss.str("");
                ss << "inputs/C" << i << "/om_input" << j + 1 << ".json";
                rapidjson::Document input;
                rapidjson::Document output;
                this->ReadJson(ss.str(), input);
                typename rapidjson::Document::MemberIterator it;
                std::cout<<ss.str()<<"\n";
                for (it = input.MemberBegin(); it != input.MemberEnd(); ++it) {
                    std::cout <<std::string(it->name.GetString())<<" = ";
                    rapidjson::Value &e = (*it).value;
                    std::cout<< e[0].GetInt()<<"\n";
                    
                }
                
                exit(0);

                ss.str("");
                ss << "inputs/C" << i << "/om_output" << j + 1 << ".json";
                this->ReadJson(ss.str(), output);

                fims::Population<double> pop;

                if (!this->ConfigurePopulationModel(pop, input)) {
                    good = false;
                };
                if (!this->RunModelLoop(pop, input)) {
                    good = false;
                };
                if (!this->CheckModelOutput(pop, output)) {
                    good = false;
                };

            }
        }


        return good;
    }

    bool ReadJson(const std::string& path,
            rapidjson::Document& json_) {

        std::stringstream ss;
        std::ifstream infile;
        infile.open(path.c_str());

        ss.str("");
        while (infile.good()) {
            std::string line;
            std::getline(infile, line);
            ss << line << "\n";
        }
        std::cout << path << "\n";
        std::cout << ss.str() << "\n";
        json_.Parse(ss.str().c_str());

        return true;
    }

    bool ConfigurePopulationModel(fims::Population<double>& pop,
            rapidjson::Document& input) {
        return true;
    }

    bool RunModelLoop(fims::Population<double>& pop,
            rapidjson::Document& input) {

        return true;
    }

    bool CheckModelOutput(fims::Population<double>& pop,
            rapidjson::Document& output) {
        return true;
    }





};

int main(int argc, char** argv) {

    IntegrationTest t(10, 160);
    t.Run();
    return 0;
}
