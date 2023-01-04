
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
            for (uint32_t j = 0; j < 10; /*this->ninput_files_m;*/ j++) {
                ss.str("");
                ss << "inputs/C" << i << "/om_input" << j + 1 << ".json";
                rapidjson::Document input;
                rapidjson::Document output;
                this->ReadJson(ss.str(), input);


                ss.str("");
                ss << "inputs/C" << i << "/om_output" << j + 1 << ".json";
                // this->ReadJson(ss.str(), output);

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

            exit(0);
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

        typename rapidjson::Document::MemberIterator it;

        size_t nfleets, nsurveys, nages, nyears;


        //get number of years
        it = input.FindMember("nyr");
        if (it != input.MemberEnd()) {
            rapidjson::Value &e = (*it).value;
            nyears = e[0].GetInt();
            std::cout << "nyr " << nyears << std::endl;
        } else {
            std::cout << "nyr not found in input\n";
        }

        //get number of ages
        it = input.FindMember("nages");
        if (it != input.MemberEnd()) {
            rapidjson::Value &e = (*it).value;
            nages = e[0].GetInt();
            std::cout << "nages " << nages << std::endl;
        } else {
            std::cout << "nages not found in input\n";
        }

        //initialize population
        pop.Initialize(nyears, 1, nages);

        //temporary container for fleets and surveys
        std::map<uint32_t, std::shared_ptr<fims::Fleet<double> > > fleets;

        //get number of surveys
        it = input.FindMember("ages");
        if (it != input.MemberEnd()) {
            rapidjson::Value &e = (*it).value;
            std::cout << "ages ";
            for (int i = 0; i < e.Size(); i++) {
                pop.ages[i] = e[i].GetDouble();
                std::cout << pop.ages[i] << " ";
            }
            std::cout << std::endl;


        } else {
            std::cout << "ages not found in input\n";
        }

        //get number of fleets
        it = input.FindMember("fleet_num");
        if (it != input.MemberEnd()) {
            rapidjson::Value &e = (*it).value;
            nfleets = e[0].GetInt();
            std::cout << "nfleets " << nfleets << std::endl;
            for (int i = 0; i < nfleets; i++) {
                std::shared_ptr<fims::Fleet<double> > f = std::make_shared<fims::Fleet<double> >();
                f->Initialize(nyears, nages);
                f->observed_index_data = std::make_shared<fims::DataObject<double> >(nyears);
                f->observed_agecomp_data = std::make_shared<fims::DataObject<double> >(nyears, nages);
                std::shared_ptr<fims::LogisticSelectivity<double> > selectivity
                        = std::make_shared<fims::LogisticSelectivity<double> >();
                std::stringstream strs;
                strs << "fleet" << i + 1;

                it = input.FindMember("sel_fleet");
                typename rapidjson::Document::MemberIterator fsel;
                fsel = it->value.FindMember(strs.str().c_str());
                typename rapidjson::Document::MemberIterator sel_a50;
                sel_a50 = fsel->value.FindMember("A50.sel");
                rapidjson::Value &a50 = (*sel_a50).value;
                selectivity->median = a50[0].GetDouble();

                typename rapidjson::Document::MemberIterator sel_slope;
                sel_slope = fsel->value.FindMember("slope.sel");
                rapidjson::Value &slope = (*sel_slope).value;
                selectivity->slope = slope[0].GetDouble();
                std::cout << selectivity->median << " " << selectivity->slope << "\n";
                f->selectivity = selectivity;

                std::cout<<"f ";
                it = input.FindMember("f");
                for(int i = 0; i < it->value.Size(); i++){
                    rapidjson::Value &e = (*it).value;
                    f->Fmort[i] = e[i].GetDouble();
                    f->log_Fmort[i] = std::log(e[i].GetDouble());
                    std::cout<< f->log_Fmort[i]<<" ";
                }
                std::cout<<"\n";
                fleets[f->GetId()] = f;
            }
        } else {
            std::cout << "fleet_num not found in input\n";
        }

        //get number of surveys
        it = input.FindMember("survey_num");
        if (it != input.MemberEnd()) {
            rapidjson::Value &e = (*it).value;
            nsurveys = e[0].GetInt();
            std::cout << "survey_num " << nfleets << std::endl;
        } else {
            std::cout << "survey_num not found in input\n";
        }







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
