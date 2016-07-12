#include <json/json.h>
#include <rapidjson/document.h>
#include <staticjson/staticjson.hpp>

#include <chrono>
#include <cstdio>
#include <functional>
#include <stdexcept>

bool enable_staticjson = true, enable_rapidjson = true, enable_jsoncpp = false;

// We don't want to use template here because inlining may result in some code optimized out
std::uint64_t how_many_milliseconds(const std::function<void(void)>& func, std::int64_t rounds)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (std::int64_t i = 0; i < rounds; ++i)
    {
        func();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
}

void runtime_assert(bool success)
{
    if (!success)
        throw std::runtime_error("Assertion failure. The benchmark results in invalid data.");
}

void benchmark_array_of_single_int(int rounds)
{
    const std::string str = "[-25555555]";

    auto l1 = [&]() {
        std::vector<int> buffer;
        runtime_assert(staticjson::from_json_string(str.c_str(), &buffer, nullptr)
                       && buffer.size() == 1
                       && buffer[0] == -25555555);
    };

    auto l2 = [&]() {
        rapidjson::Document h;
        h.Parse(str.c_str());
        runtime_assert(h.IsArray() && h.Size() == 1 && h[0].GetInt() == -25555555);
    };

    auto l3 = [&]() {
        Json::Value v;
        Json::Reader r;
        runtime_assert(r.parse(str, v));
        runtime_assert(v.isArray() && v[0] == -25555555);
    };

    fprintf(stderr,
            "Reading array of a single integer %d times:\n\tStaticJSON: %lld ms\n\tRapidJSON DOM: "
            "%lld ms\n\tJsonCpp: %lld ms\n\n",
            rounds,
            how_many_milliseconds(l1, rounds),
            how_many_milliseconds(l2, rounds),
            how_many_milliseconds(l3, rounds));

    fflush(stderr);
}

void benchmark_array_of_doubles(unsigned count)
{
    std::string str = staticjson::to_pretty_json_string(std::vector<double>(count, M_E));

    auto l1 = [&]() {
        std::vector<double> buffer;
        runtime_assert(staticjson::from_json_string(str.c_str(), &buffer, nullptr));
        runtime_assert(buffer.size() == count);
    };

    auto l2 = [&]() {
        rapidjson::Document h;
        h.Parse(str.c_str());
        runtime_assert(h.IsArray() && h.Size() == count && h[0].IsDouble());
    };

    auto l3 = [&]() {
        Json::Value v;
        Json::Reader r;
        runtime_assert(r.parse(str, v));
        runtime_assert(v.isArray() && v.size() == count && v[0].isDouble());
    };

    fprintf(stderr,
            "Reading %u numbers of doubles\n\n\tStaticJSON: %lld ms\n\tRapidJSON DOM: "
            "%lld ms\n\tJsonCpp: %lld ms\n\n",
            count,
            how_many_milliseconds(l1, 1),
            how_many_milliseconds(l2, 1),
            how_many_milliseconds(l3, 1));

    fflush(stderr);
}

struct MyClass
{
    std::string name;
    std::unordered_map<std::string, std::string> key_map;
    std::shared_ptr<bool> why_not;
    std::vector<std::vector<double>> matrix;

    void staticjson_init(staticjson::ObjectHandler* h)
    {
        h->add_property("name", &name);
        h->add_property("key_map", &key_map);
        h->add_property("why_not", &why_not);
        h->add_property("matrix", &matrix);
    }
};

