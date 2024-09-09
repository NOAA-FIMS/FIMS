#include <sstream>
#include <fstream>
#include <iostream>

#ifndef STD_LIB
#define STD_LIB
#endif

#include "../../inst/include/population_dynamics/population/population.hpp"
#include "../../inst/include/utilities/fims_json.hpp"

class IntegrationTest {
public:
    uint32_t ncases_m = 10;
    uint32_t ninput_files_m = 160;
    bool print_statements = true;

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
                JsonValue input;
                JsonValue output;

                ss.str("");
                ss << "inputs/C" << i << "/om_input" << j + 1 << ".json";
                this->ReadJson(ss.str(), input);

                ss.str("");
                ss << "inputs/C" << i << "/om_output" << j + 1 << ".json";
                this->ReadJson(ss.str(), output);

                fims_popdy::Population<double> pop;

                if (!this->ConfigurePopulationModel(pop, input, output)) {
                    good = false;
                }

                if(good){
                this->RunModelLoop(pop, input);
                }else{
                  throw std::invalid_argument("model not good!");
                }

                //                if (!this->CheckModelOutput(pop, output)) {
                //                    good = false;
                //                }
            }
        }

        return good;
    }

    bool ReadJson(const std::string &path,
            JsonValue &result) {

        std::stringstream ss;
        std::ifstream infile;
        infile.open(path.c_str());


        ss.str("");
        while (infile.good()) {
            std::string line;
            std::getline(infile, line);
            ss << line << "\n";
        }
        if (print_statements) {
            std::cout << path << "\n";
            //            std::cout << ss.str() << "\n";
        }

        JsonParser parser;
        result = parser.Parse(ss.str());
        parser.WriteToFile("out.json", result);
        //        json_.Parse(ss.str().c_str());

        return true;
    }

    bool ConfigurePopulationModel(fims_popdy::Population<double> &pop,
            JsonValue &input,
            JsonValue &output) {

        std::cout << input.GetType() << "\n";
        size_t nfleets, nsurveys, nages, nyears;

        std::cout << input.GetDouble() << "\n";
        if (input.GetType() == JsonValueType::Object && output.GetType() == JsonValueType::Object) {

            JsonObject& obj = input.GetObject();
            JsonObject& obj2 = output.GetObject();

            typename JsonObject::iterator it;

            it = obj.find("nyr");
            if (it != obj.end()) {
                JsonValue e = (*it).second;
                if (e.GetType() == JsonValueType::Array) {
                    JsonArray a = e.GetArray();
                    nyears = a[0].GetInt();
                }


                if (print_statements) {
                    std::cout << "nyr " << nyears << std::endl;
                }
            } else {
                if (print_statements) {
                    std::cout << "nyr not found in input\n";
                }
            }

            //            typename JsonObject::iterator it;

            it = obj.find("nages");
            if (it != obj.end()) {
                JsonValue e = (*it).second;
                if (e.GetType() == JsonValueType::Array) {
                    JsonArray a = e.GetArray();
                    nages = a[0].GetInt();
                }


                if (print_statements) {
                    std::cout << "nages " << nages << std::endl;
                }
            } else {
                if (print_statements) {
                    std::cout << "nages not found in input\n";
                }
            }

            //get number of fleets
            it = obj.find("fleet_num");
            if (it != obj.end()) {
                JsonValue e = (*it).second;
                if (e.GetType() == JsonValueType::Array) {
                    JsonArray a = e.GetArray();
                    nfleets = a[0].GetInt();
                }

                if (print_statements) {
                    std::cout << "nfleets " << nfleets << std::endl;
                }

                for (size_t i = 0; i < nfleets; i++) {
                    std::shared_ptr<fims_popdy::Fleet<double> > f = std::make_shared<fims_popdy::Fleet<double> >();
                    f->log_q.resize(1);
                    f->Initialize(nyears, nages);
                  //  f->observed_index_data = std::make_shared<fims_data_object::DataObject<double> >(nyears);
                  //  f->observed_agecomp_data = std::make_shared<fims_data_object::DataObject<double> >(nyears, nages);

                    std::stringstream strs;
                    strs << "fleet" << i + 1;

                    it = obj.find("sel_fleet");
                    typename JsonObject::iterator fsel;
                    if (it != obj.end()) {
                        JsonValue e = (*it).second;
                        if (e.GetType() == JsonValueType::Object) {
                            JsonObject o = e.GetObject();
                            fsel = o.find(strs.str().c_str());

                            if ((*fsel).second.GetType() == JsonValueType::Object) {
                                JsonObject fsel_o = (*fsel).second.GetObject();
                                it = fsel_o.find("pattern");
                                if ((*it).second.GetType() == JsonValueType::Array) {

                                    JsonArray sel_pattern = (*it).second.GetArray();
                                    if (print_statements) {
                                        std::cout << "Selectivity:\n";
                                    }
                                    if (sel_pattern[0].GetInt() == 1) {//logistic
                                        if (print_statements) {
                                            std::cout << "logistic\n";
                                        }
                                        std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity = std::make_shared<fims_popdy::LogisticSelectivity<double> >();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point.resize(1);
                                            selectivity->inflection_point[0] = a50[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "A50 " << selectivity->inflection_point[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope.resize(1);
                                            selectivity->slope[0] = slope[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "slope " << selectivity->slope[0] << "\n";
                                            }
                                        }

                                        f->selectivity = selectivity;


                                    } else if (sel_pattern[0].GetInt() == 2) {//double logistic
                                        if (print_statements) {
                                            std::cout << "double logistic\n";
                                        }
                                        std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<double> > selectivity = std::make_shared<fims_popdy::DoubleLogisticSelectivity<double> >();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_asc.resize(1);
                                            selectivity->inflection_point_asc[0] = a50[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "A50 asc " << selectivity->inflection_point_asc[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope_asc.resize(1);
                                            selectivity->slope_asc[0] = slope[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "slope asc " << selectivity->slope_asc[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("A50.sel2");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_desc.resize(1);
                                            selectivity->inflection_point_desc[0] = a50[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "A50 desc " << selectivity->inflection_point_desc[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("slope.sel2");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope_desc.resize(1);
                                            selectivity->slope_desc[0] = slope[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "slope desc " << selectivity->slope_desc[0] << "\n";
                                            }
                                        }
                                        f->selectivity = selectivity;
                                    }

                                }

                            }
                        }

                    }


                    f->log_q[0] = 0.0;
                    it = obj.find("f");
                    if ((*it).second.GetType() == JsonValueType::Array) {
                        JsonArray f_values = (*it).second.GetArray();
                        for (int i = 0; i < f_values.size(); i++) {
                            f->Fmort[i] = f_values[i].GetDouble();
                            f->log_Fmort[i] = std::log(f_values[i].GetDouble());
                            if (print_statements) {
                                std::cout << f->Fmort[i] << " ";
                            }
                        }

                    }

                    if (print_statements) {
                        std::cout << "\n";
                    }
                    pop.fleets.push_back(f);
                }

            } else {
                if (print_statements) {
                    std::cout << "nfleets not found in input\n";
                }
            }

            it = obj.find("survey_num");
            if (it != obj.end()) {
                JsonValue e = (*it).second;
                if (e.GetType() == JsonValueType::Array) {
                    JsonArray a = e.GetArray();
                    nsurveys = a[0].GetInt();
                }

                if (print_statements) {
                    std::cout << "nsurveys " << nsurveys << std::endl;
                }

                for (size_t i = 0; i < nsurveys; i++) {
                    std::shared_ptr<fims_popdy::Fleet<double> > s = std::make_shared<fims_popdy::Fleet<double> >();
                    s->is_survey = true;
                    s->log_q.resize(1);
                    s->Initialize(nyears, nages);
                 //   s->observed_index_data = std::make_shared<fims_data_object::DataObject<double> >(nyears);
                  //  s->observed_agecomp_data = std::make_shared<fims_data_object::DataObject<double> >(nyears, nages);

                    std::stringstream strs;
                    strs << "survey" << i + 1;

                    it = obj.find("sel_survey");
                    typename JsonObject::iterator fsel;
                    if (it != obj.end()) {
                        JsonValue e = (*it).second;
                        if (e.GetType() == JsonValueType::Object) {
                            JsonObject o = e.GetObject();
                            fsel = o.find(strs.str().c_str());


                            if ((*fsel).second.GetType() == JsonValueType::Object) {


                                JsonObject fsel_o = (*fsel).second.GetObject();
                                it = fsel_o.find("pattern");
                                if ((*it).second.GetType() == JsonValueType::Array) {

                                    JsonArray sel_pattern = (*it).second.GetArray();

                                    if (sel_pattern[0].GetInt() == 1) {//logistic
                                        std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity = std::make_shared<fims_popdy::LogisticSelectivity<double> >();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point.resize(1);
                                            selectivity->inflection_point[0] = a50[0].GetDouble();
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope.resize(1);
                                            selectivity->slope[0] = slope[0].GetDouble();
                                        }

                                        s->selectivity = selectivity;



                                    } else if (sel_pattern[0].GetInt() == 2) {//double logistic
                                        std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<double> > selectivity = std::make_shared<fims_popdy::DoubleLogisticSelectivity<double> >();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_asc.resize(1);
                                            selectivity->inflection_point_asc[0] = a50[0].GetDouble();
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope_asc.resize(1);
                                            selectivity->slope_asc[0] = slope[0].GetDouble();
                                        }

                                        it = fsel_o.find("A50.sel2");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_desc.resize(1);
                                            selectivity->inflection_point_desc[0] = a50[0].GetDouble();
                                        }

                                        it = fsel_o.find("slope.sel2");
                                        if ((*it).second.GetType() == JsonValueType::Array) {
                                            JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope_desc.resize(1);
                                            selectivity->slope_desc[0] = slope[0].GetDouble();
                                        }
                                        s->selectivity = selectivity;
                                    }

                                }

                            }
                        }

                    }


                    s->log_q[0] = 0.0;
                    it = obj2.find("survey_q");


                    if ((*it).second.GetType() == JsonValueType::Object) {
                        //                        f->log_q = fims_math::log((*it).second.GetDouble());
                        JsonObject qobj = (*it).second.GetObject();

                        typename JsonObject::iterator qit = qobj.find("survey1");

                        if ((*qit).second.GetType() == JsonValueType::Array) {
                            JsonArray a = (*qit).second.GetArray();
                            s->log_q[0] = fims_math::log(a[0].GetDouble());
                            if (this->print_statements) {
                                std::cout << "q = " << a[0].GetDouble() << "\nlog(q) = " << s->log_q << "\n";
                            }
                        }
                    }



                    pop.fleets.push_back(s);

                }

            } else {
                if (print_statements) {
                    std::cout << "nsurveys not found in input\n";
                }
            }

            pop.nfleets = pop.fleets.size();

            // initialize population
            pop.numbers_at_age.resize((nyears + 1) * nages);
            pop.Initialize(nyears, 1, nages);

            // Set initial size to value from MCP C0
            it = obj2.find("N.age");
            if (it != obj2.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    JsonArray n = (*it).second.GetArray();
                    if (n[0].GetType() == JsonValueType::Array) {
                        JsonArray init_n = n[0].GetArray();
                        for (size_t i = 0; i < pop.nages; i++) {
                            pop.log_init_naa[i] = std::log(init_n[i].GetDouble());
                        }
                    }
                }
            }

            it = obj.find("ages");
            if (it != obj.end()) {

                if ((*it).second.GetType() == JsonValueType::Array) {
                    if (print_statements) {
                        std::cout << "ages: ";
                    }
                    JsonArray ages = (*it).second.GetArray();
                    for (int i = 0; i < ages.size(); i++) {
                        pop.ages[i] = ages[i].GetDouble();
                        if (print_statements) {
                            std::cout << pop.ages[i] << " ";
                        }
                    }
                    if (print_statements) {
                        std::cout << "\n";
                    }
                }


            } else {
                if (print_statements) {
                    std::cout << "ages not found in input\n";
                }
            }

            it = obj.find("year");
            if (it != obj.end()) {

                if ((*it).second.GetType() == JsonValueType::Array) {
                    if (print_statements) {
                        std::cout << "year: ";
                    }
                    JsonArray years = (*it).second.GetArray();
                    for (int i = 0; i < years.size(); i++) {
                        pop.years[i] = years[i].GetDouble();
                        if (print_statements) {
                            std::cout << pop.years[i] << " ";
                        }
                    }
                    if (print_statements) {
                        std::cout << "\n";
                    }
                }


            } else {
                if (print_statements) {
                    std::cout << "years not found in input\n";
                }
            }
            if (print_statements) {
                std::cout << "\nMortality:\n";
            }
            it = obj.find("M");
            if ((*it).second.GetType() == JsonValueType::Array) {
                JsonArray m = (*it).second.GetArray();
                double log_M = std::log(m[0].GetDouble());
                std::fill(pop.log_M.begin(), pop.log_M.end(), log_M);
                if (print_statements) {
                    std::cout << pop.log_M.size() << "\n";
                    std::cout << "log_M ";
                    for (size_t i = 0; i < pop.log_M.size(); i++) {
                        std::cout << pop.log_M[i] << " ";
                    }
                    std::cout << std::endl;
                }
            }


            // set recruitment
            std::shared_ptr<fims_popdy::SRBevertonHolt<double> > rec =
                    std::make_shared<fims_popdy::SRBevertonHolt<double> >();
            if (print_statements) {
                std::cout << "\nRecruitment:\n";
            }
            it = obj.find("R0");
            if (it != obj.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    double r0 = (*it).second.GetArray()[0].GetDouble();
                    rec->log_rzero.resize(1);
                    rec->log_rzero[0] = std::log(r0);
                    if (print_statements) {
                        std::cout << "R0 " << rec->log_rzero[0] << "| \n";
                    }
                }
            } else {
                if (print_statements) {
                    std::cout << "'R0' not found.\n";
                }
            }

            it = obj.find("h");
            if (it != obj.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    rec->logit_steep.resize(1);
                    rec->logit_steep[0] = fims_math::logit(0.2, 1.0, (*it).second.GetArray()[0].GetDouble());
                    if (print_statements) {
                        std::cout << "'h' " << rec->logit_steep[0] << " \n";
                    }
                }
            } else {
                if (print_statements) {
                    std::cout << "'h' not found.\n";
                }
            }

            it = obj.find("logR.resid");
            /*the log_recruit_dev vector does not include a value for year == 0
              and is of length nyears - 1 where the first position of the vector
              corresponds to the second year of the time series.*/
            rec->log_recruit_devs.resize(nyears);
            std::fill(rec->log_recruit_devs.begin(), rec->log_recruit_devs.end(), 0.0);
            if (it != obj.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    JsonArray rdev = (*it).second.GetArray();
                    if (print_statements) {
                        std::cout << "recruitment deviations: ";
                    }
                    for (size_t i = 0; i < rec->log_recruit_devs.size()-1; i++) {
                        rec->log_recruit_devs[i] = rdev[i+1].GetDouble();
                        if (print_statements) {
                            std::cout << rec->log_recruit_devs[i] << " ";
                        }
                    }
                    if (print_statements) {
                        std::cout << "\n";
                    }
                }
            } else {
                if (print_statements) {
                    std::cout << "'logR.resid' not found.\n";
                }
            }
            pop.recruitment = rec;

            // set maturity
            std::shared_ptr<fims_popdy::LogisticMaturity<double> > mat =
                    std::make_shared<fims_popdy::LogisticMaturity<double> >();

            if (print_statements) {
                std::cout << "\nMaturity:\n";
            }
            it = obj.find("A50.mat");
            if (it != obj.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    mat->inflection_point.resize(1);
                    mat->inflection_point[0] = (*it).second.GetArray()[0].GetDouble();
                    if (print_statements) {
                        std::cout << "inflection_point.mat " << mat->inflection_point[0] << " \n";
                    }
                }
            } else {
                if (print_statements) {
                    std::cout << "'A50.mat' not found.\n";
                }
            }

            pop.maturity = mat;

            it = obj.find("slope.mat");
            if (it != obj.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    mat->slope.resize(1);
                    mat->slope[0] = (*it).second.GetArray()[0].GetDouble();
                    if (print_statements) {
                        std::cout << "slope.mat " << mat->slope[0] << " \n";
                    }
                }
            } else {
                if (print_statements) {
                    std::cout << "'slope.mat' not found.\n";
                }
            }

            // set empirical growth
            std::shared_ptr<fims_popdy::EWAAgrowth<double> > growth = std::make_shared<fims_popdy::EWAAgrowth<double> >();
            std::cout << "Growth:\n";

            it = obj.find("W.kg");
            if (it != obj.end()) {
                if ((*it).second.GetType() == JsonValueType::Array) {
                    JsonArray wt = (*it).second.GetArray();
                    if (print_statements) {
                        std::cout << "W.kg: ";
                    }
                    for (size_t i = 0; i < pop.ages.size(); i++) {
                        growth->ewaa[static_cast<double> (pop.ages[i])] = wt[i].GetDouble() / 1000.0;
                        if (print_statements) {
                            std::cout << growth->ewaa[static_cast<double> (pop.ages[i])] << " ";
                        }
                    }
                    if (print_statements) {
                        std::cout << "\n";
                    }
                }
            } else {
                if (print_statements) {
                    std::cout << "'logR.resid' not found.\n";
                }
            }

            pop.growth = growth;

            return true;
        }

        return false;
    }

    std::vector<double> RunModelLoop(fims_popdy::Population<double> &pop,
            const JsonValue & input) {

        JsonObject output;
        JsonArray array;



        pop.Evaluate();

        if (print_statements) {
            std::cout << "Numbers at age:\n";
        }
        for (int i = 0; i < pop.nyears; i++) {
            for (int j = 0; j < pop.nages; j++) {
                if (print_statements) {
                    std::cout << pop.numbers_at_age[i * pop.nages + j] << " ";
                }
                array.push_back(pop.numbers_at_age[i * pop.nages + j]);
            }
            if (print_statements) {
                std::cout << std::endl;
            }
        }

        output["NumbersAtAge"] = array;

        if (print_statements) {
            std::cout << "\n\n"
                    << std::endl;
        }


        return pop.numbers_at_age;
    }

    bool CheckModelOutput(fims_popdy::Population<double> &pop,
            JsonValue &output) {
        return true;
    }
};
