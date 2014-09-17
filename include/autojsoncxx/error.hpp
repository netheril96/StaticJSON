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

#ifndef AUTOJSONCXX_ERROR_HPP_29A4C106C1B1
#define AUTOJSONCXX_ERROR_HPP_29A4C106C1B1

#include <autojsoncxx/utility.hpp>
#include <rapidjson/error/error.h>
#include <rapidjson/error/en.h>
#include <cstddef>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <utility>
#include <sstream>

namespace autojsoncxx {
namespace error {

    typedef int error_type;

    static const error_type SUCCESS = 0,
                            OBJECT_MEMEMBER = 1,
                            ARRAY_ELEMENT = 2,
                            MISSING_REQUIRED = 3,
                            TYPE_MISMATCH = 4,
                            NUMBER_OUT_OF_RANGE = 5,
                            ARRAY_LENGTH_MISMATCH = 6,
                            UNKNOWN_FIELD = 7;

    class ErrorStack;

    namespace internal {
        class error_stack_const_iterator;
    }

    class ErrorBase {
    protected:
        explicit ErrorBase()
            : next(0)
        {
        }

    private:
        ErrorBase* next;

        friend class ErrorStack;
        friend class internal::error_stack_const_iterator;

    public:
        virtual error_type type() const = 0;
        virtual bool is_intermediate() const
        {
            return false;
        }
        virtual ~ErrorBase() {}
        virtual std::string description() const = 0;
    };

    class Success : public ErrorBase {
    public:
        explicit Success()
        {
        }
        std::string description() const
        {
            return "No error";
        }
        error_type type() const
        {
            return SUCCESS;
        }
    };

    class IntermediateError : public ErrorBase {
    public:
        bool is_intermediate() const
        {
            return true;
        }
    };

    class ObjectMemberError : public IntermediateError {
    private:
        const char* m_member_name; // Must be a string literal

    public:
        explicit ObjectMemberError(const char* memberName)
            : m_member_name(memberName)
        {
        }

        const char* member_name() const
        {
            return m_member_name;
        }

        std::string description() const
        {
            return std::string("Error at object member with name \"") + m_member_name + "\"";
        }

        error_type type() const
        {
            return OBJECT_MEMEMBER;
        }
    };

    class ArrayElementError : public IntermediateError {
    private:
        std::size_t m_index;

    public:
        explicit ArrayElementError(std::size_t idx)
            : m_index(idx)
        {
        }

        std::size_t index() const
        {
            return m_index;
        }

        std::string description() const
        {
            std::ostringstream ss;
            ss << "Error at array element with index " << m_index;
            return ss.str();
        }

        error_type type() const
        {
            return ARRAY_ELEMENT;
        }
    };

    class RequiredFieldMissingError : public ErrorBase {
    private:
        std::vector<const char*> m_missing_members;

    public:
        explicit RequiredFieldMissingError()
        {
        }

        std::vector<const char*>& missing_members()
        {
            return m_missing_members;
        };

        const std::vector<const char*>& missing_members() const
        {
            return m_missing_members;
        }

        std::string description() const
        {
            std::ostringstream ss;
            ss << "Missing required field(s): ";

            typedef std::vector<const char*>::const_iterator iter;
            for (iter it = m_missing_members.begin(), end = m_missing_members.end();
                 it != end; ++it) {
                ss << '\"' << *it << '\"' << ' ';
            }
            return ss.str();
        }

        error_type type() const
        {
            return MISSING_REQUIRED;
        }
    };

    class TypeMismatchError : public ErrorBase {
    private:
        const char* m_expected_type;
        const char* m_actual_type;

    public:
        explicit TypeMismatchError(const char* expectedType, const char* actualType)
            : m_expected_type(expectedType)
            , m_actual_type(actualType)
        {
        }

        const char* expected_type() const
        {
            return m_expected_type;
        }

        const char* actual_type() const
        {
            return m_actual_type;
        }

