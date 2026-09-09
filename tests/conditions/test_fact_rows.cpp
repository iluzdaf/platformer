#include <array>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    struct Weather
    {
        bool raining = false;
        float degrees = 20.0f;
        std::string wind = "calm";
    };

    using Row = FactRow<Weather>;

    constexpr std::array Rows{
        Row{"raining",
            AskedKind::YesOrNo,
            "raining",
            "dry",
            [](const Asked &asked, const Weather &weather)
            { return std::get<bool>(asked) == weather.raining; },
            ""},
        Row{"warmerThan",
            AskedKind::Number,
            "warmer than",
            "",
            [](const Asked &asked, const Weather &weather)
            { return weather.degrees > std::get<float>(asked); },
            ""},
        Row{"wind",
            AskedKind::Name,
            "wind",
            "",
            [](const Asked &asked, const Weather &weather)
            { return std::get<std::string>(asked) == weather.wind; },
            ""},
    };

    std::span<const Row> rows()
    {
        return Rows;
    }
}

TEST_CASE("Asking nothing always holds and says always", "[FactRows]")
{
    std::map<std::string, Asked> nothing;

    REQUIRE(holds(nothing, rows(), Weather{}));
    REQUIRE(whenOf(nothing, rows()) == "always");
    REQUIRE_FALSE(whyNotAsked(nothing, rows()).has_value());
}

TEST_CASE("Every asked fact must agree, each compared the way its row says", "[FactRows]")
{
    std::map<std::string, Asked> asked{
        {"raining", false}, {"warmerThan", 15.0f}, {"wind", std::string("calm")}};

    REQUIRE(holds(asked, rows(), Weather{}));
    REQUIRE_FALSE(holds(asked, rows(), Weather{true, 20.0f, "calm"}));
    REQUIRE_FALSE(holds(asked, rows(), Weather{false, 10.0f, "calm"}));
    REQUIRE_FALSE(holds(asked, rows(), Weather{false, 20.0f, "gale"}));
}

TEST_CASE(
    "The words come from the rows, in the rows' order, whatever order was asked in",
    "[FactRows]")
{
    std::map<std::string, Asked> asked{
        {"wind", std::string("gale")}, {"raining", true}, {"warmerThan", 15.0f}};

    REQUIRE(whenOf(asked, rows()) == "raining, warmer than 15, wind \"gale\"");

    std::map<std::string, Asked> dry{{"raining", false}};
    REQUIRE(whenOf(dry, rows()) == "dry");
}

TEST_CASE(
    "A fact nobody publishes, or asked with the wrong kind of value, is refused with its name",
    "[FactRows]")
{
    std::map<std::string, Asked> unknown{{"snowing", true}};
    REQUIRE(whyNotAsked(unknown, rows()) == "asks about \"snowing\", and there is no such fact");

    std::map<std::string, Asked> wrongKind{{"raining", 3.0f}};
    REQUIRE(
        whyNotAsked(wrongKind, rows()) ==
        "asks \"raining\" with a number, and it wants a yes or no");

    std::map<std::string, Asked> wrongName{{"wind", true}};
    REQUIRE(
        whyNotAsked(wrongName, rows()) == "asks \"wind\" with a yes or no, and it wants a name");
}
