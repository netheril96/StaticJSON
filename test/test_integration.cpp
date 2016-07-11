#include "catch.hpp"

#include <staticjson/staticjson.hpp>

// Uncomment the next line if you are adventurous
// #define AUTOJSONCXX_HAS_VARIADIC_TEMPLATE 1

#ifndef AUTOJSONCXX_ROOT_DIRECTORY
#define AUTOJSONCXX_ROOT_DIRECTORY ".."
#endif

#include <fstream>
#include <sstream>

using namespace staticjson;

struct Date
{
    int year, month, day;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("year", &year);
        h->add_property("month", &month);
    }
};

struct BlockEvent
{
    std::uint64_t serial_number, admin_ID;
    Date date;
    std::string description, details;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("serial_number", &serial_number);
        h->add_property("administrator ID", &admin_ID);
        h->add_property("data", &date);
        h->add_property("description", &description);
        h->add_property("details", &details);
    }
};

struct User
{
    unsigned long long ID;
    std::string nickname;
    Date birthday;
    std::shared_ptr<BlockEvent> block_event;
    std::vector<BlockEvent> dark_history;
    std::unordered_map<std::string, std::string> optional_attributes;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("ID", &ID);
        h->add_property("nickname", &nickname);
        h->add_property("birthday", &birthday);
        h->add_property("block_event", &block_event);
        h->add_property("optional_attributes", &optional_attributes);
    }
};

inline bool operator==(Date d1, Date d2)
{
    return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

inline bool operator!=(Date d1, Date d2) { return !(d1 == d2); }

inline std::string read_all(const char* file_name)
{
    std::ifstream input(file_name);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

inline Date create_date(int year, int month, int day)
{
    Date d;
    d.year = year;
    d.month = month;
    d.day = day;
    return d;
}

TEST_CASE("Test for correct parsing", "[parsing]")
{
    SECTION("Test for an array of user", "[parsing]")
    {
        std::vector<User> users;
        ParseStatus err;

        bool success = from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", &users, &err);
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

            REQUIRE(u.block_event.get() != 0);
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
        ParseStatus err;

        bool success = from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_map.json", &users, &err);
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

            REQUIRE(u.block_event.get() != 0);
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

TEST_CASE("Test for mismatch between JSON and C++ class std::vector<config::User>",
          "[parsing], [error]")
{
    std::vector<User> users;
    ParseStatus err;

    SECTION("Mismatch between array and object", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/single_object.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
        REQUIRE(std::distance(err.begin(), err.end()) == 1);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());

        REQUIRE(e.expected_type() == "array");

        REQUIRE(e.actual_type() == "object");
    }

    SECTION("Required field not present; test the path as well",
            "[parsing], [error], [missing required]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/missing_required.json", &users, &err));
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
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/unknown_field.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);

        REQUIRE(static_cast<const error::UnknownFieldError&>(*err.begin()).field_name() == "hour");
    }

    SECTION("Duplicate key", "[parsing], [error], [duplicate key]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/duplicate_key.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::DUPLICATE_KEYS);

        REQUIRE(static_cast<const error::DuplicateKeyError&>(*err.begin()).key() == "Auth-Token");
    }

    SECTION("Duplicate key in class User", "[parsing], [error], [duplicate key]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/duplicate_key_user.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::DUPLICATE_KEYS);

        REQUIRE(static_cast<const error::DuplicateKeyError&>(*err.begin()).key() == "ID");
    }

    SECTION("Out of range", "[parsing], [error], [out of range]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/out_of_range.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::NUMBER_OUT_OF_RANGE);
    }

    SECTION("Mismatch between integer and string", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/integer_string.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
    }

    SECTION("Null character in key", "[parsing], [error], [null character]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/failure/null_in_key.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);
    }
}

TEST_CASE("Test for mismatch between JSON and C++ class std::map<std::string, config::User>",
          "[parsing], [error]")
{
    std::map<std::string, User> users;
    ParseStatus err;

    SECTION("Mismatch between object and array", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());
        REQUIRE(e.expected_type() == "object");
        REQUIRE(e.actual_type() == "array");
    }

    SECTION("Mismatch in mapped element", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(AUTOJSONCXX_ROOT_DIRECTORY
                                "/examples/failure/map_element_mismatch.json",
                                &users,
                                &err));
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

TEST_CASE("Test for writing JSON", "[serialization]")
{
    std::vector<User> users;
    ParseStatus err;

    bool success = from_json_file(
        AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array.json", &users, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users.size() == 2);

    REQUIRE(to_json_string(users)
            == read_all(AUTOJSONCXX_ROOT_DIRECTORY "/examples/success/user_array_compact.json"));
}
