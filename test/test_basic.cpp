#include <staticjson/staticjson.hpp>

#include "catch.hpp"

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
    REQUIRE(from_json_string(input, &obj, nullptr));
    REQUIRE(obj.i == -980008);
}

TEST_CASE("Failure test")
{
    MyObject obj;
    const char* input = ("{\"i\": -980008, \"j\": 42}");
    ParseStatus res;
    REQUIRE(!from_json_string(input, &obj, &res));
    CAPTURE(res.description());
    REQUIRE(obj.i == -980008);
}

TEST_CASE("Vector test")
{
    std::vector<int> integers;
    const char* input = ("[1,2,3,4,5,6]");
    ParseStatus res;
    bool success = from_json_string(input, &integers, nullptr);
    CAPTURE(res.description());
    REQUIRE(success);
    REQUIRE(integers.size() == 6);
}

TEST_CASE("Serial")
{
    REQUIRE(to_json_string(123) == "123");
    MyObject obj;
    obj.i = 999;
    REQUIRE(to_pretty_json_string(obj).size() > 0);
    REQUIRE(to_json_string(std::vector<int>{1, 2, 3, 4, 5, 6}) == "[1,2,3,4,5,6]");
}