#include <rapidjson/reader.h>
#include <staticjson/primitive_types.hpp>

#include "catch.hpp"

using namespace staticjson;

TEST_CASE("Basic test")
{
    int i;
    Handler<int> h(&i);
    rapidjson::StringStream ss("123");
    rapidjson::Reader r;
    REQUIRE(r.Parse<0>(ss, h));
    REQUIRE(i == 123);
}