#include <staticjson/basic.hpp>

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

std::string ObjectHandler::type_name() { return "object"; }

bool ObjectHandler::precheck(const char* actual_type)
{
    if (depth <= 0)
    {
        the_error.reset(new error::TypeMismatchError(type_name(), actual_type));
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
    return bool(the_error);
}
}