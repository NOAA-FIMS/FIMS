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
                fims::JsonValue input;
                fims::JsonValue output;

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
            fims::JsonValue &result) {

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

        fims::JsonParser parser;
        result = parser.Parse(ss.str());
        parser.WriteToFile("out.json", result);
        //        json_.Parse(ss.str().c_str());

        return true;
    }

  void InitializeFleetObject(fims_popdy::Fleet<double>& fleet, size_t n_years,
                             size_t n_ages) {
    if (fleet.log_q.size() == 0) {
      fleet.log_q.resize(1);
      fleet.log_q[0] = static_cast<double>(0.0);
    }
    fleet.n_years = n_years;
    fleet.n_ages = n_ages;
    fleet.q.resize(fleet.log_q.size());
    fleet.log_Fmort.resize(n_years);
    fleet.Fmort.resize(n_years);
  }

  void InitializePopulationObject(fims_popdy::Population<double>& pop,
                                  size_t n_years, size_t n_ages,
                                  size_t n_fleets) {
    pop.n_years = n_years;
    pop.n_ages = n_ages;
    pop.n_fleets = n_fleets;

    // size all the vectors to length of n_ages
    pop.years.resize(n_years);
    pop.M.resize(n_years * n_ages);
    pop.ages.resize(n_ages);
    pop.log_init_naa.resize(n_ages);
    pop.log_M.resize(n_years * n_ages);
  }

    bool ConfigurePopulationModel(fims_popdy::Population<double> &pop,
            fims::JsonValue &input,
            fims::JsonValue &output) {

        std::cout << input.GetType() << "\n";
        size_t n_fleets, nsurveys, n_ages, n_years;

        std::cout << input.GetDouble() << "\n";
        if (input.GetType() == fims::JsonValueType::Object && output.GetType() == fims::JsonValueType::Object) {

            fims::JsonObject& obj = input.GetObject();
            fims::JsonObject& obj2 = output.GetObject();

            typename fims::JsonObject::iterator it;

            it = obj.find("nyr");
            if (it != obj.end()) {
                fims::JsonValue e = (*it).second;
                if (e.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray a = e.GetArray();
                    n_years = a[0].GetInt();
                }


                if (print_statements) {
                    std::cout << "nyr " << n_years << std::endl;
                }
            } else {
                if (print_statements) {
                    std::cout << "nyr not found in input\n";
                }
            }

            //            typename JsonObject::iterator it;

            it = obj.find("n_ages");
            if (it != obj.end()) {
                fims::JsonValue e = (*it).second;
                if (e.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray a = e.GetArray();
                    n_ages = a[0].GetInt();
                }


                if (print_statements) {
                    std::cout << "n_ages " << n_ages << std::endl;
                }
            } else {
                if (print_statements) {
                    std::cout << "n_ages not found in input\n";
                }
            }

            //get number of fleets
            it = obj.find("fleet_num");
            if (it != obj.end()) {
                fims::JsonValue e = (*it).second;
                if (e.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray a = e.GetArray();
                    n_fleets = a[0].GetInt();
                }

                if (print_statements) {
                    std::cout << "n_fleets " << n_fleets << std::endl;
                }

                for (size_t i = 0; i < n_fleets; i++) {
                    std::shared_ptr<fims_popdy::Fleet<double> > f = std::make_shared<fims_popdy::Fleet<double> >();
                    f->log_q.resize(1);
                    this->InitializeFleetObject(*f, n_years, n_ages);
                  //  f->observed_index_data = std::make_shared<fims_data_object::DataObject<double> >(n_years);
                  //  f->observed_agecomp_data = std::make_shared<fims_data_object::DataObject<double> >(n_years, n_ages);

                    std::stringstream strs;
                    strs << "fleet" << i + 1;

                    it = obj.find("sel_fleet");
                    typename fims::JsonObject::iterator fsel;
                    if (it != obj.end()) {
                        fims::JsonValue e = (*it).second;
                        if (e.GetType() == fims::JsonValueType::Object) {
                            fims::JsonObject o = e.GetObject();
                            fsel = o.find(strs.str().c_str());

                            if ((*fsel).second.GetType() == fims::JsonValueType::Object) {
                                fims::JsonObject fsel_o = (*fsel).second.GetObject();
                                it = fsel_o.find("pattern");
                                if ((*it).second.GetType() == fims::JsonValueType::JArray) {

                                    fims::JsonArray sel_pattern = (*it).second.GetArray();
                                    if (print_statements) {
                                        std::cout << "Selectivity:\n";
                                    }
                                    if (sel_pattern[0].GetInt() == 1) {//logistic
                                        if (print_statements) {
                                            std::cout << "logistic\n";
                                        }
                                        std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity = std::make_shared<fims_popdy::LogisticSelectivity<double> >();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point.resize(1);
                                            selectivity->inflection_point[0] = a50[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "A50 " << selectivity->inflection_point[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray slope = (*it).second.GetArray();
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
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_asc.resize(1);
                                            selectivity->inflection_point_asc[0] = a50[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "A50 asc " << selectivity->inflection_point_asc[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope_asc.resize(1);
                                            selectivity->slope_asc[0] = slope[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "slope asc " << selectivity->slope_asc[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("A50.sel2");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_desc.resize(1);
                                            selectivity->inflection_point_desc[0] = a50[0].GetDouble();
                                            if (print_statements) {
                                                std::cout << "A50 desc " << selectivity->inflection_point_desc[0] << "\n";
                                            }
                                        }

                                        it = fsel_o.find("slope.sel2");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray slope = (*it).second.GetArray();
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
                    if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                        fims::JsonArray f_values = (*it).second.GetArray();
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
                    std::cout << "n_fleets not found in input\n";
                }
            }

            it = obj.find("survey_num");
            if (it != obj.end()) {
                fims::JsonValue e = (*it).second;
                if (e.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray a = e.GetArray();
                    nsurveys = a[0].GetInt();
                }

                if (print_statements) {
                    std::cout << "nsurveys " << nsurveys << std::endl;
                }

                for (size_t i = 0; i < nsurveys; i++) {
                    std::shared_ptr<fims_popdy::Fleet<double> > s = std::make_shared<fims_popdy::Fleet<double> >();
                    s->log_q.resize(1);
                    this->InitializeFleetObject(*s, n_years, n_ages);
                 //   s->observed_index_data = std::make_shared<fims_data_object::DataObject<double> >(n_years);
                  //  s->observed_agecomp_data = std::make_shared<fims_data_object::DataObject<double> >(n_years, n_ages);

                    std::stringstream strs;
                    strs << "survey" << i + 1;

                    it = obj.find("sel_survey");
                    typename fims::JsonObject::iterator fsel;
                    if (it != obj.end()) {
                        fims::JsonValue e = (*it).second;
                        if (e.GetType() == fims::JsonValueType::Object) {
                            fims::JsonObject o = e.GetObject();
                            fsel = o.find(strs.str().c_str());


                            if ((*fsel).second.GetType() == fims::JsonValueType::Object) {


                                fims::JsonObject fsel_o = (*fsel).second.GetObject();
                                it = fsel_o.find("pattern");
                                if ((*it).second.GetType() == fims::JsonValueType::JArray) {

                                    fims::JsonArray sel_pattern = (*it).second.GetArray();

                                    if (sel_pattern[0].GetInt() == 1) {//logistic
                                        std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity = std::make_shared<fims_popdy::LogisticSelectivity<double> >();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point.resize(1);
                                            selectivity->inflection_point[0] = a50[0].GetDouble();
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope.resize(1);
                                            selectivity->slope[0] = slope[0].GetDouble();
                                        }

                    s->selectivity = selectivity;
                  } else if (sel_pattern[0].GetInt() == 2) {  // double logistic
                    std::shared_ptr<
                        fims_popdy::DoubleLogisticSelectivity<double>>
                        selectivity = std::make_shared<
                            fims_popdy::DoubleLogisticSelectivity<double>>();

                                        it = fsel_o.find("A50.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_asc.resize(1);
                                            selectivity->inflection_point_asc[0] = a50[0].GetDouble();
                                        }

                                        it = fsel_o.find("slope.sel1");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray slope = (*it).second.GetArray();
                                            selectivity->slope_asc.resize(1);
                                            selectivity->slope_asc[0] = slope[0].GetDouble();
                                        }

                                        it = fsel_o.find("A50.sel2");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray a50 = (*it).second.GetArray();
                                            selectivity->inflection_point_desc.resize(1);
                                            selectivity->inflection_point_desc[0] = a50[0].GetDouble();
                                        }

                                        it = fsel_o.find("slope.sel2");
                                        if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                                            fims::JsonArray slope = (*it).second.GetArray();
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


                    if ((*it).second.GetType() == fims::JsonValueType::Object) {
                        //                        f->log_q = fims_math::log((*it).second.GetDouble());
                        fims::JsonObject qobj = (*it).second.GetObject();

                        typename fims::JsonObject::iterator qit = qobj.find("survey1");

                        if ((*qit).second.GetType() == fims::JsonValueType::JArray) {
                            fims::JsonArray a = (*qit).second.GetArray();
                            s->log_q[0] = fims_math::log(a[0].GetDouble());
                            if (this->print_statements) {
                                std::cout << "q = " << a[0].GetDouble() << "\nlog(q) = " << s->log_q << "\n";
                            }
                        }
                    }

                    for (int i = 0; i < n_years; i++) {
                        s->log_Fmort[i] = -200.0;
                        s->Fmort[i] = std::exp(-200.0);
                        
                        if (print_statements) {
                            std::cout << s->Fmort[i] << " ";
                        }
                    }

                    pop.fleets.push_back(s);

                }

            } else {
                if (print_statements) {
                    std::cout << "nsurveys not found in input\n";
                }
            }

            pop.n_fleets = pop.fleets.size();

            // initialize population
      this->InitializePopulationObject(pop, n_years, n_ages, n_fleets);

            // Set initial size to value from MCP C0
            it = obj2.find("N.age");
            if (it != obj2.end()) {
                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray n = (*it).second.GetArray();
                    if (n[0].GetType() == fims::JsonValueType::JArray) {
                        fims::JsonArray init_n = n[0].GetArray();
                        for (size_t i = 0; i < pop.n_ages; i++) {
                            pop.log_init_naa[i] = std::log(init_n[i].GetDouble());
                        }
                    }
                }
            }

            it = obj.find("ages");
            if (it != obj.end()) {

                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                    if (print_statements) {
                        std::cout << "ages: ";
                    }
                    fims::JsonArray ages = (*it).second.GetArray();
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

                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                    if (print_statements) {
                        std::cout << "year: ";
                    }
                    fims::JsonArray years = (*it).second.GetArray();
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
            if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                fims::JsonArray m = (*it).second.GetArray();
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
            //set recruitment process
            std::shared_ptr<fims_popdy::LogDevs<double> > log_devs =
                        std::make_shared<fims_popdy::LogDevs<double> >();
            rec->process = log_devs;
            rec->process->recruitment = rec;
            if (print_statements) {
                std::cout << "\nRecruitment:\n";
            }
            it = obj.find("R0");
            if (it != obj.end()) {
                if ((*it).second.GetType() ==fims::JsonValueType::JArray) {
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
                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
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
              and is of length n_years - 1 where the first position of the vector
              corresponds to the second year of the time series.*/
            rec->log_recruit_devs.resize(n_years);           
            rec->log_expected_recruitment.resize(n_years+1);
            std::fill(rec->log_recruit_devs.begin(), rec->log_recruit_devs.end(), 0.0);
            std::fill(rec->log_expected_recruitment.begin(), rec->log_expected_recruitment.end(), 0.0);
            if (it != obj.end()) {
                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray rdev = (*it).second.GetArray();
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
                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
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
                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
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
            std::shared_ptr<fims_popdy::EWAAGrowth<double> > growth = std::make_shared<fims_popdy::EWAAGrowth<double> >();
            std::cout << "Growth:\n";

            it = obj.find("W.kg");
            if (it != obj.end()) {
                if ((*it).second.GetType() == fims::JsonValueType::JArray) {
                    fims::JsonArray wt = (*it).second.GetArray();
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
            const fims::JsonValue & input) {

        fims::JsonObject output;
        fims::JsonArray array;

    fims_popdy::CatchAtAge<double> model_loop;
    model_loop.AddPopulation(pop.id);
    model_loop.Initialize();
    model_loop.Evaluate();
 std::map<std::string, fims::Vector<Type>>& dq_ =
        this->GetPopulationDerivedQuantities(population->GetId());



        if (print_statements) {
            std::cout << "Numbers at age:\n";
        }
        for (int i = 0; i < pop.n_years; i++) {
            for (int j = 0; j < pop.n_ages; j++) {
                if (print_statements) {
                    std::cout << dq_["numbers_at_age"][i * pop.n_ages + j] << " ";
                }
                array.push_back(dq_["numbers_at_age"][i * pop.n_ages + j]);
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


        return dq_["numbers_at_age"];
    }

    bool CheckModelOutput(fims_popdy::Population<double> &pop,
            fims::JsonValue &output) {
        return true;
    }
};
