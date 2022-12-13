
#include <sstream>
#include <ifstream>
#include <iostream>
#include "../../inst/include/population_dynamics/population/population.hpp"
#include "third_party/rapidjson/document.h"

class IntegrationTest {
public:

    uint32_t ninput_files;

    bool Run() {

        bool good = true;
        std::stringstream ss;
        for (uint32_t i = 0; i < this->ninput_files; i++) {
            ss << "inputs/om_input" << i << ".json";
            rapidjson::Document input;
            rapidjson::Document output;
            this->ReadJson(ss.str(), input)
            ss.str("");
            ss << "inputs/om_output" << i << ".json";
            this->ReadJson(ss.str(), output)
            
            fims::Population<double> pop;
            
            if(!this->ConfigureModel(pop, inout)){good = false};
            if(!this->RunModelLoop(pop, inout)){good = false};
            if(!this->CheckModelOutput(pop, inout)){good = false};

        }



        return true;
    }
    
    bool ReadJson(std::string& path,
                      rapidjson::Document& json_){
        
        std::stringstream ss;
        std::ifstream infile;
        infile.open(path.c_str());
        
        ss.str("");
        while (infile.good()) {
            std::string line;
            std::getline(config, line);
            ss << line << "\n";
        }
        
        json_.Parse(ss.str().c_str());
    }
    
    bool RunModelLoop(fims::Population<double>& pop,
                      rapidjson::Document& input){
        
    }
    
    bool ConfigureModel(fims::Population<double>& pop,
                        rapidjson::Document& input){
        
    }
    
    bool CheckModelOutput(fims::Population<double>& pop,
                        rapidjson::Document& output){
        
    }





};

int main() {

    return 0;
}
