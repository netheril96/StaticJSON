#pragma once

#include <rapidjson/document.h>
#include <staticjson/error.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
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

    virtual bool RawNumber(const char*, SizeType, bool);

    virtual void prepare_for_reuse() = 0;
};

using rapidjson::Document;
using rapidjson::Value;

typedef rapidjson::MemoryPoolAllocator<> MemoryPoolAllocator;

class BaseHandler : public IHandler, private NonMobile
{
    friend class NullableHandler;

protected:
    std::unique_ptr<ErrorBase> the_error;
    bool parsed = false;

protected:
    bool set_out_of_range(const char* actual_type);
    bool set_type_mismatch(const char* actual_type);

    virtual void reset() {}

public:
    BaseHandler() {}

    virtual ~BaseHandler();

    virtual std::string type_name() const = 0;

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

    virtual bool has_error() const { return bool(the_error); }

    virtual bool reap_error(ErrorStack& errs)
    {
        if (!the_error)
            return false;
        errs.push(the_error.release());
        return true;
    }

    bool is_parsed() const { return parsed; }

    void prepare_for_reuse() override
    {
        the_error.reset();
        parsed = false;
        reset();
    }

    virtual bool write(IHandler* output) const = 0;

    virtual void generate_schema(Value& output, MemoryPoolAllocator& alloc) const = 0;
};

struct Flags
{
    static const unsigned Default = 0x0, AllowDuplicateKey = 0x1, Optional = 0x2, IgnoreRead = 0x4,
                          IgnoreWrite = 0x8, DisallowUnknownKey = 0x16;
};

// Forward declaration
template <class T>
class Handler;

class ObjectHandler : public BaseHandler
{
protected:
    struct FlaggedHandler
    {
        std::unique_ptr<BaseHandler> handler;
        unsigned flags;
    };

protected:
    std::map<std::string, FlaggedHandler> internals;
    FlaggedHandler* current = nullptr;
    std::string current_name;
    int depth = 0;
    unsigned flags = Flags::Default;

protected:
    bool precheck(const char* type);
    bool postcheck(bool success);
    void set_missing_required(const std::string& name);
    void add_handler(std::string&&, FlaggedHandler&&);
    void reset() override;

public:
    ObjectHandler();

    ~ObjectHandler();

    std::string type_name() const override;

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

    virtual bool reap_error(ErrorStack&) override;

    virtual bool write(IHandler* output) const override;

    virtual void generate_schema(Value& output, MemoryPoolAllocator& alloc) const override;

    unsigned get_flags() const { return flags; }

    void set_flags(unsigned f) { flags = f; }

    template <class T>
    void add_property(std::string name, T* pointer, unsigned flags_ = Flags::Default)
    {
        FlaggedHandler fh;
        fh.handler.reset(new Handler<T>(pointer));
        fh.flags = flags_;
        add_handler(std::move(name), std::move(fh));
    }
};

template <class T>
class Handler : public ObjectHandler
{
public:
    explicit Handler(T* t) { t->staticjson_init(this); }
};
}