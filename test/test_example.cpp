#include <staticjson/staticjson.hpp>

#include <cassert>

int builtin_test()
{
    using namespace staticjson;
    std::string a = to_json_string(std::vector<double>{1.0, 2.0, -3.1415});
    std::string b
        = to_pretty_json_string(std::map<std::string, std::shared_ptr<std::list<bool>>>{});

    std::vector<std::unordered_map<std::string, std::int64_t>> data;

    const char* json_string = "[{\" hello \": 535353, \" world \": 849}, {\" k \": -548343}]";
    assert(from_json_string(json_string, &data, nullptr));
    assert(data.size() == 2);
    assert(data[1][" k "] == -548343);
    to_pretty_json_file("/tmp/B11FA212-ED7F-4577-83B1-CC7ADBBC8ED3.json", data);
    return 0;
}

#include "catch.hpp"

TEST_CASE("Example test") { REQUIRE(builtin_test() == 0); }