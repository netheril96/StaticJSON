#include <staticjson/staticjson.hpp>

#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>

#include <cstdio>

namespace staticjson
{
// Adapted from Jettison's implementation (http://jettison.codehaus.org/)
// Original copyright (compatible with MIT):

// Copyright 2006 Envoi Solutions LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
inline std::string quote(const std::string& str)
{
    std::string sb;
    sb.reserve(str.size() + 8);
    sb += '\"';

    typedef std::string::const_iterator iterator;

    for (iterator it = str.begin(), end = str.end(); it != end; ++it)
    {
        char c = *it;
        switch (c)
        {
        case '\\':
        case '"':
            sb += '\\';
            sb += c;
            break;
        case '\b':
            sb.append("\\b", 2);
            break;
        case '\t':
            sb.append("\\t", 2);
            break;
        case '\n':
            sb.append("\\n", 2);
            break;
        case '\f':
            sb.append("\\f", 2);
            break;
        case '\r':
            sb.append("\\r", 2);
            break;
        case '\x00':
            sb.append("\\x00", 4);
            break;
        case '\x01':
            sb.append("\\x01", 4);
            break;
        case '\x02':
            sb.append("\\x02", 4);
            break;
        case '\x03':
            sb.append("\\x03", 4);
            break;
        case '\x04':
            sb.append("\\x04", 4);
            break;
        case '\x05':
            sb.append("\\x05", 4);
            break;
        case '\x06':
            sb.append("\\x06", 4);
            break;
        case '\x07':
            sb.append("\\x07", 4);
            break;
        case '\x0b':
            sb.append("\\x0b", 4);
            break;
        case '\x0e':
            sb.append("\\x0e", 4);
            break;
        case '\x0f':
            sb.append("\\x0f", 4);
            break;
        case '\x10':
            sb.append("\\x10", 4);
            break;
        case '\x11':
            sb.append("\\x11", 4);
            break;
        case '\x12':
            sb.append("\\x12", 4);
            break;
        case '\x13':
            sb.append("\\x13", 4);
            break;
        case '\x14':
            sb.append("\\x14", 4);
            break;
        case '\x15':
            sb.append("\\x15", 4);
            break;
        case '\x16':
            sb.append("\\x16", 4);
            break;
        case '\x17':
            sb.append("\\x17", 4);
            break;
        case '\x18':
            sb.append("\\x18", 4);
            break;
        case '\x19':
            sb.append("\\x19", 4);
            break;
        case '\x1a':
            sb.append("\\x1a", 4);
            break;
        case '\x1b':
            sb.append("\\x1b", 4);
            break;
        case '\x1c':
            sb.append("\\x1c", 4);
            break;
        case '\x1d':
            sb.append("\\x1d", 4);
            break;
        case '\x1e':
            sb.append("\\x1e", 4);
            break;
        case '\x1f':
            sb.append("\\x1f", 4);
            break;
        default:
            sb += c;
        }
    }
    sb += '\"';
    return sb;
}

std::string error::Success::description() const { return "No error"; }

std::string error::ObjectMemberError::description() const
{
    return "Error at object memeber with name " + quote(member_name());
}

std::string error::ArrayElementError::description() const
{
    return "Error at array element at index " + std::to_string(index());
}

std::string error::RequiredFieldMissingError::description() const
{
    std::string result = "Missing required field(s): ";
    bool first = true;
    for (auto&& name : missing_members())
    {
        if (!first)
        {
            result += ", ";
        }
        first = false;
        result += quote(name);
    }
    return result;
}

std::string error::NumberOutOfRangeError::description() const
{
    return "Number out of range: expected type " + quote(expected_type())
        + " but the type needed is " + quote(actual_type());
}

std::string error::TypeMismatchError::description() const
{
    return "Type mismatch between expected type " + quote(expected_type()) + " and actual type "
        + quote(actual_type());
}

std::string error::DuplicateKeyError::description() const
{
    return "Duplicate key in uniquely keyed map type: " + quote(key());
}

std::string error::UnknownFieldError::description() const
{
    return "Unknown field with name: " + quote(field_name());
}

std::string error::CorruptedDOMError::description() const { return "JSON has invalid structure"; }

std::string ParseStatus::description() const
{
    std::string res;
    if (has_error())
    {
        res = "Parsing failed at offset ";
        res += std::to_string(m_offset);
        res += " with error code ";
        res += std::to_string(m_code);
        res += ":\n";
        res += rapidjson::GetParseError_En(static_cast<rapidjson::ParseErrorCode>(m_code));
        res += '\n';

        if (m_stack)
        {
            res += "\nTraceback (last call first)\n";

            for (auto&& err : m_stack)
            {
                res += "* ";
                res += err.description();
                res += '\n';
            }
        }
    }
    return res;
}

IHandler::~IHandler() {}

BaseHandler::~BaseHandler() {}

bool BaseHandler::set_out_of_range(const char* actual_type)
{
    the_error.reset(new error::NumberOutOfRangeError(type_name(), actual_type));
    return false;
}

bool BaseHandler::set_type_mismatch(const char* actual_type)
{
    the_error.reset(new error::TypeMismatchError(type_name(), actual_type));
    return false;
}

ObjectHandler::ObjectHandler() {}

ObjectHandler::~ObjectHandler() {}

std::string ObjectHandler::type_name() const { return "object"; }

bool ObjectHandler::precheck(const char* actual_type)
{
    if (depth <= 0)
    {
        the_error.reset(new error::TypeMismatchError(type_name(), actual_type));
        return false;
    }
    if (!(flags & Flags::AllowDuplicateKey) && current && current->handler
        && current->handler->is_parsed())
    {
        the_error.reset(new error::DuplicateKeyError(current_name));
        return false;
    }
    return true;
}

bool ObjectHandler::postcheck(bool success)
{
    if (!success)
    {
        the_error.reset(new error::ObjectMemberError(current_name));
    }
    return success;
}

void ObjectHandler::set_missing_required(const std::string& name)
{
    if (!the_error || the_error->type() != error::MISSING_REQUIRED)
        the_error.reset(new error::RequiredFieldMissingError());

    std::vector<std::string>& missing
        = static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

    missing.push_back(name);
}

#define POSTCHECK(x) (!current || !(current->handler) || postcheck(x))

bool ObjectHandler::Double(double value)
{
    if (!precheck("double"))
        return false;
    return POSTCHECK(current->handler->Double(value));
}

bool ObjectHandler::Int(int value)
{
    if (!precheck("int"))
        return false;
    return POSTCHECK(current->handler->Int(value));
}

bool ObjectHandler::Uint(unsigned value)
{
    if (!precheck("unsigned"))
        return false;
    return POSTCHECK(current->handler->Uint(value));
}

bool ObjectHandler::Bool(bool value)
{
    if (!precheck("bool"))
        return false;
    return POSTCHECK(current->handler->Bool(value));
}

bool ObjectHandler::Int64(std::int64_t value)
{
    if (!precheck("std::int64_t"))
        return false;
    return POSTCHECK(current->handler->Int64(value));
}

bool ObjectHandler::Uint64(std::uint64_t value)
{
    if (!precheck("std::uint64_t"))
        return false;
    return POSTCHECK(current->handler->Uint64(value));
}

bool ObjectHandler::Null()
{
    if (!precheck("null"))
        return false;
    return POSTCHECK(current->handler->Null());
}

bool ObjectHandler::StartArray()
{
    if (!precheck("array"))
        return false;
    return POSTCHECK(current->handler->StartArray());
}

bool ObjectHandler::EndArray(SizeType sz)
{
    if (!precheck("array"))
        return false;
    return POSTCHECK(current->handler->EndArray(sz));
}

bool ObjectHandler::String(const char* str, SizeType sz, bool copy)
{
    if (!precheck("string"))
        return false;
    return POSTCHECK(current->handler->String(str, sz, copy));
}

bool ObjectHandler::Key(const char* str, SizeType sz, bool copy)
{
    if (depth <= 0)
    {
        the_error.reset(new error::CorruptedDOMError());
        return false;
    }
    if (depth == 1)
    {
        current_name.assign(str, sz);
        auto it = internals.find(current_name);
        if (it == internals.end())
        {
            current = nullptr;
            if ((flags & Flags::DisallowUnknownKey))
            {
                the_error.reset(new error::UnknownFieldError(str, sz));
                return false;
            }
        }
        else if (it->second.flags & Flags::IgnoreRead)
        {
            current = nullptr;
        }
        else
        {
            current = &it->second;
        }
        return true;
    }
    else
    {
        return POSTCHECK(current->handler->Key(str, sz, copy));
    }
}

bool ObjectHandler::StartObject()
{
    ++depth;
    if (depth > 1)
    {
        return POSTCHECK(current->handler->StartObject());
    }
    return true;
}

bool ObjectHandler::EndObject(SizeType sz)
{
    --depth;
    if (depth > 0)
    {
        return POSTCHECK(current->handler->EndObject(sz));
    }
    for (auto&& pair : internals)
    {
        if (pair.second.handler && !(pair.second.flags & Flags::Optional)
            && !pair.second.handler->is_parsed())
        {
            set_missing_required(pair.first);
        }
    }
    if (!the_error)
    {
        this->parsed = true;
        return true;
    }
    return false;
}

void ObjectHandler::reset()
{
    current = nullptr;
    current_name.clear();
    depth = 0;
    for (auto&& pair : internals)
    {
        if (pair.second.handler)
            pair.second.handler->prepare_for_reuse();
    }
}

bool ObjectHandler::reap_error(ErrorStack& stack)
{
    if (!the_error)
        return false;
    stack.push(the_error.release());
    if (current && current->handler)
        current->handler->reap_error(stack);
    return true;
}

bool ObjectHandler::write(IHandler* output) const
{
    SizeType count = 0;
    if (!output->StartObject())
        return false;

    for (auto&& pair : internals)
    {
        if (!pair.second.handler || (pair.second.flags & Flags::IgnoreWrite))
            continue;
        if (!output->Key(pair.first.data(), pair.first.size(), true))
            return false;
        if (!pair.second.handler->write(output))
            return false;
        ++count;
    }
    return output->EndObject(count);
}

namespace nonpublic
{
    template <class T>
    class IHandlerAdapter : public IHandler
    {
    private:
        T* t;

