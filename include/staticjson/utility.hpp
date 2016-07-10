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

#ifndef STATICJSON_UTILITY_HPP_29A4C106C1B1
#define STATICJSON_UTILITY_HPP_29A4C106C1B1

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#if STATICJSON_MODERN_COMPILER
#define STATICJSON_HAS_MODERN_TYPES 1
#define STATICJSON_HAS_RVALUE 1
#define STATICJSON_HAS_NOEXCEPT 1
#define STATICJSON_HAS_VARIADIC_TEMPLATE 1
#define STATICJSON_HAS_EXPLICIT_OPERATOR 1
#endif

#if STATICJSON_HAS_RVALUE
#define STATICJSON_MOVE(x) std::move(x)
#define DISABLE_COPY_MOVE(cls) private
#else
#define STATICJSON_MOVE(x) (x)
#endif

#if STATICJSON_HAS_NOEXCEPT
#define STATICJSON_NOEXCEPT
#define STATICJSON_MOVE_IF_NOEXCEPT(x) std::move_if_noexcept(x)
#else
#define STATICJSON_NOEXCEPT
#define STATICJSON_MOVE_IF_NOEXCEPT(x) STATICJSON_MOVE(x)
#endif

namespace staticjson
{
namespace utility
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
    typedef long long int64_t;
    typedef unsigned long long uint64_t;

    const std::size_t default_buffer_size = 256;

    namespace traits
    {
        struct true_type
        {
            static const bool value = true;
        };

        struct false_type
        {
            static const bool value = false;
        };

        template <class T>
        struct is_simple_type : public false_type
        {
        };

        template <>
        struct is_simple_type<bool> : public true_type
        {
        };

        template <>
        struct is_simple_type<char> : public true_type
        {
        };

        template <>
        struct is_simple_type<int> : public true_type
        {
        };

        template <>
        struct is_simple_type<unsigned> : public true_type
        {
        };

        template <>
        struct is_simple_type<std::int64_t> : public true_type
        {
        };

        template <>
        struct is_simple_type<std::uint64_t> : public true_type
        {
        };

        template <>
        struct is_simple_type<std::string> : public true_type
        {
        };
    }

    template <class T>
    struct default_deleter
    {
        void operator()(T* ptr) const { delete ptr; }
    };

    struct file_closer
    {
        void operator()(std::FILE* fp) const
        {
            if (fp)
                std::fclose(fp);
        }
    };

    template <class T, class Deleter = default_deleter<T>>
    class scoped_ptr
    {
    private:
        T* ptr;

        scoped_ptr(const scoped_ptr&);
        scoped_ptr& operator=(const scoped_ptr&);

#if STATICJSON_HAS_RVALUE
        scoped_ptr(scoped_ptr&&);
        scoped_ptr& operator==(scoped_ptr&&);
#endif

    public:
        typedef T element_type;
        typedef T* pointer_type;
        typedef T& reference_type;

        explicit scoped_ptr() : ptr(0) {}

        explicit scoped_ptr(pointer_type t) : ptr(t) {}

        ~scoped_ptr() { Deleter()(ptr); }

        pointer_type get() const STATICJSON_NOEXCEPT { return ptr; }

        pointer_type operator->() const STATICJSON_NOEXCEPT { return ptr; }

        pointer_type release() STATICJSON_NOEXCEPT
        {
            T* result = ptr;
            ptr = 0;
            return result;
        }

        reference_type operator*() const { return *ptr; }

        void reset(pointer_type p = 0)
        {
            Deleter()(ptr);
            ptr = p;
        }

        void swap(scoped_ptr& that) STATICJSON_NOEXCEPT { std::swap(ptr, that.ptr); }

        bool empty() const STATICJSON_NOEXCEPT { return ptr == 0; }
    };

    inline bool string_equal(const char* str1, std::size_t len1, const char* str2, std::size_t len2)
    {
        return len1 == len2 && std::equal(str1, str1 + len1, str2);
    }

   
    // The standard std::stack is insufficient because it cannot handle noncopyable types in c++03
    template <class T, std::size_t num_elements_per_node>
    class stack
    {
    private:
        // The node is always allocated with new, so no need for alignment tuning
        struct node
        {
            char raw_storage[sizeof(T) * num_elements_per_node];
            node* next;
        };

        node* head;
        std::size_t current_size;
        std::size_t total_size;

    private:
        void deallocate_current_node()
        {
            for (std::size_t i = 0; i < current_size; ++i)
                reinterpret_cast<T*>(head->raw_storage + sizeof(T) * i)->~T();

            current_size = num_elements_per_node;
            node* next = head->next;
            operator delete(head);
            head = next;
        }

    private:
        stack(const stack&);
        stack& operator=(const stack&);

    public:
        explicit stack() : head(0), current_size(num_elements_per_node), total_size(0) {}

        T& emplace()
        {
            if (current_size == num_elements_per_node)
            {
                // operator new always return memory maximumly aligned
                node* new_node = static_cast<node*>(operator new(sizeof(*new_node)));
                new_node->next = head;
                head = new_node;
                current_size = 0;
            }
            T* result = new (head->raw_storage + sizeof(T) * current_size) T();
            ++current_size;
            ++total_size;
            return *result;
        }

        void push(const T& value) { emplace() = value; }

        void push(T& value) { emplace() = value; }

        T& top()
        {
            return const_cast<T&>(static_cast<const stack<T, num_elements_per_node>*>(this)->top());
        }

        const T& top() const
        {
            assert(!empty());

            // The below code triggers a warning about strict aliasing in some versions of gcc
            // That is a false positive, because character type is allowed to alias any type
            if (current_size > 0)
                return *reinterpret_cast<T*>(head->raw_storage + sizeof(T) * (current_size - 1));
            else
                return *reinterpret_cast<T*>(head->next->raw_storage
                                             + sizeof(T) * (num_elements_per_node - 1));
        }

        void pop()
        {
            assert(!empty());
            if (current_size == 0)
                deallocate_current_node();
            top().~T();
            --current_size;
            --total_size;
        }

        void clear()
        {
            while (head)
                deallocate_current_node();
            head = 0;
        }

        bool empty() const STATICJSON_NOEXCEPT { return total_size == 0; }

        std::size_t size() const STATICJSON_NOEXCEPT { return total_size; }

        ~stack() { clear(); }
    };
}

namespace internal
{
    const signed char ARRAY = 0;
    const signed char OBJECT = -1;
}
}

#endif
