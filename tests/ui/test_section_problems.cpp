#include <catch2/catch_test_macros.hpp>
#include <array>
#include <optional>
#include <string>
#include <vector>
#include "ui/editor_section.hpp"
#include "ui/section_problems.hpp"

TEST_CASE("A panel with nothing wrong has no problems to name", "[SectionProblems]")
{
    std::array<std::optional<std::string>, EditorSections.size()> nothing;

    REQUIRE(problemsAmong(nothing).empty());
}

TEST_CASE("A problem is named with the section it is in", "[SectionProblems]")
{
    std::array<std::optional<std::string>, EditorSections.size()> whyNotSaved;
    whyNotSaved[1] = "the window is too small";

    std::vector<SectionProblem> problems = problemsAmong(whyNotSaved);

    REQUIRE(problems.size() == 1);
    REQUIRE(problems.front().section == EditorSections[1].first);
    REQUIRE(problems.front().name == EditorSections[1].second);
    REQUIRE(problems.front().because == "the window is too small");
}

TEST_CASE(
    "Every section that cannot save is named, in the order they are shown",
    "[SectionProblems]")
{
    std::array<std::optional<std::string>, EditorSections.size()> whyNotSaved;
    whyNotSaved.front() = "first";
    whyNotSaved.back() = "last";

    std::vector<SectionProblem> problems = problemsAmong(whyNotSaved);

    REQUIRE(problems.size() == 2);
    REQUIRE(problems.front().section == EditorSections.front().first);
    REQUIRE(problems.back().section == EditorSections.back().first);
}
