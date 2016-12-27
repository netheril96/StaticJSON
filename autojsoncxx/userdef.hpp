#pragma once

namespace config
{
struct Date
{
    int year;
    int month;
    int day;

    explicit Date() : year(), month(), day() {}

    void staticjson_init(staticjson::ObjectHandler* h)
    {
        h->add_property("year", &this->year, staticjson::Flags::Default);
        h->add_property("month", &this->month, staticjson::Flags::Default);
        h->add_property("day", &this->day, staticjson::Flags::Default);

        h->set_flags(staticjson::Flags::Default | staticjson::Flags::AllowDuplicateKey
                     | staticjson::Flags::DisallowUnknownKey);
    }
};
}

namespace config
{
namespace event
{
    struct BlockEvent
    {
        unsigned long long serial_number;
        unsigned long long admin_ID;
        Date date;
        std::string description;
        std::string details;

        explicit BlockEvent()
            : serial_number()
            , admin_ID(255)
            , date()
            , description("\x2f\x2a\x20\x69\x6e\x69\x74\x20\x2a\x2f\x20\x74\x72\x79\x69\x6e\x67\x20"
                          "\x74\x6f\x20\x6d\x65\x73\x73\x20\x75\x70\x20\x77\x69\x74\x68\x20\x74\x68"
                          "\x65\x20\x63\x6f\x64\x65\x20\x67\x65\x6e\x65\x72\x61\x74\x6f\x72")
            , details()
        {
            date.year = 1970;
            date.month = 1;
            date.day = 1; /* Assign date to the UNIX epoch */
        }

        void staticjson_init(staticjson::ObjectHandler* h)
        {
            h->add_property("\x73\x65\x72\x69\x61\x6c\x5f\x6e\x75\x6d\x62\x65\x72",
                            &this->serial_number,
                            staticjson::Flags::Default);
            h->add_property("administrator ID", &this->admin_ID, staticjson::Flags::Optional);
            h->add_property("date", &this->date, staticjson::Flags::Optional);
            h->add_property("description", &this->description, staticjson::Flags::Optional);
            h->add_property("details", &this->details, staticjson::Flags::Optional);

            h->set_flags(staticjson::Flags::Default | staticjson::Flags::AllowDuplicateKey);
        }
    };
}
}

namespace config
{
struct User
{
    unsigned long long ID;
    std::string nickname;
    Date birthday;
    std::shared_ptr<config::event::BlockEvent> block_event;
    std::vector<config::event::BlockEvent> dark_history;
    std::map<std::string, std::string> optional_attributes;

    explicit User()
        : ID()
        , nickname("\xe2\x9d\xb6\xe2\x9d\xb7\xe2\x9d\xb8")
        , birthday()
        , block_event()
        , dark_history()
        , optional_attributes()
    {
    }

    void staticjson_init(staticjson::ObjectHandler* h)
    {
        h->add_property("ID", &this->ID, staticjson::Flags::Default);
        h->add_property("nickname", &this->nickname, staticjson::Flags::Default);
        h->add_property("birthday", &this->birthday, staticjson::Flags::Optional);
        h->add_property("\x62\x6c\x6f\x63\x6b\x5f\x65\x76\x65\x6e\x74",
                        &this->block_event,
                        staticjson::Flags::Optional);
        h->add_property("\x64\x61\x72\x6b\x5f\x68\x69\x73\x74\x6f\x72\x79",
                        &this->dark_history,
                        staticjson::Flags::Optional);
        h->add_property(
            "\x6f\x70\x74\x69\x6f\x6e\x61\x6c\x5f\x61\x74\x74\x72\x69\x62\x75\x74\x65\x73",
            &this->optional_attributes,
            staticjson::Flags::Optional);

        h->set_flags(staticjson::Flags::Default);
    }
};
}
