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

#include <catch.hpp>

#define AUTOJSONCXX_HAS_MODERN_TYPES 1
#define AUTOJSONCXX_HAS_RVALUE 1

#include "userdef.hpp"

using namespace autojsoncxx;
using namespace config;
using namespace config::event;

inline bool operator==(Date d1, Date d2)
{
    return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

inline bool operator!=(Date d1, Date d2)
{
    return !(d1 == d2);
}

TEST_CASE("Test for correct parsing", "[parsing]")
{
    std::vector<User> users;
    ParsingResult err;

    bool success = from_json_file("./examples/user.json", users, err);
    {

        REQUIRE(success);
    }
    REQUIRE(users.size() == 2);

    {
        const User& u = users.front();
        REQUIRE(u.ID == 7947402710862746952ULL);
        REQUIRE(u.nickname == "bigger than bigger");
        REQUIRE(u.birthday == Date(1984, 9, 2));

        REQUIRE(u.block_event.get() != 0);
        const BlockEvent& e = *u.block_event;

        REQUIRE(e.admin_ID > 0);
        REQUIRE(e.date == Date(1970, 12, 31));
        REQUIRE(e.description == "advertisement");
        REQUIRE(e.details.size() > 0);

        REQUIRE(u.dark_history.empty());
        REQUIRE(u.optional_attributes.empty());
    }

    {
        const User& u = users.back();
        REQUIRE(u.ID == 13478355757133566847ULL);
        REQUIRE(u.nickname.size() == 15);
        REQUIRE(u.block_event.get() == 0);
        REQUIRE(u.optional_attributes.size() == 3);
        REQUIRE(u.optional_attributes.find("Self description") != u.optional_attributes.end());
    }
}

TEST_CASE("Test for mismatch between JSON and C++ class", "[parsing], [error]")
{
    std::vector<User> users;
    ParsingResult err;

    SECTION("Mismatch between array and object", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file("./examples/error/single_object.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
        REQUIRE(std::distance(err.begin(), err.end()) == 1);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());

        REQUIRE(std::strcmp(e.expected_type(), "array") == 0);

        REQUIRE(std::strcmp(e.actual_type(), "object") == 0);
    }

    SECTION("Required field not present; test the path as well", "[parsing], [error], [missing required]")
    {
        REQUIRE(!from_json_file("./examples/error/missing_required.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(std::distance(err.begin(), err.end()) == 5);

        auto it = err.begin();
        REQUIRE(it->type() == error::MISSING_REQUIRED);

        ++it;
        REQUIRE(it->type() == error::OBJECT_MEMBER);
        REQUIRE(static_cast<const error::ObjectMemberError&>(*it).member_name() == "date");

        ++it;
        REQUIRE(it->type() == error::ARRAY_ELEMENT);
        REQUIRE(static_cast<const error::ArrayElementError&>(*it).index() == 0);

        ++it;
        REQUIRE(it->type() == error::OBJECT_MEMBER);
        REQUIRE(static_cast<const error::ObjectMemberError&>(*it).member_name() == "dark_history");
    }

    SECTION("Unknown field in strict parsed class Date", "[parsing], [error], [unknown field]")
    {
        REQUIRE(!from_json_file("./examples/error/unknown_field.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);

        REQUIRE(static_cast<const error::UnknownFieldError&>(*err.begin()).field_name() == "hour");
    }

    SECTION("Duplicate key", "[parsing], [error], [duplicate key]")
    {
        REQUIRE(!from_json_file("./examples/error/duplicate_key.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::DUPLICATE_KEYS);

        REQUIRE(static_cast<const error::DuplicateKeyError&>(*err.begin()).key() == "Auth-Token");
    }

    SECTION("Out of range", "[parsing], [error], [out of range]")
    {
        REQUIRE(!from_json_file("./examples/error/out_of_range.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::NUMBER_OUT_OF_RANGE);
    }

    SECTION("Mismatch between integer and string", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file("./examples/error/integer_string.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
    }

    SECTION("Null character in key", "[parsing], [error], [null character]")
    {
        REQUIRE(!from_json_file("./examples/error/null_in_key.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);
    }
}
