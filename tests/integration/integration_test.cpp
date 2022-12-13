
#include <sstream>
#include <fstream>
#include <iostream>
#include "../../inst/include/population_dynamics/population/population.hpp"
#include "third_party/rapidjson/document.h"

std::ofstream test_log("integration_test.log");


class IntegrationTest {
public:
    
    uint32_t ncases = 10;
    uint32_t ninput_files = 160;
    
    bool Run() {
        
        bool good = true;
        std::stringstream ss;
        for (uint32_t i = 0; i < this->ncases; i++) {
            for (uint32_t j = 0; j < this->ninput_files; j++) {
                ss.str("");
                ss << "inputs/C"<<i<<"/om_input" << j+1 << ".json";
                rapidjson::Document input;
                rapidjson::Document output;
                this->ReadJson(ss.str(), input);
                
                ss.str("");
                ss << "inputs/C"<<i<<"/om_output" << j+1 << ".json";
                this->ReadJson(ss.str(), output);
                
                fims::Population<double> pop;
                
                if(!this->ConfigureModel(pop, input)){good = false;};
                if(!this->RunModelLoop(pop, input)){good = false;};
                if(!this->CheckModelOutput(pop, output)){good = false;};
                
            }
        }
        
        
        return true;
    }
    
    bool ReadJson(const std::string& path,
                  rapidjson::Document& json_){
        
        std::stringstream ss;
        std::ifstream infile;
        infile.open(path.c_str());
        
        ss.str("");
        while (infile.good()) {
            std::string line;
            std::getline(infile, line);
            ss << line << "\n";
        }
        std::cout<<path<<"\n";
        std::cout<<ss.str()<<"\n";
        json_.Parse(ss.str().c_str());
        
        return true;
    }
    
    bool RunModelLoop(fims::Population<double>& pop,
                      rapidjson::Document& input){
        
        return true;
    }
    
    bool ConfigureModel(fims::Population<double>& pop,
                        rapidjson::Document& input){
        return true;
    }
    
    bool CheckModelOutput(fims::Population<double>& pop,
                          rapidjson::Document& output){
        return true;
    }
    
    
    
    
    
};

int main(int argc, char** argv) {
    
    IntegrationTest t;
    t.Run();
    return 0;
}
