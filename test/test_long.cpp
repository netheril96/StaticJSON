#include "catch.hpp"

#include <staticjson/staticjson.hpp>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

using namespace staticjson;

struct LongNameStruct
{
    std::tuple<std::shared_ptr<std::vector<std::string>>, uint64_t, std::list<std::string>> element;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("128ffc8eb216405816ab325f0f175f228773e15262140783a58a82e758e70cfa31a88c6408"
                        "6a0ed45ad64456f909d17a53926715d0db17213daa7d8efa30cc67",
                        &element);
    }
};

struct Wrapper
{
    LongNameStruct element;
    void staticjson_init(ObjectHandler* h) { h->add_property("element", &element); }
};

TEST_CASE("Test for long name", "[parsing],[long],[serialization]")
{
    Wrapper wrapper;
    std::get<0>(wrapper.element.element) = std::make_shared<std::vector<std::string>>();
    std::get<0>(wrapper.element.element)->emplace_back("system");
    std::get<2>(wrapper.element.element).emplace_back("go");
    std::get<2>(wrapper.element.element).emplace_back("staticjson");

    std::string str = to_json_string(wrapper);

    CHECK(
        str
        == R"({"element":{"128ffc8eb216405816ab325f0f175f228773e15262140783a58a82e758e70cfa31a88c64086a0ed45ad64456f909d17a53926715d0db17213daa7d8efa30cc67":[["system"],0,["go","staticjson"]]}})");

    Wrapper another;
    ParseStatus status;
    CHECK(from_json_string(str.c_str(), &another, &status));
    CHECK(*std::get<0>(wrapper.element.element) == *std::get<0>(another.element.element));
    CHECK(std::get<1>(wrapper.element.element) == std::get<1>(another.element.element));
    CHECK(std::get<2>(wrapper.element.element) == std::get<2>(another.element.element));
}
