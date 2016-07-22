# StaticJSON

Fast, direct and static typed parsing of JSON with C++.

## Overview

JSON is a popular format for data exchange. Reading and writing JSON in C++, however, is nontrivial. Even with the help of libraries, one still needs to write lots of boilerplate code, and it is extremely hard to guard against all possible errors, since JSON is dynamically typed while C++ employs static typing.

More importantly, manually writing such code is a violation of **DRY** principle. When manually written, the class definition, parsing and serialization code can easily become out of sync, leading to brittle code and subtle bugs.

`StaticJSON` is an attempt to solve this problem by automating such process.

## Build

`StaticJSON` requires a C++11 compiler. Tested with clang++ 3.5, g++ 4.8 and MSVC 2015.

Just drop the `include` and `src` directory into your own project and build along with other sources. No special build settings are needed.

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

This requires you to specialize a template for your custom class. For example, the `Date` specialization is written as

```c++
namespace staticjson
{
template <>
class Handler<Date> : public ObjectHandler
{
public:
    explicit Handler(Date* d)
    {
        add_property("year", &d->year);
        add_property("month", &d->month);
        add_property("day", &d->day);
        set_flags(Flags::DisallowUnknownKey);
    }
};
}
```

You may need to declare `Handler` as a friend in order to access private and protected members.

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
* **Array types**: `std::vector<•>`, `std::deque<•>`, `std::list<•>`
* **Nullable types**: `std::nullptr_t`, `std::unique_ptr<•>`, `std::shared_ptr<•>`
* **Map types**: `std::{map, multimap, unordered_map, unordered_multimap}<std::string, •>`

## Misc

The project was originally named *autojsoncxx* and requires a code generator to run.
