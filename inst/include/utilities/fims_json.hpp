/// @file fims_json.hpp
/// @brief A simple JSON parsing and generation library.
/// @details This library provides classes and functions for parsing JSON
/// strings and generating JSON data structures.
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class JsonValue;

/// Alias for a JSON object, mapping strings to JSON values.
using JsonObject = std::map<std::string, JsonValue>;

/// Alias for a JSON array, containing a sequence of JSON values.
using JsonArray = std::vector<JsonValue>;

/// Represents different types of JSON values.
enum JsonValueType
{
  Null = 0, ///< Null JSON value.
  Number,   ///< Numeric JSON value.
  String,   ///< String JSON value.
  Bool,     ///< Boolean JSON value.
  Object,   ///< JSON object.
  Array     ///< JSON array.
};

/// Represents a JSON value.
class JsonValue
{
public:
  /// Default constructor, initializes to Null value.
  JsonValue () : type (JsonValueType::Null) {}

  /// Constructor for numeric JSON value (i.e., integer).
  JsonValue (int num) : type (JsonValueType::Number), number (num) {}

  /// Constructor for numeric JSON value (i.e., double).
  JsonValue (double num) : type (JsonValueType::Number), number (num) {}

  /// Constructor for string JSON value.
  JsonValue (const std::string &str) : type (JsonValueType::String), str (str)
  {
  }

  /// Constructor for boolean JSON value.
  JsonValue (bool b) : type (JsonValueType::Bool), boolean (b) {}

  /// Constructor for JSON object value.
  JsonValue (const JsonObject &obj)
      : type (JsonValueType::Object), object (obj)
  {
  }

  /// Constructor for JSON array value.
  JsonValue (const JsonArray &arr) : type (JsonValueType::Array), array (arr)
  {
  }

  /// Get the type of the JSON value.
  JsonValueType
  GetType () const
  {
    return type;
  }

  /// Get the numeric value as an integer.
  int
  GetInt () const
  {
    return static_cast<int> (number);
  }

  /// Get the numeric value as a double.
  double
  GetDouble () const
  {
    return number;
  }

  /// Get the string value.
  const std::string &
  GetString () const
  {
    return str;
  }

  /// Get the boolean value.
  bool
  GetBool () const
  {
    return boolean;
  }

  /// Get the JSON object.
  JsonObject &
  GetObject ()
  {
    return object;
  }

  /// Get the JSON array.
  JsonArray &
  GetArray ()
  {
    return array;
  }

private:
  JsonValueType type; ///< Type of the JSON value.
  double number;      ///< Numeric value.
  std::string str;    ///< String value.
  bool boolean;       ///< Boolean value.
  JsonObject object;  ///< JSON object.
  JsonArray array;    ///< JSON array.
};

/// Parses JSON strings and generates JSON values.
class JsonParser
{
public:
  /// Parse a JSON string and return the corresponding JSON value.
  JsonValue Parse (const std::string &json);
  /// Write a JSON value to a file.
  void WriteToFile (const std::string &filename, JsonValue jsonValue);
  /// Display a JSON value to the standard output.
  void Show (JsonValue jsonValue);

private:
  /// Skip whitespace characters in the input string.
  void SkipWhitespace ();
  /// Parse a JSON value.
  JsonValue ParseValue ();
  /// Parse a numeric JSON value.
  JsonValue ParseNumber ();
  /// Parse a string JSON value.
  JsonValue ParseString ();
  /// Parse a boolean JSON value.
  JsonValue ParseBool ();
  /// Parse a null JSON value.
  JsonValue ParseNull ();
  /// Parse a JSON object.
  JsonValue ParseObject ();
  /// Parse a JSON array.
  JsonValue ParseArray ();
  /// Write a JSON value to an output file stream.
  void WriteJsonValue (std::ofstream &outputFile, JsonValue jsonValue);
  /// Display a JSON value to an output stream.
  void PrintJsonValue (std::ostream &outputFile, JsonValue jsonValue);
  /// Indentation helper for printing JSON values in an output file stream.
  void Indent (std::ostream &outputFile, int level);
  /// Indentation helper for printing JSON values in an output stream.
  void Indent (std::ofstream &outputFile, int level);

