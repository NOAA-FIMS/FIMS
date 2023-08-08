#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>

class JsonValue;

using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

enum JsonValueType {
    Null = 0,
    Number,
    String,
    Bool,
    Object,
    Array
};

class JsonValue {
public:

    JsonValue() : type(JsonValueType::Null) {
    }

    JsonValue(int num) : type(JsonValueType::Number), number(num) {
    }

    JsonValue(double num) : type(JsonValueType::Number), number(num) {
    }

    JsonValue(const std::string& str) : type(JsonValueType::String), str(str) {
    }

    JsonValue(bool b) : type(JsonValueType::Bool), boolean(b) {
    }

    JsonValue(const JsonObject& obj) : type(JsonValueType::Object), object(obj) {
    }

    JsonValue(const JsonArray& arr) : type(JsonValueType::Array), array(arr) {
    }

    JsonValueType GetType() const {
        return type;
    }

    int GetInt() const {
        return static_cast<int> (number);
    }

    double GetDouble() const {
        return number;
    }

    const std::string& GetString() const {
        return str;
    }

    bool GetBool() const {
        return boolean;
    }

    JsonObject& GetObject() {
        return object;
    }

    JsonArray& GetArray() {
        return array;
    }

private:
    JsonValueType type;
    double number;
    std::string str;
    bool boolean;
    JsonObject object;
    JsonArray array;
};

class JsonParser {
public:
    JsonValue Parse(const std::string& json);
    void WriteToFile(const std::string& filename, JsonValue jsonValue);
    void Show(JsonValue jsonValue);

private:
    void SkipWhitespace();
    JsonValue ParseValue();
    JsonValue ParseNumber();
    JsonValue ParseString();
    JsonValue ParseBool();
    JsonValue ParseNull();
    JsonValue ParseObject();
    JsonValue ParseArray();
    void WriteJsonValue(std::ofstream& outputFile, JsonValue jsonValue);
    void PrintJsonValue(std::ostream& outputFile, JsonValue jsonValue);
    void Indent(std::ostream& outputFile, int level);
    void Indent(std::ofstream& outputFile, int level);

    std::string data;
    size_t position;
};

JsonValue JsonParser::Parse(const std::string& json) {
    data = json;
    position = 0;
    return ParseValue();
}

void JsonParser::SkipWhitespace() {
    while (position < data.length() && std::isspace(data[position])) {
        position++;
    }
}

JsonValue JsonParser::ParseValue() {
    SkipWhitespace();
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
    } else {
        return ParseNumber();
    }
}

JsonValue JsonParser::ParseNumber() {
    size_t end_pos = position;
    bool is_float = false;
    while (end_pos < data.length() && (std::isdigit(data[end_pos]) || data[end_pos] == '.' || data[end_pos] == '-' || data[end_pos]
            == 'e' || data[end_pos] == 'E')) {
        if (data[end_pos] == '.' || data[end_pos] == 'e' || data[end_pos] == 'E') {
            is_float = true;
        }
        end_pos++;
    }

    std::string num_str = data.substr(position, end_pos - position);
    position = end_pos;

    if (is_float) {
        double num;
        std::istringstream(num_str) >> num;
        return JsonValue(num);
    } else {
        int num;
        std::istringstream(num_str) >> num;
        return JsonValue(num);
    }
}

JsonValue JsonParser::ParseString() {
    position++; // Skip the initial '"'
    size_t end_pos = data.find('"', position);
    std::string str = data.substr(position, end_pos - position);
    position = end_pos + 1;
    return JsonValue(str);
}

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

JsonValue JsonParser::ParseNull() {
    if (data.compare(position, 4, "null") == 0) {
        position += 4;
        return JsonValue();
    } else {
        // Invalid null value
        return JsonValue();
    }
}

JsonValue JsonParser::ParseObject() {
    JsonObject obj;
    position++; // Skip the initial '{'

    while (data[position] != '}') {
        SkipWhitespace();
        std::string key = ParseString().GetString();

        position++; // Skip the ':'
        SkipWhitespace();
        JsonValue value = ParseValue();
        obj[key] = value;

        SkipWhitespace();
        if (data[position] == ',') {
            position++;
        }
    }

    position++; // Skip the trailing '}'
    return JsonValue(obj);
}

JsonValue JsonParser::ParseArray() {
    JsonArray arr;
    position++; // Skip the initial '['

    while (data[position] != ']') {
        SkipWhitespace();
        JsonValue value = ParseValue();
        arr.push_back(value);

        SkipWhitespace();
        if (data[position] == ',') {
            position++;
        }
    }

    position++; // Skip the trailing ']'
    return JsonValue(arr);
}

void JsonParser::WriteToFile(const std::string& filename, JsonValue jsonValue) {
    std::ofstream outputFile(filename);
    if (!outputFile) {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    // Call a private helper function to write JSON values recursively
    WriteJsonValue(outputFile, jsonValue);
}

// Private helper function to write JSON values recursively

void JsonParser::WriteJsonValue(std::ofstream& outputFile, JsonValue jsonValue) {
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
        case JsonValueType::Object:
        {
            JsonObject& obj = jsonValue.GetObject();
            outputFile << "{";
            bool first = true;
            for (const auto& pair : obj) {
                if (!first) {
                    outputFile << ",";
                }
                first = false;
                outputFile << "\"" << pair.first << "\":";
                WriteJsonValue(outputFile, pair.second);
            }
            outputFile << "}";
        }
            break;
        case JsonValueType::Array:
        {
            JsonArray& arr = jsonValue.GetArray();
            outputFile << "[";
            bool first = true;
            for (const auto& value : arr) {
                if (!first) {
                    outputFile << ",";
                }
                first = false;
                WriteJsonValue(outputFile, value);
            }
            outputFile << "]";
        }
            break;
    }
}

void JsonParser::Show(JsonValue jsonValue){
    this->PrintJsonValue(std::cout, jsonValue);
    std::cout<<std::endl;
}

void JsonParser::PrintJsonValue(std::ostream& output, JsonValue jsonValue) {
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
        case JsonValueType::Object:
        {
            JsonObject& obj = jsonValue.GetObject();
            output << "{";
            bool first = true;
            for (const auto& pair : obj) {
                if (!first) {
                    output << ",";
                }
                first = false;
                output << "\"" << pair.first << "\":";
                PrintJsonValue(output, pair.second);
            }
            output << "}";
        }
            break;
        case JsonValueType::Array:
        {
            JsonArray& arr = jsonValue.GetArray();
            output << "[";
            bool first = true;
            for (const auto& value : arr) {
                if (!first) {
                    output << ",";
                }
                first = false;
                PrintJsonValue(output, value);
            }
            output << "]";
        }
            break;
    }
}

