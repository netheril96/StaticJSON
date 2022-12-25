#include "catch.hpp"
#include "myarray.hpp"

#include <staticjson/document.hpp>
#include <staticjson/staticjson.hpp>

#include <rapidjson/schema.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>

#ifdef __has_include
#if __has_include(<optional>)
#pragma message("INFO: <optional> available for testing")
#include <staticjson/optional_support.hpp>
#endif
#endif

#ifdef _MSC_VER
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

enum class CalendarType
{
    Gregorian,
    Chinese,
    Jewish,
    Islam
};

STATICJSON_DECLARE_ENUM(CalendarType,
                        {"Gregorian", CalendarType::Gregorian},
                        {"Chinese", CalendarType::Chinese},
                        {"Jewish", CalendarType::Jewish},
                        {"Islam", CalendarType::Islam})

struct Date
{
    int year, month, day;
    CalendarType type;

    Date() : year(0), month(0), day(0), type(CalendarType::Gregorian) {}
};

namespace staticjson
{
void init(Date* d, ObjectHandler* h)
{
    h->add_property("year", &d->year);
    h->add_property("month", &d->month);
    h->add_property("day", &d->day);
    h->add_property("type", &d->type, Flags::Optional);
    h->set_flags(Flags::DisallowUnknownKey);
}
}

struct BlockEvent
{
    std::uint64_t serial_number, admin_ID = 255;
    Date date;
    std::string description, details;
#ifdef STATICJSON_OPTIONAL
    staticjson::optional<std::string> flags;
#endif

    void staticjson_init(ObjectHandler* h)
    {
        h->add_property("serial_number", &serial_number);
        h->add_property("administrator ID", &admin_ID, Flags::Optional);
        h->add_property("date", &date, Flags::Optional);
        h->add_property("description", &description, Flags::Optional);
        h->add_property("details", &details, Flags::Optional);
#ifdef STATICJSON_OPTIONAL
        h->add_property("flags", &flags, Flags::Optional);
#endif
    }
};

struct User
{
    friend class Handler<User>;

    unsigned long long ID;
    std::string nickname;
    Date birthday;
    std::shared_ptr<BlockEvent> block_event;
    std::vector<BlockEvent> dark_history;
    std::unordered_map<std::string, std::string> optional_attributes;

    std::tuple<int, std::vector<std::tuple<double, double>>, bool> auxiliary;

#ifdef STATICJSON_OPTIONAL
    staticjson::optional<BlockEvent> dark_event;
    staticjson::optional<std::vector<staticjson::optional<BlockEvent>>> alternate_history;
#endif
};

namespace staticjson
{
template <>
class Handler<User> : public ObjectHandler
{
public:
    explicit Handler(User* user)
    {
        auto h = this;
        h->add_property("ID", &user->ID);
        h->add_property("nickname", &user->nickname);
        h->add_property("birthday", &user->birthday, Flags::Optional);
        h->add_property("block_event", &user->block_event, Flags::Optional);
        h->add_property("optional_attributes", &user->optional_attributes, Flags::Optional);
        h->add_property("dark_history", &user->dark_history, Flags::Optional);
        h->add_property("auxiliary", &user->auxiliary, Flags::Optional);
#ifdef STATICJSON_OPTIONAL
        h->add_property("dark_event", &user->dark_event, Flags::Optional);
        h->add_property("alternate_history", &user->alternate_history, Flags::Optional);
#endif
    }

    std::string type_name() const override { return "User"; }
};
}

