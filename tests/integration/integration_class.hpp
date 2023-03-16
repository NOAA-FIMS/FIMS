#include <sstream>
#include <fstream>
#include <iostream>

#ifndef STD_LIB
#define STD_LIB
#endif

#include "../../inst/include/population_dynamics/population/population.hpp"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/writer.h"
#include "third_party/rapidjson/stringbuffer.h"

class IntegrationTest
{

public:
  uint32_t ncases_m = 10;
  uint32_t ninput_files_m = 160;
  bool print_statements = false;

  IntegrationTest()
  {
  }

  IntegrationTest(uint32_t ncases, uint32_t ninput_files)
      : ncases_m(ncases), ninput_files_m(ninput_files)
  {
  }

  bool Run()
  {

    bool good = true;

    std::stringstream ss;
    for (uint32_t i = 0; i < this->ncases_m; i++)
    {
      for (uint32_t j = 0; j < this->ninput_files_m; j++)
      {
        rapidjson::Document input;
        rapidjson::Document output;

        ss.str("");
        ss << "inputs/C" << i << "/om_input" << j + 1 << ".json";
        this->ReadJson(ss.str(), input);

        ss.str("");
        ss << "inputs/C" << i << "/om_output" << j + 1 << ".json";
        this->ReadJson(ss.str(), output);

        fims::Population<double> pop;

        if (!this->ConfigurePopulationModel(pop, input, output))
        {
          good = false;
        }

        this->RunModelLoop(pop, input);

        if (!this->CheckModelOutput(pop, output))
        {
          good = false;
        }
      }
    }

    return good;
  }

  bool ReadJson(const std::string &path,
                rapidjson::Document &json_)
  {

    std::stringstream ss;
    std::ifstream infile;
    infile.open(path.c_str());

    ss.str("");
    while (infile.good())
    {
      std::string line;
      std::getline(infile, line);
      ss << line << "\n";
    }
    if (print_statements)
    {
      std::cout << path << "\n";
      std::cout << ss.str() << "\n";
    }
    json_.Parse(ss.str().c_str());

    return true;
  }

