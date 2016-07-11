#include <staticjson/staticjson.hpp>

#define INSTANTIATE(type)                                                                          \
    {                                                                                              \
        staticjson::Handler<type> h(nullptr);                                                      \
    }

// Ensure that the template classes can be instantiated without compile time error
void instantiate_all_types()
{
    INSTANTIATE(int8_t)
    INSTANTIATE(uint8_t)
    INSTANTIATE(char)
    INSTANTIATE(bool)
    INSTANTIATE(int)
    INSTANTIATE(unsigned)
    INSTANTIATE(std::int64_t)
    INSTANTIATE(std::uint64_t)
    INSTANTIATE(std::string)
    INSTANTIATE(std::vector<float>)
    INSTANTIATE(std::vector<double>)
    INSTANTIATE(std::deque<std::vector<std::list<std::string>>>);
}

#undef INSTANTIATE