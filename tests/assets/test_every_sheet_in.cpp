#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "assets/every_sheet_in.hpp"
#include "assets/sheet_data.hpp"

namespace shaped
{
    struct Held
    {
        SheetData sheet;
        int frame = 0;
    };

    struct Nowhere
    {
        int width = 0;
        std::string name;
    };

    struct Everywhere
    {
        SheetData mine;
        Held nested;
        std::optional<Held> perhaps;
        std::optional<Held> absent;
        std::vector<Held> several;
        std::map<std::string, Held> named;
        Nowhere plain;
    };
}

namespace
{
    std::vector<std::string> texturesIn(const shaped::Everywhere &data)
    {
        std::vector<std::string> found;
        everySheetIn(data, [&](const SheetData &sheet) { found.push_back(sheet.texture); });

        return found;
    }

    bool holds(const std::vector<std::string> &found, const std::string &texture)
    {
        return std::ranges::find(found, texture) != found.end();
    }

    shaped::Everywhere aSheetInEveryShape()
    {
        shaped::Everywhere data;
        data.mine.texture = "mine.png";
        data.nested.sheet.texture = "nested.png";
        data.perhaps = shaped::Held{};
        data.perhaps->sheet.texture = "perhaps.png";
        data.several.push_back(shaped::Held{});
        data.several.back().sheet.texture = "several.png";
        data.named["one"].sheet.texture = "named.png";
        data.plain.name = "not a sheet";

        return data;
    }
}

TEST_CASE("A sheet is found however deeply the data holds it", "[EverySheetIn]")
{
    std::vector<std::string> found = texturesIn(aSheetInEveryShape());

    REQUIRE(holds(found, "mine.png"));
    REQUIRE(holds(found, "nested.png"));
    REQUIRE(holds(found, "perhaps.png"));
    REQUIRE(holds(found, "several.png"));
    REQUIRE(holds(found, "named.png"));
}

TEST_CASE("An optional holding nothing offers no sheet", "[EverySheetIn]")
{
    REQUIRE(texturesIn(aSheetInEveryShape()).size() == 5);
}

TEST_CASE("Data naming no sheet at all is walked without one", "[EverySheetIn]")
{
    std::vector<std::string> found;
    everySheetIn(
        shaped::Nowhere{}, [&](const SheetData &sheet) { found.push_back(sheet.texture); });

    REQUIRE(found.empty());
}
