#pragma once

#include <cstdio>
#include <staticjson/staticjson.hpp>
#include <string>

namespace autojsoncxx
{
using ParsingResult = staticjson::ParseStatus;

using staticjson::to_json_file;
using staticjson::to_json_string;
using staticjson::to_pretty_json_file;
using staticjson::to_pretty_json_string;

namespace error = staticjson::error;

template <class ValueType>
inline void to_json_string(std::string& output, const ValueType& value)
{
    output = to_json_string(value);
}

template <class ValueType>
inline void to_pretty_json_string(std::string& output, const ValueType& value)
{
    output = to_pretty_json_string(value);
}

template <class ValueType>
inline bool from_json_string(const char* str, ValueType& value, ParsingResult& err)
{
    return staticjson::from_json_string(str, &value, &err);
}

template <class ValueType>
inline bool from_json_string(const std::string& str, ValueType& value, ParsingResult& err)
{
    return staticjson::from_json_string(str.c_str(), &value, &err);
}

template <class ValueType>
inline bool from_json_file(const char* str, ValueType& value, ParsingResult& err)
{
    return staticjson::from_json_file(str, &value, &err);
}

template <class ValueType>
inline bool from_json_file(const std::string& str, ValueType& value, ParsingResult& err)
{
    return staticjson::from_json_file(str, &value, &err);
}

template <class ValueType>
inline bool from_json_file(std::FILE* fp, ValueType& value, ParsingResult& err)
{
    return staticjson::from_json_file(fp, &value, &err);
}

template <class T>
void to_document(const T& value, rapidjson::Document& doc)
{
    staticjson::to_json_document(&doc, value, nullptr);
}

template <class T>
bool from_document(T& value, const rapidjson::Document& doc, error::ErrorStack& errs)
{
    staticjson::ParseStatus status;
    bool rc = staticjson::from_json_document(doc, &value, &status);
    if (status.has_error())
        errs.swap(status.error_stack());
    return rc;
}
}
