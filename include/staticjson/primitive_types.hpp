#pragma once
#include <staticjson/basic.hpp>

#include <string>

namespace staticjson
{
template <>
class Handler<int> : public BaseHandler
{
private:
    int* m_value;

public:
    explicit Handler(int* i) : m_value(i) {}

    std::string type_name() const override { return "int"; }

    bool Int(int i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Uint(unsigned i) override
    {
        if (i > static_cast<unsigned>(std::numeric_limits<int>::max()))
            return set_out_of_range("unsigned");
        this->parsed = true;
        *m_value = static_cast<int>(i);
        return true;
    }

    bool Int64(std::int64_t i) override
    {
        if (i > static_cast<std::int64_t>(std::numeric_limits<int>::max())
            || i < static_cast<std::int64_t>(std::numeric_limits<int>::min()))
            return set_out_of_range("int64_t");
        this->parsed = true;
        *m_value = static_cast<int>(i);
        return true;
    }

    bool Uint64(std::uint64_t i) override
    {
        if (i > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
            return set_out_of_range("uint64_t");
        this->parsed = true;
        *m_value = static_cast<int>(i);
        return true;
    }

    bool write(IHandler* output) const override { return output->Int(*m_value); }
};

template <>
class Handler<unsigned> : public BaseHandler
{
private:
    unsigned* m_value;

public:
    explicit Handler(unsigned* v) : m_value(v) {}

    bool Int(int i) override
    {
        if (i < 0)
            return set_out_of_range("int");
        this->parsed = true;
        *m_value = static_cast<unsigned>(i);
        return true;
    }

    bool Uint(unsigned i) override
    {
        this->parsed = true;
        *m_value = i;
        return true;
    }

    bool Int64(std::int64_t i) override
    {
        if (i < 0 || i > static_cast<std::int64_t>(std::numeric_limits<unsigned>::max()))
            return set_out_of_range("int64_t");
        this->parsed = true;
        *m_value = static_cast<unsigned>(i);
        return true;
    }

    bool Uint64(std::uint64_t i) override
    {
        if (i > static_cast<std::uint64_t>(std::numeric_limits<unsigned>::max()))
            return set_out_of_range("uint64_t");
        this->parsed = true;
        *m_value = static_cast<unsigned>(i);
        return true;
    }

    std::string type_name() const override { return "unsigned"; }

    bool write(IHandler* out) const override { return out->Uint(*m_value); }
};

template <>
class Handler<std::int64_t> : public BaseHandler
{
private:
    std::int64_t* m_value;

public:
    explicit Handler(std::int64_t* v) : m_value(v) {}

    bool Int(int i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Uint(unsigned i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Int64(std::int64_t i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Uint64(std::uint64_t i) override
    {
        if (i > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
            return set_out_of_range("uint64_t");
        *m_value = static_cast<std::int64_t>(i);
        this->parsed = true;
        return true;
    }

    std::string type_name() const override { return "int64_t"; }

    bool write(IHandler* out) const override { return out->Int64(*m_value); }
};

template <>
class Handler<std::uint64_t> : public BaseHandler
{
private:
    std::uint64_t* m_value;

public:
    explicit Handler(std::uint64_t* v) : m_value(v) {}

    bool Int(int i) override
    {
        if (i < 0)
            return set_out_of_range("int");
        *m_value = static_cast<std::uint64_t>(i);
        this->parsed = true;
        return true;
    }

    bool Uint(unsigned i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Int64(std::int64_t i) override
    {
        if (i < 0)
            return set_out_of_range("int64_t");
        *m_value = static_cast<std::uint64_t>(i);
        this->parsed = true;
        return true;
    }

    bool Uint64(std::uint64_t i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    std::string type_name() const override { return "uint64_t"; }

    bool write(IHandler* out) const override { return out->Uint64(*m_value); }
};

template <>
class Handler<double> : public BaseHandler
{
private:
    double* m_value;

public:
    explicit Handler(double* v) : m_value(v) {}

    bool Int(int i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Uint(unsigned i) override
    {
        *m_value = i;
        this->parsed = true;
        return true;
    }

    bool Int64(std::int64_t i) override
    {
        const std::int64_t threshold = 1LL << 53;
        if (i > threshold || i < -threshold)
            return this->set_out_of_range("int64_t");
        // the maximum value of double is much larger, but we want to prevent precision loss

        *m_value = static_cast<double>(i);
        this->parsed = true;
        return true;
    }

    bool Uint64(std::uint64_t i) override
    {
        const std::uint64_t threshold = 1ULL << 53;
        if (i > threshold)
            return this->set_out_of_range("uint64_t");

        *m_value = static_cast<double>(i);
        this->parsed = true;
        return true;
    }

    bool Double(double d) override
    {
        *m_value = d;
        this->parsed = true;
        return true;
    }

    std::string type_name() const override { return "double"; }

    bool write(IHandler* out) const override { return out->Double(*m_value); }
};

template <>
class Handler<std::string> : public BaseHandler
{
private:
    std::string* m_value;

public:
    explicit Handler(std::string* v) : m_value(v) {}

    bool String(const char* str, SizeType length, bool) override
    {
        m_value->assign(str, length);
        this->parsed = true;
        return true;
    }

    std::string type_name() const override { return "string"; }

    bool write(IHandler* out) const override
    {
        return out->String(m_value->data(), SizeType(m_value->size()), true);
    }
};
}
