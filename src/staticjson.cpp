#include <staticjson/basic.hpp>
#include <staticjson/io.hpp>

#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>

#include <cstdio>

namespace staticjson
{
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
    this->parsed = true;
    for (auto&& pair : internals)
    {
        if (pair.second.handler && (pair.second.flags & Flags::Required)
            && !pair.second.handler->is_parsed())
        {
            set_missing_required(pair.first);
        }
    }
    return !the_error;
}

void ObjectHandler::reset()
{
    internals.clear();
    current = nullptr;
    current_name.clear();
    depth = 0;
}

bool ObjectHandler::reap_error(error::ErrorStack& stack)
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
    static bool read_json(InputStream& is, BaseHandler* h, ParsingResult& result)
    {
        rapidjson::Reader r;
        result.set_result(r.Parse(is, *h));
        h->reap_error(result.error_stack());
        return !result.has_error();
    }

    bool parse_json_string(const char* str, BaseHandler* handler, ParsingResult& result)
    {
        rapidjson::StringStream is(str);
        return read_json(is, handler, result);
    }

    bool parse_json_file(std::FILE* fp, BaseHandler* handler, ParsingResult& result)
    {
        char buffer[1000];
        rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
        return read_json(is, handler, result);
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
        char buffer[1000];
        rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        IHandlerAdapter<decltype(writer)> adapter(&writer);
        return handler->write(&adapter);
    }
}
}