        std::string description() const
        {
            std::ostringstream ss;
            ss << "Type mismatch between expected type \""
               << expected_type() << "\" and actual type \""
               << actual_type() << '\"';
            return ss.str();
        }

        error_type type() const
        {
            return TYPE_MISMATCH;
        }
    };

    class NumberOutOfRangeError : public ErrorBase {
        const char* m_expected_type;
        const char* m_actual_type;

    public:
        explicit NumberOutOfRangeError(const char* expectedType, const char* actualType)
            : m_expected_type(expectedType)
            , m_actual_type(actualType)
        {
        }

        const char* expected_type() const
        {
            return m_expected_type;
        }

        const char* actual_type() const
        {
            return m_actual_type;
        }

        std::string description() const
        {
            std::ostringstream ss;
            ss << "Number out of range: expected type is \""
               << expected_type() << "\" but the type needed to"
                                     " represent the number is \""
               << actual_type() << '\"';
            return ss.str();
        }

        error_type type() const
        {
            return NUMBER_OUT_OF_RANGE;
        }
    };

    class ArrayLengthMismatchError : public ErrorBase {
    private:
        std::size_t m_expected_length;
        std::size_t m_actual_length;

    public:
        explicit ArrayLengthMismatchError(std::size_t expectedLength, std::size_t actualLength)
            : m_expected_length(expectedLength)
            , m_actual_length(actualLength)
        {
        }

        std::size_t expected_length() const
        {
            return m_expected_length;
        }

        std::size_t actual_length() const
        {
            return m_actual_length;
        }

        std::string description() const
        {
            std::ostringstream ss;
            ss << "Array length mismatch between expected length "
               << expected_length() << " and actual length "
               << actual_length();
            return ss.str();
        }

        error_type type() const
        {
            return ARRAY_LENGTH_MISMATCH;
        }
    };

    class UnknownFieldError : public ErrorBase {
    private:
        std::string m_name;

    public:
        explicit UnknownFieldError(const char* name, std::size_t length)
            : m_name(name, length)
        {
        }

        const std::string& field_name() const
        {
            return m_name;
        }

        error_type type() const
        {
            return UNKNOWN_FIELD;
        }

        std::string description() const
        {
            return "Unknown field with name:\n\t" + m_name;
        }
    };

    namespace internal {

        class error_stack_const_iterator : public std::iterator<std::forward_iterator_tag, const ErrorBase> {
        private:
            const ErrorBase* e;

            typedef std::iterator<std::forward_iterator_tag, const ErrorBase> base_type;

        public:
            explicit error_stack_const_iterator(const ErrorBase* p)
                : e(p)
            {
            }
            reference operator*() const
            {
                return *e;
            }

            pointer operator->() const
            {
                return e;
            }

            error_stack_const_iterator& operator++()
            {
                e = e->next;
                return *this;
            }

            bool operator==(error_stack_const_iterator that) const
            {
                return e == that.e;
            }

            bool operator!=(error_stack_const_iterator that) const
            {
                return e != that.e;
            }
        };
    }

    class ErrorStack {
    private:
        ErrorBase* head;
        std::size_t m_size;

        ErrorStack(const ErrorStack&);
        ErrorStack& operator=(const ErrorStack&);

    public:
        typedef internal::error_stack_const_iterator const_iterator;

        explicit ErrorStack()
            : head(0)
            , m_size(0)
        {
        }

        const_iterator begin() const
        {
            return const_iterator(head);
        }

        const_iterator end() const
        {
            return const_iterator(0);
        }

        // This will take the ownership of e
        // Requires it to be dynamically allocated
        void push(ErrorBase* e)
        {
            if (e) {
                e->next = head;
                head = e;
                ++m_size;
            }
        }

        // The caller will take the responsibility of deleting the returned pointer
        // Returns NULL when empty
        ErrorBase* pop()
        {
            if (head) {
                ErrorBase* result = head;
                head = head->next;
                --m_size;
                return result;
            }
            return 0;
        }

