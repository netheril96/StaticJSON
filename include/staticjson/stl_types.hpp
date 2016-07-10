#pragma once
#include <staticjson/basic.hpp>

#include <deque>
#include <list>
#include <map>
#include <stack>
#include <unordered_map>
#include <vector>

namespace staticjson
{
template <class ElementType>
class BaseArrayHandler : public BaseHandler
{
protected:
    virtual std::size_t get_current_size() const = 0;
    virtual void push(ElementType&&) = 0;

protected:
    ElementType element;
    Handler<ElementType> internal;
    int depth = 0;

protected:
    void set_element_error() { the_error.reset(new error::ArrayElementError(get_current_size())); }

    bool precheck(const char* type)
    {
        if (depth <= 0)
        {
            the_error.reset(new error::TypeMismatchError(type_name(), type));
            return false;
        }
        return true;
    }

    bool postcheck(bool success)
    {
        if (!success)
        {
            set_element_error();
            return false;
        }
        if (depth == 1 && internal.is_parsed())
        {
            push(std::move(element));
            element = ElementType();
            internal.prepare_for_reuse();
        }
        return true;
    }

    void reset() override
    {
        element = ElementType();
        internal.prepare_for_reuse();
        depth = 0;
    }

public:
    explicit BaseArrayHandler() : element(), internal(&element) {}

    bool Null() override { return precheck("null") && postcheck(internal.Null()); }

    bool Bool(bool b) override { return precheck("bool") && postcheck(internal.Bool(b)); }

    bool Int(int i) override { return precheck("int") && postcheck(internal.Int(i)); }

    bool Uint(unsigned i) override { return precheck("unsigned") && postcheck(internal.Uint(i)); }

    bool Int64(std::int64_t i) override
    {
        return precheck("int64_t") && postcheck(internal.Int64(i));
    }

    bool Uint64(std::uint64_t i) override
    {
        return precheck("uint64_t") && postcheck(internal.Uint64(i));
    }

    bool Double(double d) override { return precheck("double") && postcheck(internal.Double(d)); }

    bool String(const char* str, SizeType length, bool copy) override
    {
        return precheck("string") && postcheck(internal.String(str, length, copy));
    }

    bool Key(const char* str, SizeType length, bool copy) override
    {
        return precheck("object") && postcheck(internal.Key(str, length, copy));
    }

    bool StartObject() override { return precheck("object") && postcheck(internal.StartObject()); }

    bool EndObject(SizeType length) override
    {
        return precheck("object") && postcheck(internal.EndObject(length));
    }

    bool StartArray() override
    {
        ++depth;
        if (depth > 1 && !internal.StartArray())
        {
            set_element_error();
            return false;
        }
        return true;
    }

    bool EndArray(SizeType length) override
    {
        --depth;

        // When depth > 1, this event should be forwarded to the element
        if (depth > 1 && !internal.EndArray(length))
        {
            set_element_error();
            return false;
        }

        postcheck(true);
        this->parsed = true;
        return true;
    }

    bool reap_error(ErrorStack& stk) override
    {
        if (!the_error)
            return false;
        stk.push(the_error.release());
        internal.reap_error(stk);
        return true;
    }
};

template <class ElementType>
class Handler<std::vector<ElementType>> : public BaseArrayHandler<ElementType>
{
private:
    std::vector<ElementType>* m_value;

protected:
    void push(ElementType&& e) override { m_value->push_back(std::move(e)); }
    std::size_t get_current_size() const override { return m_value->size(); }

public:
    explicit Handler(std::vector<ElementType>* value) : m_value(value) {}

    bool write(IHandler*) const override
    {
        std::terminate();    // Not implemented for now
    }

    std::string type_name() const override
    {
        return "std::vector<" + this->internal.type_name() + ">";
    }
};
}