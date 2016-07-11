#pragma once
#include <staticjson/basic.hpp>

#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

namespace staticjson
{
template <class ArrayType>
class ArrayHandler : public BaseHandler
{
public:
    typedef typename ArrayType::value_type ElementType;

protected:
    ElementType element;
    Handler<ElementType> internal;
    ArrayType* m_value;
    int depth = 0;

protected:
    void set_element_error() { the_error.reset(new error::ArrayElementError(m_value->size())); }

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
            m_value->emplace_back(std::move(element));
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
    explicit ArrayHandler(ArrayType* value) : element(), internal(&element), m_value(value) {}

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

    bool write(IHandler* output) const override
    {
        if (!output->StartArray())
            return false;
        for (auto&& e : *m_value)
        {
            Handler<ElementType> h(&e);
            if (!h.write(output))
                return false;
        }
        return output->EndArray(m_value->size());
    }
};

template <class T>
class Handler<std::vector<T>> : public ArrayHandler<std::vector<T>>
{
public:
    explicit Handler(std::vector<T>* value) : ArrayHandler<std::vector<T>>(value) {}

    std::string type_name() const override
    {
        return "std::vector<" + this->internal.type_name() + ">";
    }
};

template <class T>
class Handler<std::deque<T>> : public ArrayHandler<std::deque<T>>
{
public:
    explicit Handler(std::deque<T>* value) : ArrayHandler<std::deque<T>>(value) {}

    std::string type_name() const override
    {
        return "std::deque<" + this->internal.type_name() + ">";
    }
};

template <class T>
class Handler<std::list<T>> : public ArrayHandler<std::list<T>>
{
public:
    explicit Handler(std::list<T>* value) : ArrayHandler<std::list<T>>(value) {}

    std::string type_name() const override
    {
        return "std::list<" + this->internal.type_name() + ">";
    }
};
}