    public:
        explicit IHandlerAdapter(T* t) : t(t) {}

        virtual bool Null() override { return t->Null(); }

        virtual bool Bool(bool v) override { return t->Bool(v); }

        virtual bool Int(int v) override { return t->Int(v); }

        virtual bool Uint(unsigned v) override { return t->Uint(v); }

        virtual bool Int64(std::int64_t v) override { return t->Int64(v); }

        virtual bool Uint64(std::uint64_t v) override { return t->Uint64(v); }

        virtual bool Double(double v) override { return t->Double(v); }

        virtual bool String(const char* str, SizeType sz, bool copy) override
        {
            return t->String(str, sz, copy);
        }

        virtual bool StartObject() override { return t->StartObject(); }

        virtual bool Key(const char* str, SizeType sz, bool copy) override
        {
            return t->Key(str, sz, copy);
        }

        virtual bool EndObject(SizeType sz) override { return t->EndObject(sz); }

        virtual bool StartArray() override { return t->StartArray(); }

        virtual bool EndArray(SizeType sz) override { return t->EndArray(sz); }

        virtual void prepare_for_reuse() override { std::abort(); }
    };

    template <class InputStream>
    static bool read_json(InputStream& is, BaseHandler* h, ParseStatus* status)
    {
        rapidjson::Reader r;
        rapidjson::ParseResult rc = r.Parse(is, *h);
        if (status)
        {
            status->set_result(rc.Code(), rc.Offset());
            h->reap_error(status->error_stack());
        }
        return rc.Code() == 0;
    }

