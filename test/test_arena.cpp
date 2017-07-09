#include <staticjson/arena.hpp>

#include "catch.hpp"

using namespace staticjson;

TEST_CASE("astring")
{
    Arena<> arena;
    astring abc("abc", ArenaAllocator<char>(&arena));
    CHECK(abc == "abc");
    abc.resize(2000, 'd');
    astring def(3000, 'e', ArenaAllocator<char>(&arena));
    CHECK(def.size() == 3000);
}
