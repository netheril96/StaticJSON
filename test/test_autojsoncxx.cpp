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

#include "catch.hpp"

#define AUTOJSONCXX_ROOT_DIRECTORY get_base_dir() + "/autojsoncxx/" +

#include <autojsoncxx.hpp>

#include "userdef.hpp"

#include <fstream>
#include <stack>

using namespace autojsoncxx;
using namespace config;
using namespace config::event;

const std::string& get_base_dir(void);

namespace config
{
inline bool operator==(Date d1, Date d2)
{
    return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

inline bool operator!=(Date d1, Date d2) { return !(d1 == d2); }
}

namespace config
{
namespace event
{
    bool operator==(const BlockEvent& b1, const BlockEvent& b2)
    {
        return b1.admin_ID == b2.admin_ID && b1.date == b2.date && b1.description == b2.description
            && b1.details == b2.details && b1.serial_number == b2.serial_number;
    }
}

bool operator==(const User& u1, const User& u2)
{
    return u1.birthday == u2.birthday
        && (u1.block_event == u2.block_event
            || (u1.block_event && u2.block_event && *u1.block_event == *u2.block_event))
        && u1.dark_history == u2.dark_history && u1.ID == u2.ID && u1.nickname == u2.nickname
        && u1.optional_attributes == u2.optional_attributes;
}
}

static std::string read_all(const std::string& file_name)
{
    std::ifstream input(file_name.c_str());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

static Date create_date(int year, int month, int day)
{
    Date d;
    d.year = year;
    d.month = month;
    d.day = day;
    return d;
}

// If most of the cases fail, you probably set the work directory wrong.
// Point the work directory to the `test/` subdirectory
// or redefine the macro AUTOJSONCXX_ROOT_DIRECTORY.

TEST_CASE("Test for the constructor of generated class (old)", "[code generator]")
{
    User user;

    REQUIRE(user.ID == 0ULL);

    // Do not use string literal because MSVC will mess up the encoding
    static const char default_nickname[] = {char(0xe2),
                                            char(0x9d),
                                            char(0xb6),
                                            char(0xe2),
                                            char(0x9d),
                                            char(0xb7),
                                            char(0xe2),
                                            char(0x9d),
                                            char(0xb8)};

    REQUIRE(user.nickname.size() == sizeof(default_nickname));
    REQUIRE(std::equal(user.nickname.begin(), user.nickname.end(), default_nickname));

    REQUIRE(user.birthday == create_date(0, 0, 0));
    REQUIRE(user.dark_history.empty());
    REQUIRE(user.optional_attributes.empty());
    REQUIRE(!user.block_event);

    BlockEvent event;

    REQUIRE(event.admin_ID == 255ULL);
    REQUIRE(event.date == create_date(1970, 1, 1));
    REQUIRE(event.serial_number == 0ULL);
    REQUIRE(event.details.empty());
}

TEST_CASE("Test for correct parsing (old)", "[parsing]")
{
    SECTION("Test for an array of user", "[parsing]")
    {
        std::vector<User> users;
        ParsingResult err;

        bool success = from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", users, err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
        REQUIRE(users.size() == 2);

        {
            const User& u = users.front();
            REQUIRE(u.ID == 7947402710862746952ULL);
            REQUIRE(u.nickname == "bigger than bigger");
            REQUIRE(u.birthday == create_date(1984, 9, 2));

            REQUIRE(u.block_event.get() != nullptr);
            const BlockEvent& e = *u.block_event;

            REQUIRE(e.admin_ID > 0ULL);
            REQUIRE(e.date == create_date(1970, 12, 31));
            REQUIRE(e.description == "advertisement");
            REQUIRE(e.details.size() > 0ULL);

            REQUIRE(u.dark_history.empty());
            REQUIRE(u.optional_attributes.empty());
        }

        {
            const User& u = users.back();
            REQUIRE(u.ID == 13478355757133566847ULL);
            REQUIRE(u.nickname.size() == 15);
            REQUIRE(!u.block_event);
            REQUIRE(u.optional_attributes.size() == 3);
            REQUIRE(u.optional_attributes.find("Self description") != u.optional_attributes.end());
        }
    }

    SECTION("Test for a map of user", "[parsing]")
    {
        std::unordered_map<std::string, User> users;
        ParsingResult err;

        bool success = from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_map.json", users, err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
        REQUIRE(users.size() == 2);

        {
            const User& u = users["First"];
            REQUIRE(u.ID == 7947402710862746952ULL);
            REQUIRE(u.nickname == "bigger than bigger");
            REQUIRE(u.birthday == create_date(1984, 9, 2));

            REQUIRE(u.block_event.get() != nullptr);
            const BlockEvent& e = *u.block_event;

            REQUIRE(e.admin_ID > 0ULL);
            REQUIRE(e.date == create_date(1970, 12, 31));
            REQUIRE(e.description == "advertisement");
            REQUIRE(e.details.size() > 0ULL);

            REQUIRE(u.dark_history.empty());
            REQUIRE(u.optional_attributes.empty());
        }

        {
            const User& u = users["Second"];
            REQUIRE(u.ID == 13478355757133566847ULL);
            REQUIRE(u.nickname.size() == 15);
            REQUIRE(!u.block_event);
            REQUIRE(u.optional_attributes.size() == 3);
            REQUIRE(u.optional_attributes.find("Self description") != u.optional_attributes.end());
        }
    }
}

TEST_CASE("Test for mismatch between JSON and C++ class std::vector<config::User> (old)",
          "[parsing], [error]")
{
    std::vector<User> users;
    ParsingResult err;

    SECTION("Mismatch between array and object", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/single_object.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
        REQUIRE(std::distance(err.begin(), err.end()) == 1);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());

        // REQUIRE(e.expected_type() == "array");

        REQUIRE(e.actual_type() == "object");
    }

    SECTION("Required field not present; test the path as well",
            "[parsing], [error], [missing required]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/missing_required.json", users, err));
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
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/unknown_field.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);

        REQUIRE(static_cast<const error::UnknownFieldError&>(*err.begin()).field_name() == "hour");
    }

    SECTION("Duplicate key in class User", "[parsing], [error], [duplicate key]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/duplicate_key_user.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::DUPLICATE_KEYS);

        REQUIRE(static_cast<const error::DuplicateKeyError&>(*err.begin()).key() == "ID");
    }

    SECTION("Out of range", "[parsing], [error], [out of range]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/out_of_range.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::NUMBER_OUT_OF_RANGE);
    }

    SECTION("Mismatch between integer and string", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/integer_string.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
    }

    SECTION("Null character in key", "[parsing], [error], [null character]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/null_in_key.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);
    }
}

