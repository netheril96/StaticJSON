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

