#include <map>
#include <optional>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "game/renames.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "tile_map/tile_map_data.hpp"
#include "ui/renaming.hpp"
#include "ui/saveable.hpp"

namespace
{
    bool nobodyHasIt(const std::string &)
    {
        return false;
    }

    Renaming renamedOnce(HeadlessImGui &gui, const std::string &from, const std::string &to)
    {
        Renaming renaming;
        auto drawing = [&] { renaming.draw("a palette", from, nobodyHasIt); };

        gui.type("##name", to.c_str(), drawing);
        gui.pressEnter(drawing);

        return renaming;
    }
}

TEST_CASE("A rename needs a name nobody has taken", "[Renaming]")
{
    REQUIRE(whyNotARename("a palette", "default", "", false).has_value());
    REQUIRE_FALSE(whyNotARename("a palette", "default", "base", false).has_value());
}

TEST_CASE("A rename to the name it already has is allowed and does nothing", "[Renaming]")
{
    REQUIRE_FALSE(whyNotARename("a palette", "default", "default", true).has_value());
}

TEST_CASE("A rename cannot take a name already taken", "[Renaming]")
{
    std::optional<std::string> why = whyNotARename("a pickup", "coin", "gem", true);

    REQUIRE(why.has_value());
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring("already a pickup"));
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring("gem"));
}

TEST_CASE("What cannot be renamed is named in the reason", "[Renaming]")
{
    REQUIRE(*whyNotARename("an npc", "villager", "", false) == "an npc needs a name");
}

TEST_CASE("Renaming twice remembers the name that is on disk", "[Renaming]")
{
    Renames renames;

    rememberRename(renames, "default", "base");
    REQUIRE(renames == Renames{{"default", "base"}});

    rememberRename(renames, "base", "ground");
    REQUIRE(renames == Renames{{"default", "ground"}});
}

TEST_CASE("Renaming back to the name on disk remembers nothing", "[Renaming]")
{
    Renames renames;

    rememberRename(renames, "default", "base");
    rememberRename(renames, "base", "default");

    REQUIRE(renames.empty());
}

TEST_CASE("A rename waits to be written into the levels", "[Renaming]")
{
    HeadlessImGui gui;

    Renaming renaming = renamedOnce(gui, "default", "base");

    REQUIRE(renaming.sinceSaved() == Renames{{"default", "base"}});
}

TEST_CASE("Forgetting drops a rename that was never written", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming = renamedOnce(gui, "default", "base");

    renaming.forget();

    REQUIRE(renaming.sinceSaved().empty());
}

TEST_CASE("A rename already written is not written again", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming = renamedOnce(gui, "default", "base");

    renaming.applied(2);

    REQUIRE(renaming.sinceSaved().empty());
}

TEST_CASE("Forgetting puts the field back to what is selected", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming;
    std::optional<Renamed> renamed;
    auto drawing = [&]
    {
        if (std::optional<Renamed> said = renaming.draw("a palette", "default", nobodyHasIt))
            renamed = said;
    };

    gui.type("##name", "base", drawing);
    gui.stopTyping(drawing);

    renaming.forget();

    renamed.reset();
    gui.type("##name", "", drawing);
    gui.pressEnter(drawing);

    REQUIRE_FALSE(renamed.has_value());
}

TEST_CASE("A palette a level names is re-pointed", "[Renaming]")
{
    TileMapData tileMapData;
    tileMapData.tilePalette = "default";

    REQUIRE(renamePaletteIn(tileMapData, {{"default", "base"}}));
    REQUIRE(tileMapData.tilePalette == "base");
}

TEST_CASE("A palette no level names is left alone", "[Renaming]")
{
    TileMapData tileMapData;
    tileMapData.tilePalette = "default";

    REQUIRE_FALSE(renamePaletteIn(tileMapData, {{"ice", "snow"}}));
    REQUIRE(tileMapData.tilePalette == "default");
}

TEST_CASE("Every spawn of a renamed npc type is re-pointed", "[Renaming]")
{
    std::vector<NpcSpawnData> npcs{
        NpcSpawnData{"villager", glm::ivec2(1, 1), std::nullopt},
        NpcSpawnData{"explorer", glm::ivec2(2, 2), std::nullopt},
        NpcSpawnData{"villager", glm::ivec2(3, 3), std::nullopt}};

    REQUIRE(renameTypeIn(npcs, {{"villager", "farmer"}}));

    REQUIRE(npcs[0].type == "farmer");
    REQUIRE(npcs[1].type == "explorer");
    REQUIRE(npcs[2].type == "farmer");
}

TEST_CASE("Spawns of a type nobody renamed are left alone", "[Renaming]")
{
    std::vector<NpcSpawnData> npcs{NpcSpawnData{"villager", glm::ivec2(1, 1), std::nullopt}};

    REQUIRE_FALSE(renameTypeIn(npcs, {{"explorer", "scout"}}));
    REQUIRE(npcs[0].type == "villager");
}

TEST_CASE("Every spawn of a renamed pickup type is re-pointed", "[Renaming]")
{
    std::vector<PickupSpawnData> pickups{
        PickupSpawnData{"coin", glm::ivec2(1, 1)}, PickupSpawnData{"gem", glm::ivec2(2, 2)}};

    REQUIRE(renameTypeIn(pickups, {{"coin", "penny"}}));

    REQUIRE(pickups[0].type == "penny");
    REQUIRE(pickups[1].type == "gem");
}

TEST_CASE("A graph named after several types keeps the ones nobody renamed", "[Renaming]")
{
    REQUIRE(
        renamedLabel("player, villager, explorer", {{"villager", "farmer"}}) ==
        "player, farmer, explorer");
}

TEST_CASE("A graph named after one type takes the new name", "[Renaming]")
{
    REQUIRE(renamedLabel("villager", {{"villager", "farmer"}}) == "farmer");
}

TEST_CASE("A rename nobody's level uses rewrites nothing", "[Renaming]")
{
    int rewritten = renameInLevels(
        std::string(assets::Levels),
        [](LevelData &levelData)
        { return renamePaletteIn(levelData.tileMapData, {{"nobody", "somebody"}}); });

    REQUIRE(rewritten == 0);
}

TEST_CASE("Renames written into the levels stop waiting", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming = renamedOnce(gui, "nobody", "somebody");

    REQUIRE_FALSE(renaming.sinceSaved().empty());

    writeRenamesIntoLevels(
        renaming,
        [](LevelData &levelData, const Renames &renames)
        { return renamePaletteIn(levelData.tileMapData, renames); });

    REQUIRE(renaming.sinceSaved().empty());
}

TEST_CASE("Putting values back forgets the renaming that went with them", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming = renamedOnce(gui, "default", "base");

    Saveable saveable;
    std::map<std::string, int> values{{"base", 1}};
    saveable.saved("values", R"({"default":1})");

    revertTo(saveable, "values", values, renaming);

    REQUIRE(values == std::map<std::string, int>{{"default", 1}});
    REQUIRE(renaming.sinceSaved().empty());
}

TEST_CASE("The field follows what is selected", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming;
    std::string selected = "default";
    std::optional<Renamed> renamed;
    auto drawing = [&]
    {
        if (std::optional<Renamed> said = renaming.draw("a palette", selected, nobodyHasIt))
            renamed = said;
    };

    gui.type("##name", "base", drawing);
    gui.stopTyping(drawing);

    selected = "ice";
    gui.type("##name", "", drawing);
    gui.pressEnter(drawing);

    REQUIRE_FALSE(renamed.has_value());
}
