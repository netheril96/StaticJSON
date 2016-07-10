#pragma once

#include <staticjson/error.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

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

    virtual bool Null() = 0;

    virtual bool Bool(bool) = 0;

    virtual bool Int(int) = 0;

    virtual bool Uint(unsigned) = 0;

    virtual bool Int64(std::int64_t) = 0;

    virtual bool Uint64(std::uint64_t) = 0;

    virtual bool Double(double) = 0;

    virtual bool String(const char*, SizeType, bool) = 0;

    virtual bool StartObject() = 0;

    virtual bool Key(const char*, SizeType, bool) = 0;

    virtual bool EndObject(SizeType) = 0;

    virtual bool StartArray() = 0;

    virtual bool EndArray(SizeType) = 0;

    virtual bool RawNumber() = 0;

    virtual void PrepareForReuse() = 0;
};

class BaseHandler : public IHandler, private NonMobile
{
protected:
    std::unique_ptr<error::ErrorBase> the_error;
    bool parsed = false;

protected:
    bool set_out_of_range(const char* actual_type);
    bool set_type_mismatch(const char* actual_type);

    virtual void reset() {}

public:
    BaseHandler() {}

    virtual ~BaseHandler();

    virtual std::string type_name() = 0;

    virtual bool Null() override { return set_type_mismatch("null"); }

    virtual bool Bool(bool) override { return set_type_mismatch("bool"); }

    virtual bool Int(int) override { return set_type_mismatch("int"); }

    virtual bool Uint(unsigned) override { return set_type_mismatch("unsigned"); }

    virtual bool Int64(std::int64_t) override { return set_type_mismatch("int64_t"); }

    virtual bool Uint64(std::uint64_t) override { return set_type_mismatch("uint64_t"); }

    virtual bool Double(double) override { return set_type_mismatch("double"); }

    virtual bool String(const char*, SizeType, bool) override
    {
        return set_type_mismatch("string");
    }

    virtual bool StartObject() override { return set_type_mismatch("object"); }

    virtual bool Key(const char*, SizeType, bool) override { return set_type_mismatch("object"); }

    virtual bool EndObject(SizeType) override { return set_type_mismatch("object"); }

    virtual bool StartArray() override { return set_type_mismatch("array"); }

    virtual bool EndArray(SizeType) override { return set_type_mismatch("array"); }

    bool HasError() const { return bool(the_error); }

    virtual bool ReapError(error::ErrorStack& errs)
    {
        if (!the_error)
            return false;
        errs.push(the_error.release());
        return true;
    }

    bool is_parsed() const { return parsed; }

    bool RawNumber() override { return true; }

    void PrepareForReuse() override
    {
        the_error.reset();
        parsed = false;
        reset();
    }

    virtual bool write(IHandler* output) const = 0;
};

class ObjectHandler : public BaseHandler
{
protected:
    struct FlaggedHandler
    {
        std::unique_ptr<BaseHandler> handler;
        unsigned flags;
    };

protected:
    std::unordered_map<std::string, FlaggedHandler> internals;
    FlaggedHandler* current = nullptr;
    std::string current_name;
    int depth = 0;

protected:
    bool precheck(const char* type);
    bool postcheck(bool success);
    void set_missing_required(const char* name);

public:
    ObjectHandler();

    ~ObjectHandler();

    std::string type_name() override;

    virtual bool Null() override;

    virtual bool Bool(bool) override;

    virtual bool Int(int) override;

    virtual bool Uint(unsigned) override;

    virtual bool Int64(std::int64_t) override;

    virtual bool Uint64(std::uint64_t) override;

    virtual bool Double(double) override;

    virtual bool String(const char*, SizeType, bool) override;

    virtual bool StartObject() override;

    virtual bool Key(const char*, SizeType, bool) override;

    virtual bool EndObject(SizeType) override;

    virtual bool StartArray() override;

    virtual bool EndArray(SizeType) override;
};

template <class T>
class Handler : public ObjectHandler
{
private:
    T* m_value;

public:
    explicit Handler(T* t) : m_value(t) {}
};
}