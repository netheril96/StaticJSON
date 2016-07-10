#include <rapidjson/reader.h>
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
    Handler<MyObject> h(&obj);
    rapidjson::StringStream ss("{\"i\": -980008}");
    rapidjson::Reader r;
    REQUIRE(r.Parse<0>(ss, h));
    REQUIRE(obj.i == -980008);
}

TEST_CASE("Failure test")
{
    MyObject obj;
    Handler<MyObject> h(&obj);
    rapidjson::StringStream ss("{\"i\": -980008, \"j\": 42}");
    rapidjson::Reader r;
    REQUIRE(!r.Parse<0>(ss, h));
    error::ErrorStack stk;
    REQUIRE(h.reap_error(stk));
    std::cerr << stk;
}

TEST_CASE("Vector test")
{
    std::vector<int> integers;
    Handler<decltype(integers)> h(&integers);
    rapidjson::StringStream ss("[1,2,3,4,5,6]");
    rapidjson::Reader r;
    REQUIRE(r.Parse<0>(ss, h));
    REQUIRE(integers.size() == 6);
}