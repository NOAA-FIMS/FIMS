
#include <sstream>
#include <ifstream>
#include <iostream>
#include "../../inst/include/population_dynamics/population/population.hpp"
#include "third_party/rapidjson/document.h"

class IntegrationTest {
public:

    uint32_t ninput_files;

    bool Run() {

        std::stringstream ss;
        for (uint32_t i = 0; i < this->ninput_files; i++) {
            ss << "inputs/case" << i << ".json";
            rapidjson::Document input;
            std::ifstream infile;
            infile.open(ss.c_str());

            ss.str("");
            while (infile.good()) {
                std::string line;
                std::getline(config, line);
                ss << line << "\n";
            }
            
            input.Parse(ss.str().c_str());

        }



        return true;
    }





};

int main() {

    return 0;
}
