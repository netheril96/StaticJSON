#pragma once
#include <staticjson/basic.hpp>

namespace staticjson
{
template <>
class Handler<int> : public BaseHandler
{
private:
    int* m_value;

public:
    void set_data(void* p) noexcept override { m_value = static_cast<int*>(p); }

    std::string type_name() override { return "int"; }

    bool Int(int i) noexcept override
    {
        *m_value = i;
        return true;
    }

    bool Uint(unsigned i) noexcept override
    {
        if (i > static_cast<unsigned>(std::numeric_limits<int>::max()))
            return set_out_of_range("unsigned");
        *m_value = static_cast<int>(i);
        return true;
    }

    bool Int64(std::int64_t i) noexcept override
    {
        if (i > static_cast<std::int64_t>(std::numeric_limits<int>::max())
            || i < static_cast<std::int64_t>(std::numeric_limits<int>::min()))
            return set_out_of_range("int64_t");
        *m_value = static_cast<int>(i);
        return true;
    }

    bool Uint64(std::uint64_t i) noexcept override
    {
        if (i > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
            return set_out_of_range("uint64_t");
        *m_value = static_cast<int>(i);
        return true;
    }

    bool write(IHandler* output) const noexcept override { return output->Int(*m_value); }
};
}
