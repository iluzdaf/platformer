#include <catch2/catch_test_macros.hpp>
#include <string>
#include <type_traits>
#include <vector>
#include "events/event.hpp"

namespace
{
    class Bell
    {
    public:
        Event<Bell, const std::string &> onRung;
        Event<Bell> onSilent;

        void ring(const std::string &note)
        {
            onRung(note);
        }
    };
}

TEST_CASE("Anyone can hear an event, and only its owner can raise it", "[Event]")
{
    Bell bell;
    std::vector<std::string> heard;
    bell.onRung.connect([&](const std::string &note) { heard.push_back(note); });
    bell.onRung.connect([&](const std::string &note) { heard.push_back(note + "!"); });

    bell.ring("ding");

    REQUIRE(heard == std::vector<std::string>{"ding", "ding!"});
    STATIC_REQUIRE_FALSE(std::is_invocable_v<decltype(bell.onRung) &, const std::string &>);
    STATIC_REQUIRE_FALSE(std::is_invocable_v<decltype(bell.onSilent) &>);
}

TEST_CASE("A const view of an owner can still be listened to", "[Event]")
{
    Bell bell;
    const Bell &seen = bell;
    int rung = 0;
    seen.onRung.connect([&](const std::string &) { ++rung; });

    bell.ring("ding");

    REQUIRE(rung == 1);
}
