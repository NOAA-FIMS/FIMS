#ifndef FIMS_JSON_HPP
#define FIMS_JSON_HPP

/**
 * @file fims_json.hpp
 * @brief A simple JSON parsing and generation library.
 * @details This library provides classes and functions for parsing JSON
 * strings and generating JSON data structures.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#include <cctype>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

namespace fims {
class JsonValue;

/**
 * Alias for a JSON object, mapping strings to JSON values.
 */
using JsonObject = std::map<std::string, JsonValue>;

/**
 * Alias for a JSON array, containing a sequence of JSON values.
 */
using JsonArray = std::vector<JsonValue>;

/**
 * Represents different types of JSON values.
 */
enum JsonValueType {
  Null = 0, /**< Null JSON value. */
  Number,   /**< Numeric JSON value. */
  String,   /**< String JSON value. */
  Bool,     /**< Boolean JSON value. */
  Object,   /**< JSON object. */
  JArray    /**< JSON array. */
};

/**
 * Represents a JSON value.
 */
class JsonValue {
 public:
  /** Default constructor, initializes to Null value. */
  JsonValue() : type(JsonValueType::Null) {}

  /** Constructor for numeric JSON value (i.e., integer). */
  JsonValue(int num) : type(JsonValueType::Number), number(num) {}

  /** Constructor for numeric JSON value (i.e., double). */
  JsonValue(double num) : type(JsonValueType::Number), number(num) {}

  /** Constructor for string JSON value. */
  JsonValue(const std::string& str) : type(JsonValueType::String), str(str) {}

  /** Constructor for boolean JSON value. */
  JsonValue(bool b) : type(JsonValueType::Bool), boolean(b) {}

  /** Constructor for JSON object value. */
  JsonValue(const JsonObject& obj) : type(JsonValueType::Object), object(obj) {}

  /** Constructor for JSON array value. */
  JsonValue(const JsonArray& arr) : type(JsonValueType::JArray), array(arr) {}

  /** Get the type of the JSON value. */
  JsonValueType GetType() const { return type; }

  /** Get the numeric value as an integer. */
  int GetInt() const { return static_cast<int>(number); }

  /** Get the numeric value as a double. */
  double GetDouble() const { return number; }

  /** Get the string value. */
  const std::string& GetString() const { return str; }

  /** Get the boolean value. */
  bool GetBool() const { return boolean; }

  /** Get the JSON object. */
  JsonObject& GetObject() { return object; }

  /** Get the JSON array. */
  JsonArray& GetArray() { return array; }

 private:
  JsonValueType type; /**< Type of the JSON value. */
  double number;      /**< Numeric value. */
  std::string str;    /**< String value. */
  bool boolean;       /**< Boolean value. */
  JsonObject object;  /**< JSON object. */
  JsonArray array;    /**< JSON array. */
};

/**
 * Parses JSON strings and generates JSON values.
 */
class JsonParser {
 public:
  /** Parse a JSON string and return the corresponding JSON value. */
  JsonValue Parse(const std::string& json);
  /** Write a JSON value to a file. */
  void WriteToFile(const std::string& filename, JsonValue jsonValue);
  /** Display a JSON value to the standard output. */
  void Show(JsonValue jsonValue);

  /** Remove whitespace in JSON. */
  static std::string removeWhitespace(const std::string& input) {
    std::string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace),
                 result.end());
    return result;
  }

  /**
   * @brief Formats a JSON string.
   * @param json
   * @return
   */
  static std::string PrettyFormatJSON(const std::string& json) {
    std::string result;
    std::string input = JsonParser::removeWhitespace(json);
    int indentLevel = 0;
    bool inQuotes = false;

    for (size_t i = 0; i < input.size(); ++i) {
      char current = input[i];

      switch (current) {
        case '{':
        case '[':
          result += current;
          if (!inQuotes) {
            result += '\n';
            indentLevel++;
            result += std::string(indentLevel * 4, ' ');
          }
          break;

        case '}':
        case ']':
          if (!inQuotes) {
            result += '\n';
            indentLevel--;
            result += std::string(indentLevel * 4, ' ');
          }
          result += current;
          break;

        case ',':
          result += current;
          if (!inQuotes) {
            result += '\n';
            result += std::string(indentLevel * 4, ' ');
          }
          break;

        case ':':
          result += current;
          if (!inQuotes) result += " ";
          break;

        case '"':
          result += current;
          // Toggle inQuotes when we encounter a double-quote
          if (i == 0 || input[i - 1] != '\\') {
            inQuotes = !inQuotes;
          }
          break;

        default:
          result += current;
          break;
      }
    }
    return result;
  }

 private:
  /** Skip whitespace characters in the input string. */
  void SkipWhitespace();
  /** Parse a JSON value. */
  JsonValue ParseValue();
  /** Parse a numeric JSON value. */
  JsonValue ParseNumber();
  /** Parse a string JSON value. */
  JsonValue ParseString();
  /** Parse a boolean JSON value. */
  JsonValue ParseBool();
  /** Parse a null JSON value. */
  JsonValue ParseNull();
  /** Parse a JSON object. */
  JsonValue ParseObject();
  /** Parse a JSON array. */
  JsonValue ParseArray();
  /** Write a JSON value to an output file stream. */
  void WriteJsonValue(std::ofstream& outputFile, JsonValue jsonValue);
  /** Display a JSON value to an output stream. */
  void PrintJsonValue(std::ostream& outputFile, JsonValue jsonValue);
  /** Indentation helper for printing JSON values in an output file stream. */
  void Indent(std::ostream& outputFile, int level);
  /** Indentation helper for printing JSON values in an output stream. */
  void Indent(std::ofstream& outputFile, int level);

  std::string data; /**< Input JSON data. */
  size_t position;  /**< Current position in the data. */
};

}  // namespace fims
#endif
