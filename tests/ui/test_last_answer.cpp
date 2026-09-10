#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "ui/last_answer.hpp"

TEST_CASE("A question asked again with the same words is answered from memory", "[LastAnswer]")
{
    LastAnswer gate;
    int asked = 0;
    auto compute = [&]() -> std::optional<std::string>
    {
        ++asked;
        return "no";
    };

    REQUIRE(gate.to("is it fine", compute) == "no");
    REQUIRE(gate.to("is it fine", compute) == "no");

    REQUIRE(asked == 1);
}

TEST_CASE("A question in new words is worked out afresh", "[LastAnswer]")
{
    LastAnswer gate;
    std::optional<std::string> answer = "no";
    auto compute = [&] { return answer; };

    REQUIRE(gate.to("is it fine", compute) == "no");
    answer = std::nullopt;
    REQUIRE(gate.to("is it fine", compute) == "no");
    REQUIRE_FALSE(gate.to("is it fine now", compute).has_value());
}

TEST_CASE("The first question is always worked out, even in no words at all", "[LastAnswer]")
{
    LastAnswer gate;
    int asked = 0;
    auto compute = [&]() -> std::optional<std::string>
    {
        ++asked;
        return std::nullopt;
    };

    REQUIRE_FALSE(gate.to("", compute).has_value());
    REQUIRE_FALSE(gate.to("", compute).has_value());
    REQUIRE(asked == 1);
}
