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

TEST_CASE("arena ptr")
{
    Arena<> arena;
    ArenaPtr<int> aint(&arena, 1);
    CHECK(*aint == 1);
    CHECK(aint.get() != nullptr);

    ArenaPtr<std::vector<char, ArenaAllocator<char>>> avector(&arena, ArenaAllocator<char>(&arena));
    avector->push_back(1);
    avector->push_back(2);
    CHECK(avector->size() == 2);
    CHECK(avector->back() == 2);
}
