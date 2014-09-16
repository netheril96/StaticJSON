#ifndef AUTOJSONCXX_TO_JSON_HPP_29A4C106C1B1
#define AUTOJSONCXX_TO_JSON_HPP_29A4C106C1B1

#include <autojsoncxx/utility.hpp>
#include <autojsoncxx/base.hpp>
#include <autojsoncxx/error.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <cstdio>

namespace autojsoncxx {

template <class Writer, class ValueType>
inline void write_json(Writer& w, const ValueType& v)
{
    Serializer<Writer, ValueType>()(w, v);
}

template <class OutputStream, class ValueType>
inline void to_json(OutputStream& os, const ValueType& v)
{
    rapidjson::Writer<OutputStream> w(os);
    write_json(w, v);
}

template <class ValueType>
inline void to_json_string(std::string& str, const ValueType& v,
                           std::size_t BufferSize = utility::default_buffer_size)
{
    typedef rapidjson::StringBuffer OutputStreamType;
    OutputStreamType os(0, BufferSize);
    to_json(os, v);
    str.assign(os.GetString(), os.GetSize());
}

template <class ValueType>
inline void to_json_file(std::FILE* fp, const ValueType& v)
{
    char buffer[utility::default_buffer_size];
    rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
    to_json(os, v);
}

template <class ValueType>
inline bool to_json_file(const char* file_name, const ValueType& v)
{
    std::FILE* fp = std::fopen(file_name, "w");
    if (!fp)
        return false;
    to_json_file(fp, v);
    return fclose(fp) == 0;
}

template <class ValueType>
inline bool to_json_file(const std::string& file_name, const ValueType& v)
{
    return to_json_file(file_name.c_str(), v);
}

template <class OutputStream, class ValueType>
inline void to_pretty_json(OutputStream& os, const ValueType& v)
{
    rapidjson::PrettyWriter<OutputStream> w(os);
    write_json(w, v);
}

template <class ValueType>
inline void to_pretty_json_string(std::string& str, const ValueType& v,
                                  std::size_t BufferSize = utility::default_buffer_size)
{
    typedef rapidjson::StringBuffer OutputStreamType;
    OutputStreamType os(0, BufferSize);
    to_pretty_json(os, v);
    str.assign(os.GetString(), os.GetSize());
}

template <class ValueType>
inline void to_pretty_json_file(std::FILE* fp, const ValueType& v)
{
    char buffer[utility::default_buffer_size];
    rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
    to_pretty_json(os, v);
}

template <class ValueType>
inline bool to_pretty_json_file(const char* file_name, const ValueType& v)
{
    std::FILE* fp = std::fopen(file_name, "w");
    if (!fp)
        return false;
    to_pretty_json_file(fp, v);
    return fclose(fp) == 0;
}

template <class ValueType>
inline bool to_pretty_json_file(const std::string& file_name, const ValueType& v)
{
    return to_pretty_json_file(file_name.c_str(), v);
}
}

#endif
