// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef STATICJSON_NULLABLE_TYPES_HPP_29A4C106C1B1
#define STATICJSON_NULLABLE_TYPES_HPP_29A4C106C1B1

#include <staticjson/utility.hpp>
#include <staticjson/base.hpp>
#include <staticjson/error.hpp>

#if STATICJSON_HAS_MODERN_TYPES
#include <memory>
#endif

namespace staticjson {

template <class ValueType, class Derived>
class NullableBaseSAXEventHandler {
private:
    utility::scoped_ptr<SAXEventHandler<ValueType> > internal_handler;
    int depth;

    void initialize()
    {
        if (static_cast<Derived*>(this)->IsNull()) {
            ValueType* ptr = static_cast<Derived*>(this)->Initialize();
            internal_handler.reset(new SAXEventHandler<ValueType>(ptr));
        }
    }

    bool checked_event_forwarding(bool success)
    {
        return success;
    }

protected:
    explicit NullableBaseSAXEventHandler()
        : internal_handler(0)
        , depth(0)
    {
    }

public:
    bool Null()
    {
        if (depth == 0) {
            static_cast<Derived*>(this)->SetNull();
            return true;
        } else {
            initialize();
            return checked_event_forwarding(internal_handler->Null());
        }
    }

    bool Bool(bool b)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Bool(b));
    }

    bool Int(int i)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Int(i));
    }

    bool Uint(unsigned i)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Uint(i));
    }

    bool Int64(utility::int64_t i)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Int64(i));
    }

    bool Uint64(utility::uint64_t i)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Uint64(i));
    }

    bool Double(double i)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Double(i));
    }

    bool String(const char* str, SizeType len, bool copy)
    {
        initialize();
        return checked_event_forwarding(internal_handler->String(str, len, copy));
    }

    bool Key(const char* str, SizeType len, bool copy)
    {
        initialize();
        return checked_event_forwarding(internal_handler->Key(str, len, copy));
    }

    bool StartObject()
    {
        initialize();
        ++depth;
        return internal_handler->StartObject();
    }

    bool EndObject(SizeType len)
    {
        initialize();
        --depth;
        return checked_event_forwarding(internal_handler->EndObject(len));
    }

    bool StartArray()
    {
        initialize();
        ++depth;
        return internal_handler->StartArray();
    }

    bool EndArray(SizeType len)
    {
        initialize();
        --depth;
        return checked_event_forwarding(internal_handler->EndArray(len));
    }

    bool has_error() const
    {
        return !internal_handler.empty() && internal_handler->has_error();
    }

    bool reap_error(error::ErrorStack& errs)
    {
        if (internal_handler.empty())
            return false;
        return internal_handler->reap_error(errs);
    }

    void prepare_for_reuse()
    {
        depth = 0;
        internal_handler.reset();
    }
};

#if STATICJSON_HAS_MODERN_TYPES
template <class T>
class SAXEventHandler<std::shared_ptr<T> >
    : public NullableBaseSAXEventHandler<T, SAXEventHandler<std::shared_ptr<T> > > {

public:
    typedef std::shared_ptr<T> smart_pointer_type;

private:
    smart_pointer_type* m_value;

public:
    explicit SAXEventHandler(smart_pointer_type* v)
        : m_value(v)
    {
    }

    bool IsNull() const
    {
        return m_value->get() == 0;
    }

    T* Initialize()
    {
        std::make_shared<T>().swap(*m_value);
        return m_value->get();
    }

    void SetNull()
    {
        m_value->reset();
    }
};

template <class Writer, class T>
struct Serializer<Writer, std::shared_ptr<T> > {
    void operator()(Writer& w, const std::shared_ptr<T>& ptr) const
    {
        if (!ptr)
            w.Null();
        else
            Serializer<Writer, T>()(w, *ptr);
    }
};

namespace utility {
    namespace traits {
        template <class T>
        struct is_simple_type<std::shared_ptr<T> > : public is_simple_type<T> {
        };
    }
}

template <class T, class Deleter>
class SAXEventHandler<std::unique_ptr<T, Deleter> >
    : public NullableBaseSAXEventHandler<T, SAXEventHandler<std::unique_ptr<T, Deleter> > > {

public:
    typedef std::unique_ptr<T, Deleter> smart_pointer_type;

private:
    smart_pointer_type* m_value;

public:
    explicit SAXEventHandler(smart_pointer_type* v)
        : m_value(v)
    {
    }

    bool IsNull() const
    {
        return m_value->get() == 0;
    }

    T* Initialize()
    {
        m_value->reset(new T());
        return m_value->get();
    }

    void SetNull()
    {
        m_value->reset();
    }
};

template <class Writer, class T, class Allocator>
struct Serializer<Writer, std::unique_ptr<T, Allocator> > {
    void operator()(Writer& w, const std::unique_ptr<T, Allocator>& ptr) const
    {
        if (!ptr)
            w.Null();
        else
            Serializer<Writer, T>()(w, *ptr);
    }
};

namespace utility {
    namespace traits {
        template <class T, class Allocator>
        struct is_simple_type<std::unique_ptr<T, Allocator> > : public is_simple_type<T> {
        };
    }
}

#endif
}
#endif
