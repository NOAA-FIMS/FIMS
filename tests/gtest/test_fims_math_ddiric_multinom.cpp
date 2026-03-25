/**
 * @file test_fims_math_ddiric_multinom.cpp
 * @brief Unit test for the Dirichlet-multinomial probability mass function.
 */

#include <gtest/gtest.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "common/fims_math.hpp"

namespace {

struct DirichletMultinomialTestCase {
    int case_id;
    std::vector<double> x;
    std::vector<double> p;
    double theta;
    double expected_log_prob;
    double expected_prob;
};

std::string Trim(const std::string& input) {
    std::size_t start = 0;
    while (start < input.size() &&
           std::isspace(static_cast<unsigned char>(input[start])) != 0) {
        ++start;
    }

    std::size_t end = input.size();
    while (end > start &&
           std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
        --end;
    }

    return input.substr(start, end - start);
}

std::vector<std::string> ParseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool in_quotes = false;

    for (char ch : line) {
        if (ch == '"') {
            in_quotes = !in_quotes;
        } else if (ch == ',' && !in_quotes) {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }

    fields.push_back(current);
    return fields;
}

std::vector<double> ParseRVector(const std::string& input) {
    std::string text = Trim(input);

    if (text.size() < 3 || text.rfind("c(", 0) != 0 || text.back() != ')') {
        throw std::runtime_error("Invalid R vector format: " + input);
    }

    text = text.substr(2, text.size() - 3);

    std::vector<double> values;
    std::stringstream ss(text);
    std::string token;
    while (std::getline(ss, token, ',')) {
        values.push_back(std::stod(Trim(token)));
    }

    return values;
}

std::ifstream OpenCsvFileOrThrow(const std::string& filename) {
    const std::vector<std::filesystem::path> candidate_paths = {
        std::filesystem::path(filename),
        std::filesystem::path(__FILE__).parent_path() / filename,
        std::filesystem::path("tests/gtest") / filename,
        std::filesystem::path("../tests/gtest") / filename,
        std::filesystem::path("../../tests/gtest") / filename
    };

    for (const auto& path : candidate_paths) {
        std::ifstream input(path);
        if (input.is_open()) {
            return input;
        }
    }

    throw std::runtime_error("Could not open CSV file: " + filename);
}

std::vector<DirichletMultinomialTestCase> ReadTestCasesFromCsv(
    const std::string& filename) {
    std::ifstream input = OpenCsvFileOrThrow(filename);

    std::string header;
    if (!std::getline(input, header)) {
        throw std::runtime_error("CSV file is empty: " + filename);
    }

    std::vector<DirichletMultinomialTestCase> test_cases;
    std::string line;
    while (std::getline(input, line)) {
        if (Trim(line).empty()) {
            continue;
        }

        const std::vector<std::string> fields = ParseCsvLine(line);
        if (fields.size() != 6) {
            throw std::runtime_error("Expected 6 CSV fields but found " +
                                     std::to_string(fields.size()) +
                                     " in line: " + line);
        }

        DirichletMultinomialTestCase test_case;
        test_case.case_id = std::stoi(Trim(fields[0]));
        test_case.x = ParseRVector(fields[1]);
        test_case.p = ParseRVector(fields[2]);
        test_case.theta = std::stod(Trim(fields[3]));
        test_case.expected_log_prob = std::stod(Trim(fields[4]));
        test_case.expected_prob = std::stod(Trim(fields[5]));

        test_cases.push_back(test_case);
    }

    return test_cases;
}

}  // namespace

TEST(FimsMathDdiricMultinomTest, UsesMultipleInputs) {
    const std::vector<DirichletMultinomialTestCase> test_cases =
        ReadTestCasesFromCsv("dirichlet_multinomial_test_values.csv");

    ASSERT_EQ(test_cases.size(), 5u);

    const double tolerance = 1e-6;

    for (const auto& test_case : test_cases) {
        ASSERT_EQ(test_case.x.size(), test_case.p.size())
            << "Mismatched vector lengths in case " << test_case.case_id;

        fims::Vector<double> x(test_case.x.size());
        fims::Vector<double> p(test_case.p.size());

        for (std::size_t i = 0; i < test_case.x.size(); ++i) {
            x[i] = test_case.x[i];
            p[i] = test_case.p[i];
        }

        const double theta = test_case.theta;
        const double expected_log_prob = test_case.expected_log_prob;
        const double expected_prob = test_case.expected_prob;

        const double result_log =
            fims_math::ddiric_multinom<double>(x, p, theta, 1);
        const double result_prob =
            fims_math::ddiric_multinom<double>(x, p, theta, 0);

        SCOPED_TRACE("case_id = " + std::to_string(test_case.case_id));
        EXPECT_NEAR(result_log, expected_log_prob, tolerance);
        EXPECT_NEAR(result_prob, expected_prob, tolerance);
    }
}