    bool parse_json_string(const char* str, BaseHandler* handler, ParseStatus* status)
    {
        rapidjson::StringStream is(str);
        return read_json(is, handler, status);
    }

    bool parse_json_file(std::FILE* fp, BaseHandler* handler, ParseStatus* status)
    {
        if (!fp)
            return false;
        char buffer[1000];
        rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
        return read_json(is, handler, status);
    }

    struct StringOutputStream : private NonMobile
    {
        std::string* str;

        void Put(char c) { str->push_back(c); }

        void Flush() {}
    };

    std::string serialize_json_string(const BaseHandler* handler)
    {
        std::string result;
        StringOutputStream os;
        os.str = &result;
        rapidjson::Writer<StringOutputStream> writer(os);
        IHandlerAdapter<decltype(writer)> adapter(&writer);
        handler->write(&adapter);
        return result;
    }

    bool serialize_json_file(std::FILE* fp, const BaseHandler* handler)
    {
        if (!fp)
            return false;
        char buffer[1000];
        rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
        rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
        IHandlerAdapter<decltype(writer)> adapter(&writer);
        return handler->write(&adapter);
    }

    std::string serialize_pretty_json_string(const BaseHandler* handler)
    {
        std::string result;
        StringOutputStream os;
        os.str = &result;
        rapidjson::PrettyWriter<StringOutputStream> writer(os);
        IHandlerAdapter<decltype(writer)> adapter(&writer);
        handler->write(&adapter);
        return result;
    }

    bool serialize_pretty_json_file(std::FILE* fp, const BaseHandler* handler)
    {
        if (!fp)
            return false;
        char buffer[1000];
        rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        IHandlerAdapter<decltype(writer)> adapter(&writer);
        bool res = handler->write(&adapter);
        if (res)
        {
            putc('\n', fp);
        }
        return res;
    }
}
}