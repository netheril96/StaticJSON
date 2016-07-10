#include <staticjson/io.hpp>
#include <staticjson/primitive_types.hpp>
#include <staticjson/stl_types.hpp>

#include "catch.hpp"

#include <iostream>

using namespace staticjson;

struct MyObject
{
    int i;

    void staticjson_init(ObjectHandler* h)
    {
        h->set_flags(Flags::DisallowUnknownKey);
        h->add_property("i", &i);
    }
};

TEST_CASE("Basic test")
{
    MyObject obj;
    const char* input = "{\"i\": -980008}";
    ParseStatus res;
    REQUIRE(from_json_string(input, obj, res));
    CAPTURE(res.description());
    REQUIRE(obj.i == -980008);
}

TEST_CASE("Failure test")
{
    MyObject obj;
    const char* input = ("{\"i\": -980008, \"j\": 42}");
    ParseStatus res;
    REQUIRE(!from_json_string(input, obj, res));
    CAPTURE(res.description());
    REQUIRE(obj.i != -980008);
}

TEST_CASE("Vector test")
{
    std::vector<int> integers;
    const char* input = ("[1,2,3,4,5,6]");
    ParseStatus res;
    REQUIRE(from_json_string(input, integers, res));
    REQUIRE(integers.size() == 6);
}