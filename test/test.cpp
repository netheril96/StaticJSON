#include <catch.hpp>
#define AUTOJSONCXX_MODERN_COMPILER 1
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

    REQUIRE(from_json_file("./examples/user.json", users, err));
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
