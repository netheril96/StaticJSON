#include <deque>
#include <staticjson/staticjson.hpp>

// This class is used to test custom conversion functions in StaticJSON.

template <class T>
class Array
{
private:
    T* m_data;
    size_t m_size;

public:
    explicit Array() : m_data(nullptr), m_size(0) {}
    explicit Array(size_t size) : m_size(size) { m_data = new T[size]; }
    Array(Array&& that) noexcept
    {
        m_data = that.m_data;
        m_size = that.m_size;
        that.m_data = nullptr;
        that.m_size = 0;
    }
    Array& operator=(Array&& that) noexcept
    {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
        return *this;
    }
    const T& operator[](size_t i) const { return m_data[i]; }
    T& operator[](size_t i) { return m_data[i]; }
    size_t size() const { return m_size; }
    const T& back() const { return m_data[m_size - 1]; }
    T& back() { return m_data[m_size - 1]; }
    const T& front() const { return m_data[0]; }
    T& front() { return m_data[0]; }
    bool empty() const { return m_size == 0; }
};

namespace staticjson
{
template <class T>
struct Converter<Array<T>>
{
    typedef std::deque<T> shadow_type;

    static std::unique_ptr<ErrorBase> from_shadow(const shadow_type& shadow, Array<T>& value)
    {
        value = Array<T>(shadow.size());
        for (size_t i = 0; i < shadow.size(); ++i)
        {
            value[i] = shadow[i];
        }
        return nullptr;
    }

    static void to_shadow(const Array<T>& value, shadow_type& shadow)
    {
        shadow.resize(value.size());
        for (size_t i = 0; i < shadow.size(); ++i)
        {
            shadow[i] = value[i];
        }
    }
};
}