  std::string data; ///< Input JSON data.
  size_t position;  ///< Current position in the data.
};

/// Parse a JSON string and return the corresponding JSON value.
/// @param json The JSON string to parse.
/// @return The parsed JSON value.
JsonValue
JsonParser::Parse (const std::string &json)
{
  data = json;
  position = 0;
  return ParseValue ();
}

void
JsonParser::SkipWhitespace ()
{
  while (position < data.length () && std::isspace (data[position]))
    {
      position++;
    }
}

/// Parse a JSON value.
/// @return The parsed JSON value.
JsonValue
JsonParser::ParseValue ()
{
  /// Skip whitespace characters in the input string.
  SkipWhitespace ();
  char current = data[position];
  if (current == '{')
    {
      return ParseObject ();
    }
  else if (current == '[')
    {
      return ParseArray ();
    }
  else if (current == '"')
    {
      return ParseString ();
    }
  else if (current == 't' || current == 'f')
    {
      return ParseBool ();
    }
  else if (current == 'n')
    {
      return ParseNull ();
    }
  else
    {
      return ParseNumber ();
    }
}

/// Parse a numeric JSON value.
/// @return The parsed JSON value.
JsonValue
JsonParser::ParseNumber ()
{
  size_t end_pos = position;
  bool is_float = false;
  while (end_pos < data.length ()
         && (std::isdigit (data[end_pos]) || data[end_pos] == '.'
             || data[end_pos] == '-' || data[end_pos] == 'e'
             || data[end_pos] == 'E'))
    {
      if (data[end_pos] == '.' || data[end_pos] == 'e' || data[end_pos] == 'E')
        {
          is_float = true;
        }
      end_pos++;
    }

  std::string num_str = data.substr (position, end_pos - position);
  position = end_pos;

  if (is_float)
    {
      double num;
      std::istringstream (num_str) >> num;
      return JsonValue (num);
    }
  else
    {
      int num;
      std::istringstream (num_str) >> num;
      return JsonValue (num);
    }
}

/// Parse a string JSON value.
/// @return The parsed JSON value.
JsonValue
JsonParser::ParseString ()
{
  position++; // Skip the initial '"'
  size_t end_pos = data.find ('"', position);
  std::string str = data.substr (position, end_pos - position);
  position = end_pos + 1;
  return JsonValue (str);
}

/// Parse a boolean JSON value.
/// @return The parsed JSON value.
JsonValue
JsonParser::ParseBool ()
{
  if (data.compare (position, 4, "true") == 0)
    {
      position += 4;
      return JsonValue (true);
    }
  else if (data.compare (position, 5, "false") == 0)
    {
      position += 5;
      return JsonValue (false);
    }
  else
    {
      // Invalid boolean value
      return JsonValue ();
    }
}

/// Parse a null JSON value.
/// @return The parsed JSON value.
JsonValue
JsonParser::ParseNull ()
{
  if (data.compare (position, 4, "null") == 0)
    {
      position += 4;
      return JsonValue ();
    }
  else
    {
      // Invalid null value
      return JsonValue ();
    }
}

/// Parse a JSON object.
/// @return The parsed JSON value representing the object.
JsonValue
JsonParser::ParseObject ()
{
  JsonObject obj;
  position++; // Skip the initial '{'

  while (data[position] != '}')
    {
      SkipWhitespace ();
      std::string key = ParseString ().GetString ();

      position++; // Skip the ':'
      SkipWhitespace ();
      JsonValue value = ParseValue ();
      obj[key] = value;

      SkipWhitespace ();
      if (data[position] == ',')
        {
          position++;
        }
    }

  position++; // Skip the trailing '}'
  return JsonValue (obj);
}

