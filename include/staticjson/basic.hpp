#pragma once

#include <staticjson/error.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>

namespace staticjson
{
struct NonMobile
{
    NonMobile() {}
    ~NonMobile() {}
    NonMobile(const NonMobile&) = delete;
    NonMobile(NonMobile&&) = delete;
    NonMobile& operator=(const NonMobile&) = delete;
    NonMobile& operator=(NonMobile&&) = delete;
};

typedef unsigned int SizeType;

class IHandler
{
public:
    IHandler() {}

    virtual ~IHandler();

    virtual bool Null() noexcept = 0;

    virtual bool Bool(bool) noexcept = 0;

    virtual bool Int(int) noexcept = 0;

    virtual bool Uint(unsigned) noexcept = 0;

    virtual bool Int64(std::int64_t) noexcept = 0;

    virtual bool Uint64(std::uint64_t) noexcept = 0;

    virtual bool Double(double) noexcept = 0;

    virtual bool String(const char*, SizeType, bool) noexcept = 0;

    virtual bool StartObject() noexcept = 0;

    virtual bool Key(const char*, SizeType, bool) noexcept = 0;

    virtual bool EndObject(SizeType) noexcept = 0;

    virtual bool StartArray() noexcept = 0;

    virtual bool EndArray(SizeType) noexcept = 0;

    virtual bool HasError() const noexcept = 0;

    virtual bool ReapError(error::ErrorStack& errs) noexcept = 0;

    virtual bool RawNumber() noexcept = 0;

    virtual void PrepareForReuse() noexcept = 0;
};

class BaseHandler : public IHandler, private NonMobile
{
protected:
    std::unique_ptr<error::ErrorBase> the_error;

protected:
    bool set_out_of_range(const char* actual_type);
    bool set_type_mismatch(const char* actual_type);

public:
    BaseHandler() {}

    virtual ~BaseHandler();

    virtual std::string type_name() = 0;

    virtual void set_data(void* p) noexcept = 0;

    virtual bool Null() noexcept override { return set_type_mismatch("null"); }

    virtual bool Bool(bool) noexcept override { return set_type_mismatch("bool"); }

    virtual bool Int(int) noexcept override { return set_type_mismatch("int"); }

    virtual bool Uint(unsigned) noexcept override { return set_type_mismatch("unsigned"); }

    virtual bool Int64(std::int64_t) noexcept override { return set_type_mismatch("int64_t"); }

    virtual bool Uint64(std::uint64_t) noexcept override { return set_type_mismatch("uint64_t"); }

    virtual bool Double(double) noexcept override { return set_type_mismatch("double"); }

    virtual bool String(const char*, SizeType, bool) noexcept override
    {
        return set_type_mismatch("string");
    }

    virtual bool StartObject() noexcept override { return set_type_mismatch("object"); }

    virtual bool Key(const char*, SizeType, bool) noexcept override
    {
        return set_type_mismatch("object");
    }

    virtual bool EndObject(SizeType) noexcept override { return set_type_mismatch("object"); }

    virtual bool StartArray() noexcept override { return set_type_mismatch("array"); }

    virtual bool EndArray(SizeType) noexcept override { return set_type_mismatch("array"); }

    bool HasError() const noexcept override { return bool(the_error); }

    bool ReapError(error::ErrorStack& errs) noexcept override
    {
        if (!the_error)
            return false;
        errs.push(the_error.release());
        return true;
    }

    bool RawNumber() noexcept override { return true; }

    void PrepareForReuse() noexcept override { the_error.reset(); }

    virtual bool write(IHandler* output) const noexcept = 0;
};

class ObjectHandler : public BaseHandler
{
public:
    std::string type_name() override;
};

template <class T>
class Handler : public ObjectHandler
{
private:
    T* pointer;

public:
    void set_data(void* p) noexcept override { pointer = static_cast<T*>(p); }
};
}