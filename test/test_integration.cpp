#include "catch.hpp"

#include <staticjson/staticjson.hpp>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>

#ifdef WIN32
#define stat _stat
#endif

const std::string& get_base_dir()
{
    struct Initializer
    {
        std::string path;

        Initializer()
        {
            path = ".";
            struct stat st;
            for (int i = 0; i < 16; ++i)
            {
                if (::stat((path + "/examples").c_str(), &st) == 0)
                {
                    return;
                }
                else
                {
                    path += "/..";
                }
            }
            fprintf(stderr,
                    "%s",
                    "No 'examples' directory found in the working directory or its ancestors\n");
            std::abort();
        }
    };

    static const Initializer init;
    return init.path;
}

using namespace staticjson;

struct Date
{
    int year, month, day;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("year", &year);
        h->add_property("month", &month);
        h->add_property("day", &day);
        h->set_flags(Flags::DisallowUnknownKey);
    }
};

struct BlockEvent
{
    std::uint64_t serial_number, admin_ID = 255;
    Date date;
    std::string description, details;

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("serial_number", &serial_number);
        h->add_property("administrator ID", &admin_ID, Flags::Optional);
        h->add_property("date", &date, Flags::Optional);
        h->add_property("description", &description, Flags::Optional);
        h->add_property("details", &details, Flags::Optional);
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
        h->add_property("birthday", &birthday, Flags::Optional);
        h->add_property("block_event", &block_event, Flags::Optional);
        h->add_property("optional_attributes", &optional_attributes, Flags::Optional);
        h->add_property("dark_history", &dark_history, Flags::Optional);
    }
};

inline bool operator==(Date d1, Date d2)
{
    return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

inline bool operator!=(Date d1, Date d2) { return !(d1 == d2); }

inline std::string read_all(const std::string& file_name)
{
    nonpublic::FileGuard fg(std::fopen(file_name.c_str(), "rb"));
    if (!fg.fp)
        throw std::system_error(errno, std::system_category());
    std::string result;
    while (1)
    {
        int ch = getc(fg.fp);
        if (ch == EOF)
            break;
        result.push_back(static_cast<unsigned char>(ch));
    }
    return result;
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

        bool success
            = from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err);
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

        bool success
            = from_json_file(get_base_dir() + "/examples/success/user_map.json", &users, &err);
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
        REQUIRE(
            !from_json_file(get_base_dir() + "/examples/failure/single_object.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
        REQUIRE(std::distance(err.begin(), err.end()) == 1);
    }

    SECTION("Missing requied simple", "[missing required]")
    {
        Date date;
        const char* str = "{ \"month\": 12 }";
        REQUIRE(!from_json_string(str, &date, nullptr));
    }

    SECTION("Required field not present; test the path as well",
            "[parsing], [error], [missing required]")
    {
        REQUIRE(!from_json_file(
            get_base_dir() + "/examples/failure/missing_required.json", &users, &err));
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
        REQUIRE(
            !from_json_file(get_base_dir() + "/examples/failure/unknown_field.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::UNKNOWN_FIELD);

        REQUIRE(static_cast<const error::UnknownFieldError&>(*err.begin()).field_name() == "hour");
    }

    SECTION("Duplicate key in class User", "[parsing], [error], [duplicate key]")
    {
        REQUIRE(!from_json_file(
            get_base_dir() + "/examples/failure/duplicate_key_user.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::DUPLICATE_KEYS);

        REQUIRE(static_cast<const error::DuplicateKeyError&>(*err.begin()).key() == "ID");
    }

    SECTION("Out of range", "[parsing], [error], [out of range]")
    {
        REQUIRE(
            !from_json_file(get_base_dir() + "/examples/failure/out_of_range.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::NUMBER_OUT_OF_RANGE);
    }

    SECTION("Mismatch between integer and string", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            get_base_dir() + "/examples/failure/integer_string.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);
    }

    SECTION("Null character in key", "[parsing], [error], [null character]")
    {
        REQUIRE(
            !from_json_file(get_base_dir() + "/examples/failure/null_in_key.json", &users, &err));
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
        REQUIRE(
            !from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::TYPE_MISMATCH);

        auto&& e = static_cast<const error::TypeMismatchError&>(*err.begin());
        REQUIRE(e.actual_type() == "array");
    }

    SECTION("Mismatch in mapped element", "[parsing], [error], [type mismatch]")
    {
        REQUIRE(!from_json_file(
            get_base_dir() + "/examples/failure/map_element_mismatch.json", &users, &err));
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

    bool success
        = from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users.size() == 2);

    REQUIRE(to_json_string(users)
            == read_all(get_base_dir() + "/examples/success/user_array_compact.json"));
}
