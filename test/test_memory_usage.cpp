#include <staticjson/staticjson.hpp>

#include "catch.hpp"

using namespace staticjson;

namespace
{
const std::string COMPLEX_VALUES = "complex_values";

struct Simple
{
    std::vector<float> floats;

    void staticjson_init(ObjectHandler* h) { h->add_property("floats", &floats); }
};

struct Struct
{
    std::string name;
    std::vector<std::tuple<std::vector<int>, std::list<std::string>>> complex_values;
    Simple simple;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("name", &name);
        h->add_property(COMPLEX_VALUES, &complex_values, staticjson::Flags::AllowDuplicateKey);
        h->add_property("simple", &simple);
    }
};
}

TEST_CASE("Test memory usage")
{
    size_t memory_usage_before = 0, memory_usage_after = 0;
    std::string serialized;

    std::vector<Struct> structs(100);
    for (size_t i = 0; i < structs.size(); ++i)
    {
        structs[i].name = "Struct" + std::to_string(i);
        structs[i].complex_values.resize(i);
        for (size_t j = 0; j < i; ++j)
        {
            auto&& v = structs[i].complex_values[j];
            std::get<0>(v).push_back(j);
            for (size_t k = 0; k < 2 * j; ++k)
            {
                std::get<1>(v).emplace_back(std::to_string(k));
            }
        }
    }
    Handler<std::vector<Struct>> h(&structs);
    serialized = nonpublic::serialize_json_string(&h);
    memory_usage_before = h.get_internal_handler().get_memory_pool().Capacity();

    structs.clear();
    h.prepare_for_reuse();

    ParseStatus status;
    REQUIRE(nonpublic::parse_json_string(serialized.c_str(), &h, &status));
    memory_usage_after = h.get_internal_handler().get_memory_pool().Capacity();

    printf("Memory usage: before %zu, after %zu\n", memory_usage_before, memory_usage_after);
    CHECK(memory_usage_before == memory_usage_after);
}
