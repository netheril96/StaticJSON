#pragma once
#include <rapidjson/document.h>
#include <staticjson/basic.hpp>

#include <array>

namespace staticjson
{
class JSONHandler : public BaseHandler
{
private:
    static const int MAX_DEPTH = 16;

    std::array<Value, MAX_DEPTH> m_stack;
    Value* m_value;
    MemoryPoolAllocator* m_alloc;
    int m_depth;

private:
    bool stack_push();
    void stack_pop();
    Value& stack_top();
    bool postprocess();
    bool set_corrupted_dom();

public:
    explicit JSONHandler(Value* v, MemoryPoolAllocator* a);

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

    virtual bool write(IHandler* output) const override;

    virtual void reset() override;

    void generate_schema(Value& output, MemoryPoolAllocator&) const override { output.SetObject(); }
};

template <>
class Handler<Document> : public JSONHandler
{
public:
    explicit Handler(Document* h) : JSONHandler(h, &h->GetAllocator()) {}
};

bool value_to_pretty_file(std::FILE* fp, const Value& v);
}