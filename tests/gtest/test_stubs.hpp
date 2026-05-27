#ifndef FIMS_TEST_STUBS_HPP
#define FIMS_TEST_STUBS_HPP
/**
 * \file test_stubs.hpp
 * \brief Stubs for testing Rcpp interfaces in the FIMS framework.
 */
#include "def.hpp"
#include "fims_json.hpp"

namespace fims {

std::shared_ptr<FIMSLog> FIMSLog::fims_log = std::make_shared<FIMSLog>();

/**
 * Parse a JSON string and return the corresponding JSON value.
 * @param json The JSON string to parse.
 * @return The parsed JSON value.
 */
JsonValue JsonParser::Parse(const std::string &json) {
  data = json;
  position = 0;
  return ParseValue();
}

/**
 * @brief Skip the white space.
 *
 */
void JsonParser::SkipWhitespace() {
  while (position < data.size() && std::isspace(data[position])) {
    position++;
  }
}

/**
 * Parse a JSON value.
 *  @return The parsed JSON value.
 */
JsonValue JsonParser::ParseValue() {
  /** Skip whitespace characters in the input string. */
  SkipWhitespace();
  if (position >= data.size()) {
    return JsonValue();
  }

  char current = data[position];
  if (current == '{') {
    return ParseObject();
  } else if (current == '[') {
    return ParseArray();
  } else if (current == '"') {
    return ParseString();
  } else if (current == 't' || current == 'f') {
    return ParseBool();
  } else if (current == 'n') {
    return ParseNull();
  } else if (std::isdigit(current) || current == '-') {
    return ParseNumber();
  } else {
    // Unknown token.
    position++;
    return JsonValue();
  }
}

/**
 * Parse a numeric JSON value.
 * @return The parsed JSON value.
 */
JsonValue JsonParser::ParseNumber() {
  size_t end_pos = position;
  bool is_float = false;
  while (end_pos < data.size() &&
         (std::isdigit(data[end_pos]) || data[end_pos] == '.' ||
          data[end_pos] == '-' || data[end_pos] == 'e' ||
          data[end_pos] == 'E')) {
    if (data[end_pos] == '.' || data[end_pos] == 'e' || data[end_pos] == 'E') {
      is_float = true;
    }
    end_pos++;
  }

  std::string num_str = data.substr(position, end_pos - position);
  position = end_pos;

  if (is_float) {
    double num = 0.0;
    std::istringstream(num_str) >> num;
    return JsonValue(num);
  } else {
    int num = 0;
    std::istringstream(num_str) >> num;
    return JsonValue(num);
  }
}

/**
 * Parse a string JSON value.
 * @return The parsed JSON value.
 */
JsonValue JsonParser::ParseString() {
  position++;  // Skip the initial '"'
  size_t end_pos = data.find('"', position);
  if (end_pos == std::string::npos) {
    std::string str = data.substr(position);
    position = data.size();
    return JsonValue(str);
  }
  std::string str = data.substr(position, end_pos - position);
  position = end_pos + 1;
  return JsonValue(str);
}

/**
 * Parse a boolean JSON value.
 * @return The parsed JSON value.
 */
JsonValue JsonParser::ParseBool() {
  if (data.compare(position, 4, "true") == 0) {
    position += 4;
    return JsonValue(true);
  } else if (data.compare(position, 5, "false") == 0) {
    position += 5;
    return JsonValue(false);
  } else {
    // Invalid boolean value
    return JsonValue();
  }
}

/**
 * Parse a null JSON value.
 * @return The parsed JSON value.
 */
JsonValue JsonParser::ParseNull() {
  if (data.compare(position, 4, "null") == 0) {
    position += 4;
    return JsonValue();
  } else {
    // Invalid null value
    return JsonValue();
  }
}

/**
 * Parse a JSON object.
 * @return The parsed JSON value representing the object.
 */
JsonValue JsonParser::ParseObject() {
  JsonObject obj;
  position++;  // Skip the initial '{'

  SkipWhitespace();
  if (position < data.size() && data[position] == '}') {
    position++;  // Skip empty object close brace
    return JsonValue(obj);
  }

  while (position < data.size() && data[position] != '}') {
    SkipWhitespace();
    if (position >= data.size() || data[position] != '"') {
      return JsonValue(obj);
    }
    std::string key = ParseString().GetString();

    SkipWhitespace();
    if (position >= data.size() || data[position] != ':') {
      return JsonValue(obj);
    }
    position++;  // Skip the ':'
    SkipWhitespace();
    JsonValue value = ParseValue();
    obj[key] = value;

    SkipWhitespace();
    if (position < data.size() && data[position] == ',') {
      position++;
    }
  }

  if (position < data.size() && data[position] == '}') {
    position++;  // Skip the trailing '}'
  }
  return JsonValue(obj);
}

/**
 * Parse a JSON array.
 * @return The parsed JSON value representing the array.
 */
JsonValue JsonParser::ParseArray() {
  JsonArray arr;
  position++;  // Skip the initial '['

  SkipWhitespace();
  if (position < data.size() && data[position] == ']') {
    position++;  // Skip empty array close bracket
    return JsonValue(arr);
  }

  while (position < data.size() && data[position] != ']') {
    SkipWhitespace();
    JsonValue value = ParseValue();
    arr.push_back(value);

    SkipWhitespace();
    if (position < data.size() && data[position] == ',') {
      position++;
    }
  }

  if (position < data.size() && data[position] == ']') {
    position++;  // Skip the trailing ']'
  }
  return JsonValue(arr);
}

/**
 *  Write a JSON value to an output file.
 * @param filename The name of the output file.
 * @param jsonValue The JSON value to write.
 */
void JsonParser::WriteToFile(const std::string &filename, JsonValue jsonValue) {
  std::ofstream outputFile(filename);
  if (!outputFile) {
    std::cerr << "Error: Unable to open file " << filename << " for writing."
              << std::endl;
    return;
  }

  /** Call a private helper function to write JSON values recursively */
  WriteJsonValue(outputFile, jsonValue);
}

/**
 * Write a JSON value to an output file.
 * Private helper function to write JSON values recursively
 * @param outputFile The output file stream.
 *  @param jsonValue The JSON value to write.
 */
void JsonParser::WriteJsonValue(std::ofstream &outputFile,
                                JsonValue jsonValue) {
  switch (jsonValue.GetType()) {
    case JsonValueType::Null:
      outputFile << "null";
      break;
    case JsonValueType::Number:
      outputFile << jsonValue.GetDouble();
      break;
    case JsonValueType::String:
      outputFile << "\"" << jsonValue.GetString() << "\"";
      break;
    case JsonValueType::Bool:
      outputFile << (jsonValue.GetBool() ? "true" : "false");
      break;
    case JsonValueType::Object: {
      JsonObject &obj = jsonValue.GetObject();
      outputFile << "{";
      bool first = true;
      for (const auto &pair : obj) {
        if (!first) {
          outputFile << ",";
        }
        first = false;
        outputFile << "\"" << pair.first << "\":";
        WriteJsonValue(outputFile, pair.second);
      }
      outputFile << "}";
    } break;
    case JsonValueType::JArray: {
      JsonArray &arr = jsonValue.GetArray();
      outputFile << "[";
      bool first = true;
      for (const auto &value : arr) {
        if (!first) {
          outputFile << ",";
        }
        first = false;
        WriteJsonValue(outputFile, value);
      }
      outputFile << "]";
    } break;
  }
}

/**
 * Display a JSON value to the standard output.
 * @param jsonValue The JSON value to display.
 */
void JsonParser::Show(JsonValue jsonValue) {
  this->PrintJsonValue(std::cout, jsonValue);
  std::cout << std::endl;
}

/**
 * Display a JSON value to an output stream.
 * @param output The output stream.
 * @param jsonValue The JSON value to display.
 */
void JsonParser::PrintJsonValue(std::ostream &output, JsonValue jsonValue) {
  switch (jsonValue.GetType()) {
    case JsonValueType::Null:
      output << "null";
      break;
    case JsonValueType::Number:
      output << jsonValue.GetDouble();
      break;
    case JsonValueType::String:
      output << "\"" << jsonValue.GetString() << "\"";
      break;
    case JsonValueType::Bool:
      output << (jsonValue.GetBool() ? "true" : "false");
      break;
    case JsonValueType::Object: {
      JsonObject &obj = jsonValue.GetObject();
      output << "{";
      bool first = true;
      for (const auto &pair : obj) {
        if (!first) {
          output << ",";
        }
        first = false;
        output << "\"" << pair.first << "\":";
        PrintJsonValue(output, pair.second);
      }
      output << "}";
    } break;
    case JsonValueType::JArray: {
      JsonArray &arr = jsonValue.GetArray();
      output << "[";
      bool first = true;
      for (const auto &value : arr) {
        if (!first) {
          output << ",";
        }
        first = false;
        PrintJsonValue(output, value);
      }
      output << "]";
    } break;
  }
}

}  // namespace fims

#endif