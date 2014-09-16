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

#ifndef AUTOJSONCXX_UTILITY_HPP_29A4C106C1B1
#define AUTOJSONCXX_UTILITY_HPP_29A4C106C1B1

#include <string>
#include <cstring>
#include <cstddef>
#include <utility>
#include <algorithm>

#if AUTOJSONCXX_MODERN_COMPILER
#define AUTOJSONCXX_HAS_MODERN_TYPES 1
#define AUTOJSONCXX_HAS_RVALUE 1
#define AUTOJSONCXX_HAS_NOEXCEPT 1
#define AUTOJSONCXX_HAS_VARIADIC_TEMPLATE 1
#define AUTOJSONCXX_HAS_EXPLICIT_OPERATOR 1
#endif

#if AUTOJSONCXX_HAS_RVALUE
#define AUTOJSONCXX_MOVE(x) std::move(x)
#else
#define AUTOJSONCXX_MOVE(x) (x)
#endif

#if AUTOJSONCXX_HAS_NOEXCEPT
#define AUTOJSONCXX_NOEXCEPT noexcept
#define AUTOJSONCXX_MOVE_IF_NOEXCEPT(x) std::move_if_noexcept(x)
#else
#define AUTOJSONCXX_NOEXCEPT
#define AUTOJSONCXX_MOVE_IF_NOEXCEPT(x) AUTOJSONCXX_MOVE(x)
#endif

namespace autojsoncxx {
namespace utility {

    typedef unsigned int SizeType;
    typedef long long int64_t;
    typedef unsigned long long uint64_t;

    const std::size_t default_buffer_size = 256;

    namespace traits {
        struct true_type {
            static const bool value = true;
        };

        struct false_type {
            static const bool value = false;
        };

        template <class T>
        struct is_simple_type : public false_type {
        };

        template <>
        struct is_simple_type<bool> : public true_type {
        };

        template <>
        struct is_simple_type<char> : public true_type {
        };

        template <>
        struct is_simple_type<int> : public true_type {
        };

        template <>
        struct is_simple_type<unsigned> : public true_type {
        };

        template <>
        struct is_simple_type<int64_t> : public true_type {
        };

        template <>
        struct is_simple_type<uint64_t> : public true_type {
        };

        template <>
        struct is_simple_type<std::string> : public true_type {
        };
    }

    template <class T>
    class scoped_ptr {
    private:
        T* ptr;

        scoped_ptr(const scoped_ptr&);
        scoped_ptr& operator=(const scoped_ptr&);

#if AUTOJSONCXX_HAS_RVALUE
        scoped_ptr(scoped_ptr&&);
        scoped_ptr& operator==(scoped_ptr&&);
#endif

    public:
        typedef T element_type;
        typedef T* pointer_type;
        typedef T& reference_type;

        explicit scoped_ptr()
            : ptr(0)
        {
        }

        explicit scoped_ptr(pointer_type t)
            : ptr(t)
        {
        }

        ~scoped_ptr()
        {
            delete ptr;
        }

        pointer_type get() const AUTOJSONCXX_NOEXCEPT
        {
            return ptr;
        }

        pointer_type operator->() const AUTOJSONCXX_NOEXCEPT
        {
            return ptr;
        }

        pointer_type release() AUTOJSONCXX_NOEXCEPT
        {
            T* result = ptr;
            ptr = 0;
            return result;
        }

        reference_type operator*() const
        {
            return *ptr;
        }

        void reset(pointer_type p)
        {
            delete ptr;
            ptr = p;
        }

        void swap(scoped_ptr& that) AUTOJSONCXX_NOEXCEPT
        {
            std::swap(ptr, that.ptr);
        }

        bool empty() const AUTOJSONCXX_NOEXCEPT
        {
            return ptr == 0;
        }
    };

    inline bool string_equal(const char* str1, size_t len1, const char* str2)
    {
        size_t len = std::strlen(str2);
        return len == len1 && memcmp(str1, str2, len) == 0;
    }
}
}

#endif