void benchmark_custom_class(unsigned number)
{
    MyClass object{
        "hello",
        {{"123", "456"},
         {"999", "asdfosje"},
         {"DFDFDCD7-FECE-46E9-9CD2-79922D701332", "7CC8646A-4D89-4580-A469-B057A8F6AEE8"}},
        {},
        {{1, 3, 4, 5, 6}, {M_PI, M_E, M_PI * M_PI}}};

    auto str = staticjson::to_json_string(std::vector<MyClass>(number, object));

    {
        auto l1 = [&]() {
            std::vector<MyClass> buffer;
            staticjson::ParseStatus status;
            if (!staticjson::from_json_string(str.c_str(), &buffer, &status))
            {
                fprintf(stderr, "%s", status.description().c_str());
                runtime_assert(false);
            }
            runtime_assert(buffer.size() == number);
            runtime_assert(buffer[0].name == object.name);
            runtime_assert(buffer[0].key_map == object.key_map);
        };

        auto l2 = [&]() {
            rapidjson::Document h;
            h.Parse(str.c_str());
            runtime_assert(h.IsArray() && h.Size() == number);
            runtime_assert(h[0].IsObject() && h[0].HasMember("why_not"));
        };

        auto l3 = [&]() {
            Json::Value v;
            Json::Reader r;
            runtime_assert(r.parse(str, v));
            runtime_assert(v.isArray() && v.size() == number);
            runtime_assert(v[0].isObject() && v[0]["name"] == object.name);
        };

        fprintf(stderr,
                "Reading %u numbers of custom objects\n\n\tStaticJSON: %lld ms\n\tRapidJSON DOM: "
                "%lld ms\n\tJsonCpp: %lld ms\n\n",
                number,
                enable_staticjson ? how_many_milliseconds(l1, 1) : 0ll,
                enable_rapidjson ? how_many_milliseconds(l2, 1) : 0ll,
                enable_jsoncpp ? how_many_milliseconds(l3, 1) : 0ll);

        fflush(stderr);
    }
    {
        auto l1 = [&]() {
            std::vector<MyClass> buffer;
            staticjson::ParseStatus status;
            if (!staticjson::from_json_string(str.c_str(), &buffer, &status))
            {
                fprintf(stderr, "%s", status.description().c_str());
                runtime_assert(false);
            }
            runtime_assert(buffer.size() == number);
            runtime_assert(buffer[0].name == object.name);
            runtime_assert(buffer[0].key_map == object.key_map);
        };

        auto l2 = [&]() {
            rapidjson::Document h;
            h.Parse(str.c_str());
            runtime_assert(h.IsArray() && h.Size() == number);
            runtime_assert(h[0].IsObject() && h[0].HasMember("why_not"));

            std::vector<MyClass> buffer(h.Size());
            for (unsigned i = 0; i < number; ++i)
            {
                buffer[i].name = h[i]["name"].GetString();
                auto&& key_map = h[i]["key_map"];
                for (auto it = key_map.MemberBegin(), end = key_map.MemberEnd(); it != end; ++it)
                {
                    buffer[i].key_map.emplace(it->name.GetString(), it->value.GetString());
                }
                auto&& matrix = h[i]["matrix"];
                buffer[i].matrix.resize(matrix.Size());
                for (unsigned j = 0; j < buffer[i].matrix.size(); ++j)
                {
                    buffer[i].matrix[j].resize(matrix[j].Size());
                    for (unsigned k = 0; k < buffer[i].matrix[j].size(); ++k)
                    {
                        buffer[i].matrix[j][k] = matrix[j][k].GetDouble();
                    }
                }
            }
        };

        auto l3 = [&]() {
            Json::Value v;
            Json::Reader r;
            runtime_assert(r.parse(str, v));
            runtime_assert(v.isArray() && v.size() == number);
            runtime_assert(v[0].isObject() && v[0]["name"] == object.name);
        };

        fprintf(stderr,
                "Reading %u numbers of custom objects (eventually into C++)\n\n\tStaticJSON: %lld "
                "ms\n\tRapidJSON DOM: "
                "%lld ms\n\tJsonCpp: %lld ms\n\n",
                number,
                enable_staticjson ? how_many_milliseconds(l1, 1) : 0ll,
                enable_rapidjson ? how_many_milliseconds(l2, 1) : 0ll,
                enable_jsoncpp ? how_many_milliseconds(l3, 1) : 0ll);

        fflush(stderr);
    }
}

int main()
{
    // benchmark_array_of_single_int(1000000);
    // benchmark_array_of_doubles(1000000);
    benchmark_custom_class(1000000);
    return 0;
}