  bool ConfigurePopulationModel(fims::Population<double> &pop,
                                rapidjson::Document &input,
                                rapidjson::Document &output)
  {

    typename rapidjson::Document::MemberIterator it;

    size_t nfleets, nsurveys, nages, nyears;

    // get number of years
    it = input.FindMember("nyr");
    if (it != input.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      nyears = e[0].GetInt();

      if (print_statements)
      {
        std::cout << "nyr " << nyears << std::endl;
      }
    }
    else
    {
      if (print_statements)
      {
        std::cout << "nyr not found in input\n";
      }
    }

    // get number of ages
    it = input.FindMember("nages");
    if (it != input.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      nages = e[0].GetInt();

      if (print_statements)
      {
        std::cout << "nages " << nages << std::endl;
      }
    }
    else
    {
      if (print_statements)
      {
        std::cout << "nages not found in input\n";
      }
    }

    // get number of fleets
    it = input.FindMember("fleet_num");
    if (it != input.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      nfleets = e[0].GetInt();
      bool parse_alternate_name = false;

      // instantiate fleets
      if (print_statements)
      {
        std::cout << "nfleets " << nfleets << std::endl;
      }
      for (int i = 0; i < nfleets; i++)
      {
        std::shared_ptr<fims::Fleet<double>> f = std::make_shared<fims::Fleet<double>>();
        f->Initialize(nyears, nages);
        f->observed_index_data = std::make_shared<fims::DataObject<double>>(nyears);
        f->observed_agecomp_data = std::make_shared<fims::DataObject<double>>(nyears, nages);

        std::stringstream strs;
        strs << "fleet" << i + 1;

        // set fleet selectivity
        it = input.FindMember("sel_fleet");
        typename rapidjson::Document::MemberIterator fsel;
        fsel = it->value.FindMember(strs.str().c_str());
        rapidjson::Value &ss = (*fsel).value;
        typename rapidjson::Document::MemberIterator pattern;
        pattern = fsel->value.FindMember("pattern");
        rapidjson::Value &sel_pattern = (*pattern).value;

        if (sel_pattern[0].GetDouble() == 1)
        { // logistic
          std::shared_ptr<fims::LogisticSelectivity<double>> selectivity = std::make_shared<fims::LogisticSelectivity<double>>();
          typename rapidjson::Document::MemberIterator sel_a50;
          sel_a50 = fsel->value.FindMember("A50.sel1");

          rapidjson::Value &a50 = (*sel_a50).value;
          selectivity->median = a50[0].GetDouble();

          typename rapidjson::Document::MemberIterator sel_slope;
          sel_slope = fsel->value.FindMember("slope.sel1");
          rapidjson::Value &slope = (*sel_slope).value;
          selectivity->slope = slope[0].GetDouble();
          f->selectivity = selectivity;
        }
        else if (sel_pattern[0].GetDouble() == 2)
        { // double logistic
          std::shared_ptr<fims::DoubleLogisticSelectivity<double>> selectivity = std::make_shared<fims::DoubleLogisticSelectivity<double>>();

          typename rapidjson::Document::MemberIterator sel_a50;
          sel_a50 = fsel->value.FindMember("A50.sel1");
          rapidjson::Value &a501 = (*sel_a50).value;
          selectivity->median_asc = a501[0].GetDouble();

          typename rapidjson::Document::MemberIterator sel_slope;
          sel_slope = fsel->value.FindMember("slope.sel1");
          rapidjson::Value &slope1 = (*sel_slope).value;
          selectivity->slope_asc = slope1[0].GetDouble();

          sel_a50 = fsel->value.FindMember("A50.sel2");
          rapidjson::Value &a502 = (*sel_a50).value;
          selectivity->median_desc = a502[0].GetDouble();

          sel_slope = fsel->value.FindMember("slope.sel2");
          rapidjson::Value &slope2 = (*sel_slope).value;
          selectivity->slope_desc = slope2[0].GetDouble();
          f->selectivity = selectivity;
        }

        // set fleet fishing mortality
        if (print_statements)
        {
          std::cout << "f ";
        }
        it = input.FindMember("f");
        for (int i = 0; i < it->value.Size(); i++)
        {
          rapidjson::Value &e = (*it).value;
          f->Fmort[i] = e[i].GetDouble();
          f->log_Fmort[i] = std::log(e[i].GetDouble());
          f->log_q[i] = 0.0;
          if (print_statements)
          {
            std::cout << f->log_Fmort[i] << " ";
          }
        }

        if (print_statements)
        {
          std::cout << "\n";
        }

        pop.fleets.push_back(f);
      }
    }
    else
    {
      if (print_statements)
      {
        std::cout << "fleet_num not found in input\n";
      }
    }

    // get number of surveys
    it = input.FindMember("survey_num");
    if (it != input.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      nsurveys = e[0].GetInt();
      for (int i = 0; i < nsurveys; i++)
      {
        std::shared_ptr<fims::Fleet<double>> s = std::make_shared<fims::Fleet<double>>();
        s->Initialize(nyears, nages);
        s->observed_index_data = std::make_shared<fims::DataObject<double>>(nyears);
        s->observed_agecomp_data = std::make_shared<fims::DataObject<double>>(nyears, nages);
        std::shared_ptr<fims::LogisticSelectivity<double>> selectivity = std::make_shared<fims::LogisticSelectivity<double>>();
        std::stringstream strs;
        strs << "survey" << i + 1;

        // set survey selectivity
        it = input.FindMember("sel_survey");
        typename rapidjson::Document::MemberIterator fsel;
        fsel = it->value.FindMember(strs.str().c_str());
        rapidjson::Value &ss = (*fsel).value;
        typename rapidjson::Document::MemberIterator pattern;
        pattern = fsel->value.FindMember("pattern");
        rapidjson::Value &sel_pattern = (*pattern).value;

        if (sel_pattern[0].GetDouble() == 1)
        { // logistic
          std::shared_ptr<fims::LogisticSelectivity<double>> selectivity = std::make_shared<fims::LogisticSelectivity<double>>();
          typename rapidjson::Document::MemberIterator sel_a50;
          sel_a50 = fsel->value.FindMember("A50.sel1");

          rapidjson::Value &a50 = (*sel_a50).value;
          selectivity->median = a50[0].GetDouble();

          typename rapidjson::Document::MemberIterator sel_slope;
          sel_slope = fsel->value.FindMember("slope.sel1");
          rapidjson::Value &slope = (*sel_slope).value;
          selectivity->slope = slope[0].GetDouble();

          s->selectivity = selectivity;
        }
        else if (sel_pattern[0].GetDouble() == 1)
        { // double logistic
          std::shared_ptr<fims::DoubleLogisticSelectivity<double>> selectivity = std::make_shared<fims::DoubleLogisticSelectivity<double>>();

          typename rapidjson::Document::MemberIterator sel_a50;
          sel_a50 = fsel->value.FindMember("A50.sel1");
          rapidjson::Value &a501 = (*sel_a50).value;
          selectivity->median_asc = a501[0].GetDouble();

          typename rapidjson::Document::MemberIterator sel_slope;
          sel_slope = fsel->value.FindMember("slope.sel1");
          rapidjson::Value &slope1 = (*sel_slope).value;
          selectivity->slope_asc = slope1[0].GetDouble();

          sel_a50 = fsel->value.FindMember("A50.sel2");
          rapidjson::Value &a502 = (*sel_a50).value;
          selectivity->median_desc = a502[0].GetDouble();

          sel_slope = fsel->value.FindMember("slope.sel2");
          rapidjson::Value &slope2 = (*sel_slope).value;
          selectivity->slope_desc = slope2[0].GetDouble();
          s->selectivity = selectivity;
        }

        // set survey catchability
        it = output.FindMember("survey_q");
        typename rapidjson::Document::MemberIterator fleet2_q;
        fleet2_q = it->value.FindMember("survey1");
        rapidjson::Value &fleet_q = (*fleet2_q).value;
        std::fill(s->log_q.begin(), s->log_q.end(), fims::log(fleet_q[0].GetDouble()));

        std::fill(s->log_Fmort.begin(), s->log_Fmort.end(), fims::log(0.0));
        pop.fleets.push_back(s);
      }
      if (print_statements)
      {
        std::cout << "survey_num " << nfleets << std::endl;
      }
    }
    else
    {
      if (print_statements)
      {
        std::cout << "survey_num not found in input\n";
      }
    }
    pop.nfleets = pop.fleets.size();

    // initialize population
    pop.Initialize(nyears, 1, nages);

    // Set initial size to value from MCP C0
    it = output.FindMember("N.age");
    if (it != output.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      std::vector<double> init_naa(pop.nages, 0.0);
      for (int age = 0; age < pop.nages; age++)
      {
        init_naa[age] = e[0][age].GetDouble();
        pop.log_init_naa[age] = std::log(init_naa[age]);
      }
    }

    // set ages vector
    it = input.FindMember("ages");
    if (it != input.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      if (print_statements)
      {
        std::cout << "ages ";
      }
      for (int i = 0; i < e.Size(); i++)
      {
        pop.ages[i] = e[i].GetDouble();
        if (print_statements)
        {
          std::cout << pop.ages[i] << " ";
        }
      }
      if (print_statements)
      {
        std::cout << std::endl;
      }
    }
    else
    {
      if (print_statements)
      {
        std::cout << "ages not found in input\n";
      }
    }

    // set years vector
    it = input.FindMember("year");
    if (it != input.MemberEnd())
    {
      rapidjson::Value &e = (*it).value;
      if (print_statements)
      {
        std::cout << "years ";
      }
      for (int i = 0; i < e.Size(); i++)
      {
        pop.years[i] = e[i].GetDouble();
        if (print_statements)
        {
          std::cout << pop.years[i] << " ";
        }
      }
      if (print_statements)
      {
        std::cout << std::endl;
      }
    }
    else
    {
      if (print_statements)
      {
        std::cout << "years not found in input\n";
      }
    }

    // set mortality vector
    it = input.FindMember("M");
    rapidjson::Value &e = (*it).value;
    double log_M = std::log(e[0].GetDouble());
    std::fill(pop.log_M.begin(), pop.log_M.end(), log_M);
    if (print_statements)
    {
      std::cout << "mortality: " << e[0].GetDouble() << std::endl;
    }
    // set recruitment
    std::shared_ptr<fims::SRBevertonHolt<double>> rec =
        std::make_shared<fims::SRBevertonHolt<double>>();
    it = input.FindMember("R0");
    e = (*it).value;
    rec->rzero = e[0].GetDouble();

    it = input.FindMember("h");
    e = (*it).value;
    rec->steep = e[0].GetDouble();

    it = input.FindMember("logR_sd");
    e = (*it).value;
    rec->log_sigma_recruit = e[0].GetDouble();
    it = input.FindMember("logR.resid");
    e = (*it).value;
    rec->recruit_deviations.resize(nyears + 1);
    std::fill(rec->recruit_deviations.begin(), rec->recruit_deviations.end(), 1.0);
    for (int i = 0; i < e.Size(); i++)
      {
        rec->recruit_deviations[i] = std::exp(e[i].GetDouble());
        if (print_statements)
        {
          std::cout << rec->recruit_deviations[i] << " ";
        }
      }
    rec->use_recruit_bias_adjustment = false;
    pop.recruitment = rec;

    // set maturity
    std::shared_ptr<fims::LogisticMaturity<double>> mat =
        std::make_shared<fims::LogisticMaturity<double>>();
    it = input.FindMember("A50.mat");
    e = (*it).value;
    mat->median = e[0].GetDouble();

    it = input.FindMember("slope.mat");
    e = (*it).value;
    mat->slope = e[0].GetDouble();
    pop.maturity = mat;

    // set empirical growth
    std::shared_ptr<fims::EWAAgrowth<double>> growth = std::make_shared<fims::EWAAgrowth<double>>();
    it = input.FindMember("W.kg");
    e = (*it).value;
    for (int i = 0; i < e.Size(); i++)
    {
      growth->ewaa[static_cast<double>(pop.ages[i])] = e[i].GetDouble() / 1000.0;
    }
    pop.growth = growth;

    return true;
  }