TEST_CASE("Test for mismatch between JSON and C++ class std::map<std::string, config::User> (old)",
          "[parsing], [error]")
{
    std::map<std::string, config::User> users;
    ParsingResult err;

    SECTION("Mismatch between object and array", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());
        // REQUIRE(e.expected_type() == "object");
        REQUIRE(e.actual_type() == "array");
    }

    SECTION("Mismatch in mapped element", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/map_element_mismatch.json", users, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());
        {
            REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);

            auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());
        }
        {
            auto it = ++err.begin();
            REQUIRE(it != err.end());
            REQUIRE(it->type() == error::OBJECT_MEMBER);
            auto&& e = static_cast<const error::ObjectMemberError&>(*it);
            REQUIRE(e.member_name() == "Third");
        }
    }
}

TEST_CASE("Test for writing JSON (old)", "[serialization]")
{
    std::vector<User> users;
    ParsingResult err;

    bool success = from_json_file(
        AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", users, err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users.size() == 2);

    std::string str = to_json_string(users);
    std::vector<User> copied_users;

    success = from_json_string(str, copied_users, err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users == copied_users);
}

TEST_CASE("Test for DOM support (old)", "[DOM]")
{
    rapidjson::Document doc;
    ParsingResult err;
    bool success = from_json_file(
        AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array_compact.json", doc, err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }

    SECTION("Test for parsed result", "[DOM], [parsing]")
    {
        REQUIRE(doc.IsArray());
        REQUIRE(doc.Size() == 2);

        const rapidjson::Value& second = doc[1u];
        REQUIRE(second["ID"].IsUint64());
        REQUIRE(second["ID"].GetUint64() == 13478355757133566847ULL);
        REQUIRE(second["block_event"].IsNull());
        REQUIRE(second["dark_history"].IsArray());
        REQUIRE(second["dark_history"][0u].IsObject());
        REQUIRE(second["dark_history"][0u]["description"] == "copyright infringement");
    }

    SECTION("Test for serialization", "[DOM], [serialization]")
    {
        std::string output;
        to_json_string(output, doc);
        REQUIRE(
            output
            == read_all(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array_compact.json"));
    }

    SECTION("Test for to/from DOM", "[DOM], [conversion]")
    {
        std::vector<User> users;
        error::ErrorStack errs;

        REQUIRE(from_document(users, doc, errs));

        REQUIRE(users.size() == 2);
        REQUIRE(users[0].birthday == create_date(1984, 9, 2));
        REQUIRE(users[0].block_event);
        REQUIRE(users[0].block_event->details == "most likely a troll");

        rapidjson::Document another_doc;
        to_document(users, another_doc);
        REQUIRE(doc == another_doc);
    }
}

TEST_CASE("Test for parsing tuple type (old)", "[parsing], [tuple]")
{
    typedef std::tuple<BlockEvent,
                       int,
                       std::nullptr_t,
                       double,
                       std::unordered_map<std::string, std::shared_ptr<User>>,
                       bool>
        hard_type;
    hard_type hetero_array;
    ParsingResult err;

    SECTION("Test for valid tuple", "[parsing], [tuple]")
    {
        bool success = from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/hard.json", hetero_array, err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
        REQUIRE(std::get<1>(hetero_array) == -65535);
        REQUIRE(std::get<std::tuple_size<hard_type>::value - 1>(hetero_array) == false);
    }

    SECTION("Test for invalid tuple", "[parsing], [tuple], [error]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/hard.json", hetero_array, err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());
        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
        REQUIRE(static_cast<const error::TypeMismatchError&>(*err.begin()).actual_type() == "null");
    }
}
