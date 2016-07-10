#pragma once

#include <staticjson/basic.hpp>

#include <cstdio>
#include <string>

namespace staticjson
{

namespace nonpublic
{
    bool parse_json_string(const char* str, BaseHandler* handler, ParseStatus& result);
    bool parse_json_file(std::FILE* fp, BaseHandler* handler, ParseStatus& result);
    std::string serialize_json_string(const BaseHandler* handler);
    bool serialize_json_file(std::FILE* fp, const BaseHandler* handler);
    std::string serialize_pretty_json_string(const BaseHandler* handler);
    bool serialize_pretty_json_file(std::FILE* fp, const BaseHandler* handler);

    struct FileGuard : private NonMobile
    {
        std::FILE* fp;

        explicit FileGuard(std::FILE* fp) : fp(fp) {}
        ~FileGuard() { std::fclose(fp); }
    };
}

template <class T>
inline bool from_json_string(const char* str, T& value, ParseStatus& result)
{
    Handler<T> h(&value);
    return nonpublic::parse_json_string(str, &h, result);
}

template <class T>
inline bool from_json_file(std::FILE* fp, T& value, ParseStatus& result)
{
    Handler<T> h(&value);
    return nonpublic::parse_json_file(fp, &h, result);
}

template <class T>
inline bool from_json_file(const char* filename, T& value, ParseStatus& result)
{
    nonpublic::FileGuard fg(std::fopen(filename, "r"));
    return from_json_file(fg.fp, value, result);
}

template <class T>
inline std::string to_json_string(const T& value)
{
    Handler<T> h(const_cast<T&>(value));
    return nonpublic::serialize_json_string(h);
}

template <class T>
inline bool to_json_file(std::FILE* fp, T& value)
{
    Handler<T> h(const_cast<T&>(value));
    return nonpublic::serialize_json_file(fp, h);
}

template <class T>
inline bool to_json_file(const char* filename, T& value)
{
    nonpublic::FileGuard fg(std::fopen(filename, "wb"));
    return to_json_file(fg.fp, value);
}

template <class T>
inline std::string to_pretty_json_string(const T& value)
{
    Handler<T> h(const_cast<T&>(value));
    return nonpublic::serialize_pretty_json_string(h);
}

template <class T>
inline bool to_pretty_json_file(std::FILE* fp, T& value)
{
    Handler<T> h(const_cast<T&>(value));
    return nonpublic::serialize_pretty_json_file(fp, h);
}

template <class T>
inline bool to_pretty_json_file(const char* filename, T& value)
{
    nonpublic::FileGuard fg(std::fopen(filename, "wb"));
    return to_pretty_json_file(fg.fp, value);
}
}