/// Parse a JSON array.
/// @return The parsed JSON value representing the array.
JsonValue
JsonParser::ParseArray ()
{
  JsonArray arr;
  position++; // Skip the initial '['

  while (data[position] != ']')
    {
      SkipWhitespace ();
      JsonValue value = ParseValue ();
      arr.push_back (value);

      SkipWhitespace ();
      if (data[position] == ',')
        {
          position++;
        }
    }

  position++; // Skip the trailing ']'
  return JsonValue (arr);
}

/// Write a JSON value to an output file.
/// @param filename The name of the output file.
/// @param jsonValue The JSON value to write.
void
JsonParser::WriteToFile (const std::string &filename, JsonValue jsonValue)
{
  std::ofstream outputFile (filename);
  if (!outputFile)
    {
      std::cerr << "Error: Unable to open file " << filename << " for writing."
                << std::endl;
      return;
    }

  // Call a private helper function to write JSON values recursively
  WriteJsonValue (outputFile, jsonValue);
}

/// Write a JSON value to an output file.
// Private helper function to write JSON values recursively
/// @param outputFile The output file stream.
/// @param jsonValue The JSON value to write.
void
JsonParser::WriteJsonValue (std::ofstream &outputFile, JsonValue jsonValue)
{
  switch (jsonValue.GetType ())
    {
    case JsonValueType::Null:
      outputFile << "null";
      break;
    case JsonValueType::Number:
      outputFile << jsonValue.GetDouble ();
      break;
    case JsonValueType::String:
      outputFile << "\"" << jsonValue.GetString () << "\"";
      break;
    case JsonValueType::Bool:
      outputFile << (jsonValue.GetBool () ? "true" : "false");
      break;
    case JsonValueType::Object:
      {
        JsonObject &obj = jsonValue.GetObject ();
        outputFile << "{";
        bool first = true;
        for (const auto &pair : obj)
          {
            if (!first)
              {
                outputFile << ",";
              }
            first = false;
            outputFile << "\"" << pair.first << "\":";
            WriteJsonValue (outputFile, pair.second);
          }
        outputFile << "}";
      }
      break;
    case JsonValueType::Array:
      {
        JsonArray &arr = jsonValue.GetArray ();
        outputFile << "[";
        bool first = true;
        for (const auto &value : arr)
          {
            if (!first)
              {
                outputFile << ",";
              }
            first = false;
            WriteJsonValue (outputFile, value);
          }
        outputFile << "]";
      }
      break;
    }
}

/// Display a JSON value to the standard output.
/// @param jsonValue The JSON value to display.
void
JsonParser::Show (JsonValue jsonValue)
{
  this->PrintJsonValue (std::cout, jsonValue);
  std::cout << std::endl;
}

/// Display a JSON value to an output stream.
/// @param output The output stream.
/// @param jsonValue The JSON value to display.
void
JsonParser::PrintJsonValue (std::ostream &output, JsonValue jsonValue)
{
  switch (jsonValue.GetType ())
    {
    case JsonValueType::Null:
      output << "null";
      break;
    case JsonValueType::Number:
      output << jsonValue.GetDouble ();
      break;
    case JsonValueType::String:
      output << "\"" << jsonValue.GetString () << "\"";
      break;
    case JsonValueType::Bool:
      output << (jsonValue.GetBool () ? "true" : "false");
      break;
    case JsonValueType::Object:
      {
        JsonObject &obj = jsonValue.GetObject ();
        output << "{";
        bool first = true;
        for (const auto &pair : obj)
          {
            if (!first)
              {
                output << ",";
              }
            first = false;
            output << "\"" << pair.first << "\":";
            PrintJsonValue (output, pair.second);
          }
        output << "}";
      }
      break;
    case JsonValueType::Array:
      {
        JsonArray &arr = jsonValue.GetArray ();
        output << "[";
        bool first = true;
        for (const auto &value : arr)
          {
            if (!first)
              {
                output << ",";
              }
            first = false;
            PrintJsonValue (output, value);
          }
        output << "]";
      }
      break;
    }
}