        bool empty() const
        {
            return head == 0;
        }

        std::size_t size() const AUTOJSONCXX_NOEXCEPT
        {
            return m_size;
        }

        ~ErrorStack()
        {
            while (head) {
                ErrorBase* next = head->next;
                delete head;
                head = next;
            }
        }

        void swap(ErrorStack& other) AUTOJSONCXX_NOEXCEPT
        {
            std::swap(head, other.head);
            std::swap(m_size, other.m_size);
        }

#if AUTOJSONCXX_HAS_RAVLUE

        ErrorStack(ErrorStack&& other)
            : head(other.head)
            , m_size(other.m_size)
        {
            other.head = 0;
            other.m_size = 0;
        }

        ErrorStack& operator==(ErrorStack&& other) AUTOJSONCXX_NOEXCEPT
        {
            swap(other);
        }

#endif
    };

    template <class OutputStream>
    OutputStream& operator<<(OutputStream& out, const ErrorStack& errs)
    {
        typedef ErrorStack::const_iterator iterator;
        for (iterator it = errs.begin(); it != errs.end(); ++it) {
            out << "(*) " << it->description() << '\n';
        }
        return out;
    }

    class ParsingResult;

    template <class OutputStream>
    OutputStream& operator<<(OutputStream& out, const ParsingResult& result);

    class ParsingResult {
    private:
        rapidjson::ParseResult result;
        ErrorStack stack;

    public:
        explicit ParsingResult()
            : result()
            , stack()
        {
        }

        rapidjson::ParseResult& json_parse_result()
        {
            return result;
        }

        const rapidjson::ParseResult& json_parse_result() const
        {
            return result;
        }

        rapidjson::ParseErrorCode error_code() const
        {
            return result.Code();
        }

        std::size_t offset() const
        {
            return result.Offset();
        }

        ErrorStack& error_stack()
        {
            return stack;
        }

        const ErrorStack& error_stack() const
        {
            return stack;
        }

        typedef ErrorStack::const_iterator const_iterator;

        const_iterator begin() const
        {
            return stack.begin();
        }

        const_iterator end() const
        {
            return stack.end();
        }

        bool has_error() const AUTOJSONCXX_NOEXCEPT
        {
            return result.IsError() || !stack.empty();
        }

        std::string description() const
        {
            std::ostringstream ss;
            ss << *this;
            return ss.str();
        }

        void swap(ParsingResult& other) AUTOJSONCXX_NOEXCEPT
        {
            std::swap(result, other.result);
            stack.swap(other.stack);
        }

        bool operator!() const AUTOJSONCXX_NOEXCEPT
        {
            return has_error();
        }

#if AUTOJSONCXX_HAS_EXPLICIT_OPERATOR

        explicit operator bool() const AUTOJSONCXX_NOEXCEPT
        {
            return !has_error();
        }

#endif

#if AUTOJSONCXX_HAS_RVALUE

        ParsingResult(ParsingResult&& other)
            : result()
            , stack()
        {
            swap(other);
        }

        ParsingResult& operator==(ParsingResult&& other) AUTOJSONCXX_NOEXCEPT
        {
            swap(other);
            return *this;
        }
#endif
    };

    template <class OutputStream>
    OutputStream& operator<<(OutputStream& out, const ParsingResult& result)
    {
        if (result.has_error()) {
            out << "Parsing failed at offset " << result.json_parse_result().Offset()
                << " with error code " << result.json_parse_result().Code() << ":\n"
                << rapidjson::GetParseError_En(result.json_parse_result().Code())
                << '\n';

            if (result.json_parse_result().Code() == rapidjson::kParseErrorTermination
                || !result.error_stack().empty()) {
                out << "\nTrace back (last call first):\n" << result.error_stack();
            }
        }
        return out;
    }
}

using error::ParsingResult;
}

#endif
