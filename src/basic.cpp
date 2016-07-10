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

std::string ObjectHandler::type_name() { return "object"; }
}