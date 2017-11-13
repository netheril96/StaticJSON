#include <staticjson/staticjson.hpp>

#include "catch.hpp"
#include "myarray.hpp"

#include <cerrno>
#include <deque>
#include <list>
#include <vector>

using namespace staticjson;

const std::string& get_base_dir();

TEST_CASE("Success tensor")
{
    std::list<std::array<Array<double>, 3>> tensor;
    ParseStatus err;

    bool success = from_json_file(get_base_dir() + "/examples/success/tensor.json", &tensor, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }

    REQUIRE(tensor.size() == 4);
    REQUIRE(!tensor.back().empty());
    REQUIRE(tensor.back().back().empty());
    auto&& first = tensor.front();

    REQUIRE(first.size() == 3);
    REQUIRE(first[0].size() == 3);
    REQUIRE(first[0][0] == 1);
}

TEST_CASE("Error tensor")
{
    std::list<std::array<Array<double>, 2>> tensor;
    {
        ParseStatus err;
        REQUIRE(!from_json_file(
            get_base_dir() + "/examples/failure/tensor_type_mismatch.json", &tensor, &err));
        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
    }
    {
        ParseStatus err;
        REQUIRE(!from_json_file(
            get_base_dir() + "/examples/failure/tensor_length_error.json", &tensor, &err));
        REQUIRE(err.begin()->type() == error::ARRAY_LENGTH_MISMATCH);
    }
}
