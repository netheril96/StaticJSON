autojsoncxx
===========
A header-only library and a code generator to **automagically** translate between **JSON** and **C++** types.

## Overview

JSON is an excellent format for data serialization due to its simplicity, flexibility, portability and human-readable nature. Writing code to parse and generate JSON, however, is not an easy task in a statically typed language. Even with the help of JSON libraries, you need to write a lot of boilerplate code, and convoluted ones if you need to enforce the static typing of C++.

More importantly, maually writing the code means duplication of effort, and duplication is bad for programmers. When your client or PM request a change in feature, many classes (like the class responsible for configuration) will likely change, and you will have to rewrite the code. During the rewrite, time is wasted, people become impatient, and bugs may be introduced when the class definition, parsing and serialization code become out of sync.

*autojsoncxx* is an attempt to solve this problem by automating such process. It is currently still in alpha stage, so expect things to change in the future, or to break down when your class structure is too convoluted.

Dependency: RapidJSON (https://github.com/miloyip/rapidjson)

## Features

* The parsing/serializing code are *automagically* generated. You don't even need to understand what is proper JSON to use it, although it may help you diagnose problems.
* *Detailed error message*. Not only do you get informed if the JSON is not valid, but you will have a verbose trace back pointing to the location of the problem as well, if the JSON value does not fit your class structure.
* *Ease of use*. Many convience functions are added so that a single function call is enough for most use cases. The library as well as its dependency are header only, so no complicated setup for your build system is needed.
* *Fast*. The underlying JSON engine (RapidJSON) has been benchmarked to be about an order of magnitude faster than other popular JSON libraries. Besides, this library uses its SAX API, obviating the need of constructing a Document Object Model as the intermediate representation. Lastly, the library utilizes C++ templates to generate the algorithm at compile time, so no overhead of runtime indirection (except when error occurs).
* *Flexible framework*. You can add more type support to the library by specializing certain template classes. In addition, whenever a class is generated, you can also parse/serialize an array of such class, a nullable wrapper of such class, another class that contains it, etc.
* *Liberal licence*. Both the library and its dependency are licenced liberally (MIT or BSD-like). Anyone is free to copy, distribute, modify or include in their own projects, be it open source or commercial.

## First example

The code generator reads a JSON file that defines the class structure. An example definition is like this (remember to fully qualify the type name with its namespace)

```javascript
{
    "name": "Person",
    "members":
    [
        ["unsigned long long", "ID", {"required": true}],
        ["std::string", "name", {"default": "anonymous"}],
        ["double", "height"],
        ["double", "weight"],
        ["std::vector<unsigned long long>", "known_associates"]
    ]
}
```

Run the script *autojsoncxx.py* (requires Python 3) on this definition file, and a header file will be generated. It includes a definition for `Person` as well as some helper classes. The `Person` is a `struct` with all members public, meant as a data holder without any additional functionalities.

```bash
python3 autojsoncxx.py --out=person.hpp def.json
```

Remember to add the include directory of *autojsoncxx* and *rapidjson* to your project header search path (no linking is required). 

The below examples uses c++11 features, but the library should also work with c++03 compilers.

### Serialization
```c++
#define AUTOJSONCXX_MODERN_COMPILER 1 // Turn on all the c++11 features of the library
#include <iostream>
#include "person.hpp"

int main()
{
    Person p;
    p.name = "Mike";
    p.ID = 8940220481904ULL;
    p.weight = 70;
    p.height = 1.77;
    p.known_associates = { 149977889346362, 90000134866608, 44412567664 };
    // Use successive push_back() if your compiler is not c++11 ready

    autojsoncxx::to_pretty_json_file("person.json", p);
    return 0;
}
```
This will generate a file `person.json` with contents below:
```javascript
{
    "name": "Mike",
    "ID": 8940220481904,
    "height": 1.77,
    "weight": 70.0,
    "known_associates": [
        149977889346362,
        90000134866608,
        44412567664
    ]
}
```

### Parsing
Now let's try read that back
```c++
#define AUTOJSONCXX_MODERN_COMPILER 1
#include <iostream>
#include "person.hpp"

int main()
{
    autojsoncxx::ParsingResult result;
    Person p;
    if (!autojsoncxx::from_json_file("person.json", p, result)) {
        std::cerr << result << '\n';
        return -1;
    }

    std::cout << "ID: " << p.ID << '\n'
              << "name:  " << p.name << '\n'
              << "height: " << p.height << '\n'
              << "weight: " << p.weight << '\n';

    std::cout << "known associates: ";
    for (auto&& id : p.known_associates)
        std::cout << id << '\t';
    std::cout << '\n';
    return 0;
}
```
### Error handling
If the JSON file is malformed, any decent JSON library will detect it and tell you what goes wrong. But what if the JSON value is perfectly valid, but not layed out the way you expected? Usually you have to manually check the DOM tree against your specification, but this library will automatically generates the necessary code.

Here is valid JSON file

```js
{
    "name": "Mike",
    "ID": 8940220481904,
    "height": 1.77,
    "weight": 70.0,
    "known_associates": [
        "Jack", "Mary"
    ]
}
```

Running through the parsing code, and you will get an error output:

```
Parsing failed at offset 127 with error code 16:
Terminate parsing due to Handler error.

Trace back (last call first):
(*) Type mismatch between expected type "uint64_t" and actual type "string"
(*) Error at array element with index 0
(*) Error at object member with name "known_associates"
```

To programmingly examine the error, iterate over the `autojsoncxx::ParsingResult` class. For example:

```c++
for (auto&& e : result) {
    using namespace autojsoncxx::error;

    switch (e.type()) {
    case UNKNOWN_FIELD: {
        const UnknownFieldError& err = static_cast<const UnknownFieldError&>(e);
        if (err.field_name().find("Version") != std::string::npos)
            std::cerr << "This is a definition of different protocol version\n";
    } break;

    case NUMBER_OUT_OF_RANGE:
        std::cerr << "Maybe you should use a 64-bit integer type instead?\n";
        break;

    case TYPE_MISMATCH: {
        const TypeMismatchError& err = static_cast<const TypeMismatchError&>(e);
        std::cout << "don't you dare use a " << err.actual_type()
                  << " to fool me!\n";
    } break;

    case OBJECT_MEMEMBER: {
        const ObjectMemberError& err = static_cast<const ObjectMemberError&>(e);
        std::cout << "The member " << err.member_name() << " is naughty!\n";
    } break;

    default:
        break;
    }
}
```
## Type support
These types are supported by this library (you can contain members variables with such types):

* Basic types: `bool`, `int`, `unsigned int`, `long long`, `unsigned long long`, `std::string`
* Array tyeps: `std::vector<>`, `std::deque<>`, `std::array<>`, `std::tuple<>` (this one needs special care)
* Nullable types: `std::shared_ptr<>`

If you include `<autojsoncxx/boost_types.hpp>`, you will also get support for

* Array types: `boost::container::vector<>`, `boost::container::deque<>`, `boost::array`
* Nullable types: `boost::shared_ptr<>`, `boost::optional<>`

**No raw pointer and reference types are supported**. They do not convey any information about ownership, and will make correct memory management (especially by a code generator) much more difficult.

The supported types can be arbitrarily nested, for example

```c++
#define AUTOJSONCXX_MODERN_COMPILER 1 
#include <iostream>
#include "person.hpp"

int main()
{
    auto test = std::make_tuple(std::vector<std::string>{"A", "BC", "DEF"},
                                9.0, true, std::shared_ptr<int>(),
                                std::make_shared<std::array<Person, 2>>());
    
    std::string str;
    
    // This requires true variadic template support 
    // MSVC 2012 has std::tuple<>, but it is faked with macros
    // Avoid std::tuple<> if your compiler is not strong enough
    autojsoncxx::to_pretty_json_string(str, test);
    
    std::cout << str << '\n';
    return 0;
}
```

Sample output

```js
[
    [
        "A",
        "BC",
        "DEF"
    ],
    9.0,
    true,
    null,
    [
        {
            "ID": 0,
            "name": "anonymous",
            "height": 0.0,
            "weight": 0.0,
            "known_associates": []
        },
        {
            "ID": 0,
            "name": "anonymous",
            "height": 0.0,
            "weight": 0.0,
            "known_associates": []
        }
    ]
]
```

## Other

You can have multiple definition of classes in the same file, simply by making the root an array of definitions.

## To do

* More extensive testing of the library.
* Testing it on compilers other than clang.
* More documentation and comments.
* Map types support (e.g. `std::map<>`, `std::unordered_multimap<>`).