  std::vector<double> RunModelLoop(fims::Population<double> &pop,
                                   rapidjson::Document &input)
  {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    rapidjson::Document output;

    // define the document as an object rather than an array
    output.SetObject();

    // create a rapidjson array type with similar syntax to std::vector
    rapidjson::Value array(rapidjson::kArrayType);

    // must pass an allocator when the object may need to allocate memory
    rapidjson::Document::AllocatorType &allocator = output.GetAllocator();

    // create a rapidjson object type
    // rapidjson::Value object(rapidjson::kObjectType);
    // object.AddMember("NumbersAtAge", "50", allocator);

    pop.Evaluate();
    if (print_statements)
    {
      std::cout << "Numbers at age:\n";
    }
    for (int i = 0; i < pop.nyears; i++)
    {
      for (int j = 0; j < pop.nages; j++)
      {
        if (print_statements)
        {
          std::cout << pop.numbers_at_age[i * pop.nages + j] << " ";
        }
        array.PushBack(pop.numbers_at_age[i * pop.nages + j], allocator);
      }
      if (print_statements)
      {
        std::cout << std::endl;
      }
    }
    if (print_statements)
    {
      std::cout << "\n\n"
                << std::endl;
    }

    output.Accept(writer);
    if (print_statements)
    {
      std::cout << buffer.GetString() << std::endl;
    }
    return pop.numbers_at_age;
  }

  bool CheckModelOutput(fims::Population<double> &pop,
                        rapidjson::Document &output)
  {
    return true;
  }
};