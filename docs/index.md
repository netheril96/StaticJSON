# Overview

JSON is an excellent format for data serialization due to its simplicity, flexibility, portability and human-readable nature. Writing code to parse and generate JSON, however, is not an easy task in a statically typed language. Even with the help of JSON libraries, you still need to write a lot of boilerplate code, and convoluted ones if you need to enforce the static typing of C++.

More importantly, manually writing such code means duplication of effort, a violation of **DRY** principle. When manually written, the class definition, parsing and serialization code can easily become out of sync with one another, leading to brittle code and subtle bugs.

*autojsoncxx* is an attempt to solve this problem by automating such process.

### Dependency 

* [RapidJSON](https://github.com/miloyip/rapidjson) 
* [Python](https://www.python.org) (2.7 or 3.3+)
* (optional) [Parsimonious](https://github.com/erikrose/parsimonious)
* (optional) [Catch](https://github.com/philsquared/Catch)
* (optional) [Boost](http://www.boost.org)

## Features

* The parsing/serializing code are **automagically** generated. You don't even need to understand what is proper JSON to use it, although it may help you diagnose problems.
* **Detailed error message**. Not only do you get informed if the JSON is not valid, but you will have a verbose trace back pointing to the location of the problem as well, if the JSON value does not fit your class structure.
* **Ease of use**. Many convenience functions are added so that a single function call is enough for most use cases. The library as well as its dependency are header only, while the code generator depends only on standard library of Python, so no complicated setup for your build system is needed.
* **Fast**. The underlying JSON engine (RapidJSON) has been benchmarked to be about an order of magnitude faster than other popular JSON libraries. Besides, this library uses its SAX API, obviating the need of constructing a Document Object Model as the intermediate representation. Lastly, the library utilizes C++ templates to generate the algorithm at compile time, so no overhead of runtime indirection (except when error occurs).
* **Flexible framework**. You can add more type support to the library by specializing certain template classes. In addition, whenever a class is generated, you can also parse/serialize an array of such class, a nullable wrapper of such class, another class that contains it, etc.
* **Liberal license**. Both the library and its dependency are licensed liberally (MIT or BSD-like). Anyone is free to copy, distribute, modify or include in their own projects, be it open source or commercial.

## Testing

[![Build Status](https://travis-ci.org/netheril96/autojsoncxx.svg?branch=master)](https://travis-ci.org/netheril96/autojsoncxx)

To build the test, you need a sufficiently new compiler because the goal is to test all the type support, including many ones only introduced in c++11.

First clone the repository, and pull the dependency

```bash
git clone https://github.com/netheril96/autojsoncxx.git
git submodule init
git submodule update
```

*UNIX/Linux/Mac users*:

```bash
make
make test
```

*Windows users*: 

Generate the `test/userdef.hpp` file from the definition `examples/userdef.json`. Then open the solution file under `test/mscvXX_test/` to build and run the test.

If too many tests fail, make sure your work directory points to the `test` directory.

### Currently tested compilers

* Clang 3.4/3.5 on Mac OS X (11.9)
* GCC 4.9 (Homebrew) on Mac OS X (11.9)
* Clang 3.0 on Ubuntu 12.04 (x64)
* GCC 4.8 on Ubuntu 14.04.1 (x86/x64)
* MSVC 10 (x86) on Windows 7
* MSVC 11/12 (x86/x64) on Windows 7

## Memory handling and exceptions

Exception handling (`throw`, `try`, `catch`) is not used by this library, to accommodate the needs of fake C++ programmers. It is designed, however, to be exception safe by using RAII wrappers to do all resource management. Copy, move constructor/assignment operator are disabled at certain places to avoid ownership mismanagement.

Notably, the `ParsingResult` class is not copyable. This simplifies the memory handling because it fully owns the error stack. It is movable, however, if you define `AUTOJSONCXX_HAS_RVALUE`. If you ever need to pass it around or store it somewhere, the simplest way is to use a shared pointer.

## C++11 features

A set of macros control the usage of c++11 features. Define these macros as nonzero constants *before* inclusion, or define it in your build system.

* `AUTOJSONCXX_MODERN_COMPILER`: turn on all of the below
* `AUTOJSONCXX_HAS_MODERN_TYPES`: add support for c++11 new types, such as `std::shared_ptr<>`.
* `AUTOJSONCXX_HAS_RVALUE`: enable the use of r-value references and move semantic.
* `AUTOJSONCXX_HAS_NOEXCEPT`: enable the use of keyword `noexcept` and the function `std::move_if_noexcept()`.
* `AUTOJSONCXX_HAS_VARIADIC_TEMPLATE`: enable the use of variadic templates. required if `std::tuple<>` is used.
* `AUTOJSONCXX_HAS_EXPLICIT_OPERATOR`: enable the use of `explicit operator bool()`. Otherwise no conversion to bool operator is defined.

The 64-bit integer type `long long` and `unsigned long long` is always required. Though not in C++03 standard, most compilers support it nonetheless.

## Encoding

The default encoding is `UTF-8`. If you need to read/write JSON in `UTF-16` or `UTF-32`, instantiate the class `SAXEventHandler` and/or `Serializer`, and use it in combination with RapidJSON's transcoding capability.
