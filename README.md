autojsoncxx
===========
A header-only library and a code generator to **automagically** translate between **JSON** and **C++** types.

## Overview

JSON is an excellent format for data serialization due to its simplicity, flexibility, portability and human-readable nature. Writing code to parse and generate JSON, however, is not an easy task in a statically typed language even with the help of libraries, especially when you want to retain the advantage of such language, that is, the type checking. *autojsoncxx* is an attempt to automate such process. It is still currently in development, so expect things to change in the future, or it breaks down when your class structure is deeply nested.

Dependency: RapidJSON (https://github.com/miloyip/rapidjson)

## First example

The code generator reads a JSON file that defines the class structure. An example definition is like this


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

The below examples uses c++11 features, but the library also works with c++03 compilers.

### Serialization
```c++
#define AUTOJSONCXX_MODERN_COMPILER 1 
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
    // Use successive push_back() if your compiler is not C++11 ready

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
