# StaticJSON

![](https://travis-ci.org/netheril96/StaticJSON.svg?branch=master)
![](https://ci.appveyor.com/api/projects/status/github/netheril96/StaticJSON?branch=master&svg=true)

Fast, direct and static typed parsing of JSON with C++.

## Overview

JSON is a popular format for data exchange. Reading and writing JSON in C++, however, is nontrivial. Even with the help of libraries, one still needs to write lots of boilerplate code, and it is extremely hard to guard against all possible errors, since JSON is dynamically typed while C++ employs static typing.

More importantly, manually writing such code is a violation of **DRY** principle. When manually written, the class definition, parsing and serialization code can easily become out of sync, leading to brittle code and subtle bugs.

`StaticJSON` is an attempt to solve this problem by automating such process.

## Usage

`StaticJSON` requires a C++11 compiler. Tested with clang++ 3.5, g++ 4.8 and MSVC 2015.

Just drop the `include` and `src` directory into your own project and build along with other sources. It requires you to separately install [`rapidjson`](rapidjson.org). Currently tested against version 1.1 (2016-8-25).

The cmake files are provided for building and running the integration test.

## Quick start

### Builtin types

```c++
#include <staticjson/staticjson.hpp>

int builtin_test() {
    using namespace staticjson;
    std::string a = to_json_string(std::vector<double>{1.0, 2.0, -3.1415});
    std::string b = to_pretty_json_string(std::map<std::string, std::shared_ptr<std::list<bool>>>{});

    std::vector<std::unordered_map<std::string, std::int64_t>> data;
    const char* json_string = "[{\" hello \": 535353, \" world \": 849},"
        " {\" k \": -548343}]";
    assert(from_json_string(json_string, &data, nullptr));
    assert(data.size() == 2);
    assert(data[1][" k "] == -548343);

    to_pretty_json_file(stdout, data);
    return 0;
}
```

### Register custom class types

For your own classes, you need to add some definitions first before you can use the `from_json` and `to_json` functions. There are two ways of doing this.

#### Intrusive definition

This way requires you to implement a special method in the class prototyped `void staticjson_init(staticjson::ObjectHandler* h)`. Example definition

```c++
struct Date
{
    int year, month, day;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("year", &year);
        h->add_property("month", &month);
        h->add_property("day", &day);
        h->set_flags(Flags::DisallowUnknownKey);
    }
};

struct BlockEvent
{
    std::uint64_t serial_number, admin_ID = 255;
    Date date;
    std::string description, details;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("serial_number", &serial_number);
        h->add_property("administrator ID", &admin_ID, Flags::Optional);
        h->add_property("date", &date, Flags::Optional);
        h->add_property("description", &description, Flags::Optional);
        h->add_property("details", &details, Flags::Optional);
    }
};
```

### Non-intrusive definition

This requires you to overload a special function for your custom class. For example, the `Date` overload is written as

```c++
namespace staticjson
{
void init(Date* d, ObjectHandler* h)
{
    h->add_property("year", &d->year);
    h->add_property("month", &d->month);
    h->add_property("day", &d->day);
    h->set_flags(Flags::DisallowUnknownKey);
}
}
```

You may need to declare `staticjson::init` as a friend function in order to access private and protected members.

### Register enumeration types

Example

```c++
enum class CalendarType
{
    Gregorian,
    Chinese,
    Jewish,
    Islam
};

STATICJSON_DECLARE_ENUM(CalendarType,
                        {"Gregorian", CalendarType::Gregorian},
                        {"Chinese", CalendarType::Chinese},
                        {"Jewish", CalendarType::Jewish},
                        {"Islam", CalendarType::Islam})
```

This will convert the enum type to/from strings, and signal error if the string is not in the list.

Note that this macro must not be instantiated inside a namespace.

## Custom conversion

If you want a type to be serialized in a different way, such as a custom `Date` object as an ISO8601 string or an arbitrary precision integer as a list of 32-bit integers, you can enable the custom conversion for the type. To do so, specialize the template class in namespace `staticjson`

```c++
namespace staticjson
{
template <>
struct Converter<Date>
{
    typedef std::string shadow_type; 
    // This typedef is a must. The shadow type is a C++ type 
    // that can be directly converted to and from JSON values.

    static std::unique_ptr<ErrorBase> from_shadow(const shadow_type& shadow, Date& value)
    {
        bool success = value.parseISO8601(shadow);
        if (success)
            return nullptr;
        return std::make_unique<CustomError>("Invalid ISO 8601 string");
    }

    static void to_shadow(const Date& value, shadow_type& shadow)
    {
        shadow = value.toISO8601();
    }
};
}

```

## Error handling

`StaticJSON` strives not to let any mismatch between the C++ type specifications and the JSON object slip. It detects and reports all kinds of errors, including type mismatch, integer out of range, floating number precision loss, required fields missing, duplicate keys etc. Many of them can be tuned on or off. It also reports an stack trace in case of error (not actual C++ exception).

The third parameter of all `from_json` family of functions is a nullable pointer to `staticjson::ParseStatus` object. If present, the error information will be dumped into it. An example error message is

```
Parsing failed at offset 1000 with error code 16:
Terminate parsing due to Handler error.

Traceback (last call first)
* Type mismatch between expected type "unsigned long long" and actual type "string"
* Error at object member with name "serial_number"
* Error at array element at index 0
* Error at object member with name "dark_history"
* Error at array element at index 1
```

## List of builtin supported types

* **Boolean types**: `bool`, `char`
* **Integer types**: `int`, `unsigned int`, `long`, `unsigned long`, `long long`, `unsigned long long`
* **Floating point types**: `float`, `double`
* **String types**: `std::string`
* **Array types**: `std::vector<•>`, `std::deque<•>`, `std::list<•>`, `std::array<•>`
* **Nullable types**: `std::nullptr_t`, `std::unique_ptr<•>`, `std::shared_ptr<•>`
* **Map types**: `std::{map, multimap, unordered_map, unordered_multimap}<std::string, •>`
* **Tuple types**: `std::tuple<...>`

## Dynamic typing

If you need occasional escape from the rigidity of C++'s static type system, but do not want complete dynamism, you can still find the middle ground in `StaticJSON`.

* You can embed a `staticjson::Document` (alias of `rapidjson::Document`) in your class/struct, which allows static typing for some class members and dynamic typing for others. Note `Document` is already nullable so do not use a smart pointer to `Document`.
* You can convert a `Document` or `Value` to and from a C++ type registered in `StaticJSON`. The functions are aptly named `from_json_value`, `from_json_document`, `to_json_value`, `to_json_document`.


## Export as JSON Schema

Function `export_json_schema` allows you to export the validation rules used by `StaticJSON` as JSON schema. It can then be used in other languages to do the similar validation. Note the two rules are only approximate match, because certain rules cannot be expressed in JSON schema yet, and because some languages have different treatments of numbers from C++.

## Misc

The project was originally named *autojsoncxx* and requires a code generator to run.
