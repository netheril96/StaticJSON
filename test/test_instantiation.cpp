#include <staticjson/document.hpp>
#include <staticjson/staticjson.hpp>

#define INSTANTIATE(type)                                                                          \
    {                                                                                              \
        staticjson::Handler<type> h(nullptr);                                                      \
    }

// Ensure that the template classes can be instantiated without compile time error
void instantiate_all_types()
{
    INSTANTIATE(char)
    INSTANTIATE(bool)
    INSTANTIATE(int)
    INSTANTIATE(unsigned)
    INSTANTIATE(std::int64_t)
    INSTANTIATE(std::uint64_t)
    INSTANTIATE(std::string)
    typedef std::array<long long, 10> my_array;
    INSTANTIATE(my_array)
    INSTANTIATE(std::vector<float>)
    INSTANTIATE(std::vector<double>)
    INSTANTIATE(std::deque<std::shared_ptr<std::unique_ptr<std::vector<std::list<std::string>>>>>);
    INSTANTIATE(staticjson::Document);
    INSTANTIATE(decltype(nullptr));

    typedef std::tuple<int,
                       double,
                       std::array<rapidjson::Document, 5>,
                       std::vector<std::string>,
                       std::unique_ptr<std::deque<rapidjson::Document>>>
        complex_tuple_type;
    INSTANTIATE(complex_tuple_type);
}

#undef INSTANTIATE