inline bool operator==(Date d1, Date d2)
{
    return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

inline bool operator!=(Date d1, Date d2) { return !(d1 == d2); }

inline bool operator==(const BlockEvent& b1, const BlockEvent& b2)
{
    return b1.admin_ID == b2.admin_ID && b1.date == b2.date && b1.description == b2.description
        && b1.details == b2.details;
}

inline bool operator!=(const BlockEvent& b1, const BlockEvent& b2) { return !(b1 == b2); }

inline bool operator==(const User& u1, const User& u2)
{
    return u1.birthday == u2.birthday && u1.ID == u2.ID && u1.nickname == u2.nickname
        && u1.dark_history == u2.dark_history && u1.optional_attributes == u2.optional_attributes
        && ((!u1.block_event && !u2.block_event)
            || ((u1.block_event && u2.block_event) && *u1.block_event == *u2.block_event));
}

inline bool operator!=(const User& u1, const User& u2) { return !(u1 == u2); }

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

void check_first_user(const User& u)
{
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

#ifdef STATICJSON_OPTIONAL
    REQUIRE(!!u.dark_event);
    REQUIRE(u.dark_event->serial_number == 9876543210123456789ULL);
    REQUIRE(u.dark_event->admin_ID == 11223344556677889900ULL);
    REQUIRE(u.dark_event->date == create_date(1970, 12, 31));
    REQUIRE(u.dark_event->description == "promote");
    REQUIRE(u.dark_event->details == "at least like a troll");
    REQUIRE(static_cast<bool>(u.dark_event->flags) == false);

    REQUIRE(static_cast<bool>(u.alternate_history) == true);
    REQUIRE(u.alternate_history->size() == 2);
    REQUIRE(static_cast<bool>(u.alternate_history->at(0)) == false);
    REQUIRE(static_cast<bool>(u.alternate_history->at(1)) == true);

    {
        const auto& opt_e = u.alternate_history->at(1);
        REQUIRE(opt_e);
        const BlockEvent& e = *opt_e;
        REQUIRE(e.serial_number == 1123581321345589ULL);
        REQUIRE(e.admin_ID == 1123581321345589ULL);
        REQUIRE(e.date == create_date(1970, 12, 31));
        REQUIRE(e.description == "redacted");
        REQUIRE(e.details == "redacted");
        REQUIRE(static_cast<bool>(e.flags) == true);
        REQUIRE(*e.flags == "x");
    }
#endif
}

void check_second_user(const User& u)
{
    REQUIRE(u.ID == 13478355757133566847ULL);
    REQUIRE(u.nickname.size() == 15);
    REQUIRE(!u.block_event);
    REQUIRE(u.optional_attributes.size() == 3);
    REQUIRE(u.optional_attributes.find("Self description") != u.optional_attributes.end());
}

template <class ArrayOfUsers>
void check_array_of_user(const ArrayOfUsers& users)
{
    REQUIRE(users.size() == 3);

    check_first_user(users[0]);
    check_second_user(users[1]);
}

void check_array_of_user(const Document& users)
{
    REQUIRE(users.IsArray());
    REQUIRE(users.Size() == 3);

    const Value& u = users[0];
    REQUIRE(u.IsObject());
    REQUIRE(u.HasMember("ID"));
    REQUIRE(u["ID"].IsUint64());
    REQUIRE(u["ID"].GetUint64() == 7947402710862746952ULL);
    REQUIRE(u.HasMember("nickname"));
    REQUIRE(u["nickname"].IsString());
    REQUIRE(std::strcmp(u["nickname"].GetString(), "bigger than bigger") == 0);
    REQUIRE(u.HasMember("birthday"));
    REQUIRE(u["birthday"].IsObject());
    REQUIRE(u["birthday"].HasMember("year"));
    REQUIRE(u["birthday"]["year"] == 1984);

    REQUIRE(u.HasMember("block_event"));
    const Value& e = u["block_event"];
    REQUIRE(e.HasMember("administrator ID"));
    REQUIRE(e.HasMember("description"));
    const Value& desc = e["description"];
    REQUIRE(desc.IsString());
    REQUIRE(std::strcmp(desc.GetString(), "advertisement") == 0);

#ifdef STATICJSON_OPTIONAL
    REQUIRE(u.HasMember("dark_event"));
    {
        const Value& e = u["dark_event"];
        REQUIRE(e.HasMember("administrator ID"));
        REQUIRE(e.HasMember("description"));
        REQUIRE(e["flags"].IsNull());
    }

    REQUIRE(u.HasMember("alternate_history"));
    {
        const Value& e = u["alternate_history"];
        REQUIRE(e.IsArray());
        REQUIRE(e[0].IsNull());
        REQUIRE(e[1].IsObject());
        REQUIRE(e[1]["flags"].IsString());
    }
#endif
}

TEST_CASE("Test for correct parsing", "[parsing],[c]")
{
    SECTION("Simple date parsing", "[parsing]")
    {
        Date d;
        ParseStatus err;
        bool success = from_json_string(
            "{\"year\": 1900, \"day\": 3, \"month\": 11, \"type\": \"Chinese\"}", &d, &err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
    }

    SECTION("Test for an array of user", "[parsing]")
    {
        Array<User> users;
        ParseStatus err;

        // Do it twice to test if vectors are reset properly at parsing.
        for (int i = 0; i < 2; ++i)
        {
            bool success = from_json_file(
                get_base_dir() + "/examples/success/user_array.json", &users, &err);
            {
                CAPTURE(err.description());
                REQUIRE(success);
            }
            check_array_of_user(users);
            REQUIRE(users[0].birthday.type == CalendarType::Jewish);
        }

        Document d;
        REQUIRE(to_json_document(&d, users, nullptr));
        check_array_of_user(d);
    }
    SECTION("Test for document", "[parsing]")
    {
        Document users;
        ParseStatus err;

        bool success
            = from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err);
        {
            CAPTURE(err.description());
            REQUIRE(success);
        }
        check_array_of_user(users);

        std::vector<User> vusers;
        REQUIRE(from_json_document(users, &vusers, nullptr));
        check_array_of_user(vusers);
    }

    SECTION("Test for a map of user", "[parsing], [q]")
    {
        std::unordered_map<std::string, User> users;
        ParseStatus err;

        // Do it twice to test if map is reset properly at parsing.
        for (int i = 0; i < 2; ++i)
        {
            bool success
                = from_json_file(get_base_dir() + "/examples/success/user_map.json", &users, &err);
            {
                CAPTURE(err.description());
                REQUIRE(success);
            }
            REQUIRE(users.size() == 2);
            check_first_user(users["First"]);
            check_second_user(users["Second"]);
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

    SECTION("Invalid enum", "[parsing], [error], [enum]")
    {
        REQUIRE(
            !from_json_file(get_base_dir() + "/examples/failure/invalid_enum.json", &users, &err));
        CAPTURE(err.description());
        REQUIRE(!err.error_stack().empty());

        REQUIRE(err.begin()->type() == error::INVALID_ENUM);
        const auto& e = static_cast<const error::InvalidEnumError&>(*err.begin());
        REQUIRE(e.description() == "\"West\" is an invalid enum name");
    }
}

TEST_CASE("Test for max leaf number check", "[serialization]")
{
    std::vector<User> users, reparsed_users;
    ParseStatus err;

    bool success
        = from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users.size() == 3);

    std::string json_output = to_pretty_json_string(users);
    CAPTURE(json_output);
    staticjson::GlobalConfig::getInstance()->setMaxLeaves(20);
    success = from_json_string(json_output.c_str(), &reparsed_users, &err);
    staticjson::GlobalConfig::getInstance()->unsetMaxLeavesFlag();
    {
        CAPTURE(err.description());
        REQUIRE(!success);
    }

    REQUIRE(err.description().find("Too many leaves") != std::string::npos);
}

TEST_CASE("Test for max depth check", "[serialization]")
{
    std::vector<User> users, reparsed_users;
    ParseStatus err;
    staticjson::GlobalConfig::getInstance()->setMaxDepth(3);
    bool success
        = from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err);
    staticjson::GlobalConfig::getInstance()->unsetMaxDepthFlag();

    {
        CAPTURE(err.description());
        REQUIRE(!success);
    }
    REQUIRE(err.description().find("Too many levels of recursion") != std::string::npos);
}

TEST_CASE("Test for writing JSON", "[serialization]")
{
    std::vector<User> users, reparsed_users;
    ParseStatus err;

    bool success
        = from_json_file(get_base_dir() + "/examples/success/user_array.json", &users, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }
    REQUIRE(users.size() == 3);

    std::string json_output = to_pretty_json_string(users);
    CAPTURE(json_output);

    success = from_json_string(json_output.c_str(), &reparsed_users, &err);
    {
        CAPTURE(err.description());
        REQUIRE(success);
    }

    REQUIRE(users == reparsed_users);
}

static bool is_valid_json(const std::string& filename, rapidjson::SchemaValidator* validator)
{
    Document d;
    REQUIRE(from_json_file(filename, &d, nullptr));
    bool rc = d.Accept(*validator);
    validator->Reset();
    return rc;
}

TEST_CASE("Schema generation and validation")
{
    const char* invalid_json_filenames[] = {"out_of_range.json",
                                            "integer_string.json",
                                            "missing_required.json",
                                            "map_element_mismatch.json",
                                            "null_in_key.json",
                                            "single_object.json",
                                            "unknown_field.json",
                                            "invalid_enum.json"};
    {
        std::vector<User> users;
        Document schema = export_json_schema(&users);
        rapidjson::SchemaDocument sd(schema);
        rapidjson::SchemaValidator validator(sd);

        REQUIRE(is_valid_json(get_base_dir() + "/examples/success/user_array.json", &validator));
        REQUIRE(is_valid_json(get_base_dir() + "/examples/success/user_array_compact.json",
                              &validator));

        for (const char* fn : invalid_json_filenames)
            REQUIRE(!is_valid_json(get_base_dir() + "/examples/failure/" + fn, &validator));
    }
    {
        std::map<std::string, User> users;
        Document schema = export_json_schema(&users);
        rapidjson::SchemaDocument sd(schema);
        rapidjson::SchemaValidator validator(sd);

        REQUIRE(is_valid_json(get_base_dir() + "/examples/success/user_map.json", &validator));
        for (const char* fn : invalid_json_filenames)
            REQUIRE(!is_valid_json(get_base_dir() + "/examples/failure/" + fn, &validator));
    }
}
