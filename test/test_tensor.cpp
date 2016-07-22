#include <staticjson/staticjson.hpp>

#include "catch.hpp"

#include <cerrno>
#include <deque>
#include <list>
#include <vector>

using namespace staticjson;

const std::string& get_base_dir();

TEST_CASE("Success tensor")
{
    std::list<std::vector<std::deque<double>>> tensor;
    ParseStatus err;

    bool success = from_json_file(get_base_dir() + "/examples/success/tensor.json", &tensor, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }

    REQUIRE(tensor.size() == 4);
    REQUIRE(tensor.back().empty());
    auto&& first = tensor.front();

    REQUIRE(first.size() == 3);
    REQUIRE(first[0].size() == 3);
    REQUIRE(first[0][0] == 1);
}

TEST_CASE("Error tensor")
{
    std::list<std::vector<std::deque<double>>> tensor;
    errno = 0;
    REQUIRE(!from_json_file(get_base_dir() + "/examples/failure/tensor.json", &tensor, nullptr));
    REQUIRE(errno == 0);    // require that the error comes from parsing, not filesystem I/O
}
