#include <map>
#include <optional>
#include <string>
#include <vector>
#include <filesystem>
#include <glaze/glaze.hpp>
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
#include "test_helpers/asset_path.hpp"

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

    renaming.applied({"levels/level1.json", "levels/level2.json"});

    REQUIRE(renaming.sinceSaved().empty());
    REQUIRE(renaming.rePointedLevels() == "level1 and level2");
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

TEST_CASE("A rename nobody's level uses rewrites nothing", "[Renaming]")
{
    std::vector<std::string> rewritten = renameInLevels(
        std::string(assets::Levels),
        [](LevelData &levelData)
        { return renamePaletteIn(levelData.tileMapData, {{"nobody", "somebody"}}); });

    REQUIRE(rewritten.empty());
}

TEST_CASE("Renames written into the levels stop waiting", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming = renamedOnce(gui, "nobody", "somebody");

    REQUIRE_FALSE(renaming.sinceSaved().empty());

    writeRenamesIntoLevels(
        renaming,
        std::string(assets::Levels),
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

namespace
{
    std::filesystem::path someLevelsToRewrite()
    {
        std::filesystem::path directory =
            std::filesystem::temp_directory_path() / "platformer_renaming_levels";
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);

        for (const char *name : {"level1.json", "level2.json"})
            std::filesystem::copy_file(assetPath(std::string("levels/") + name), directory / name);

        return directory;
    }
}

TEST_CASE("One level re-pointed is named on its own", "[Renaming]")
{
    REQUIRE(levelsInAList({"levels/level1.json"}) == "level1");
}

TEST_CASE("Two levels re-pointed are joined by and", "[Renaming]")
{
    REQUIRE(levelsInAList({"levels/level1.json", "levels/level2.json"}) == "level1 and level2");
}

TEST_CASE("More levels re-pointed take commas up to the last", "[Renaming]")
{
    REQUIRE(
        levelsInAList({"levels/level1.json", "levels/level2.json", "levels/level5.json"}) ==
        "level1, level2 and level5");
}

TEST_CASE("No levels re-pointed says nothing", "[Renaming]")
{
    REQUIRE(levelsInAList({}).empty());
}

TEST_CASE("The levels a rename reaches are handed back", "[Renaming]")
{
    std::filesystem::path directory = someLevelsToRewrite();

    std::vector<std::string> rewritten = renameInLevels(
        directory.string(),
        [](LevelData &levelData)
        { return renamePaletteIn(levelData.tileMapData, {{"default", "base"}}); });

    REQUIRE(levelsInAList(rewritten) == "level1 and level2");

    std::filesystem::remove_all(directory);
}

TEST_CASE("The levels a rename reaches are named back", "[Renaming]")
{
    HeadlessImGui gui;
    Renaming renaming = renamedOnce(gui, "default", "base");
    std::filesystem::path directory = someLevelsToRewrite();

    writeRenamesIntoLevels(
        renaming,
        directory.string(),
        [](LevelData &levelData, const Renames &renames)
        { return renamePaletteIn(levelData.tileMapData, renames); });

    REQUIRE(renaming.sinceSaved().empty());
    REQUIRE(renaming.rePointedLevels() == "level1 and level2");

    LevelData rewritten;
    REQUIRE_FALSE(
        glz::read_file_json(rewritten, (directory / "level1.json").string(), std::string{}));
    REQUIRE(rewritten.tileMapData.tilePalette == "base");

    std::filesystem::remove_all(directory);
}

TEST_CASE("Renames that take effect move the keys they name", "[Renaming]")
{
    std::map<std::string, int> catalogue{{"villager", 1}, {"explorer", 2}};

    renamesTakeEffect({{"villager", "farmer"}}, catalogue);

    REQUIRE(catalogue == std::map<std::string, int>{{"farmer", 1}, {"explorer", 2}});
}

TEST_CASE("A rename naming nothing in the catalogue moves nothing", "[Renaming]")
{
    std::map<std::string, int> catalogue{{"villager", 1}};

    renamesTakeEffect({{"nobody", "somebody"}}, catalogue);

    REQUIRE(catalogue == std::map<std::string, int>{{"villager", 1}});
}

TEST_CASE("A name is what the renames make of it", "[Renaming]")
{
    REQUIRE(nameAfterRenames({{"villager", "farmer"}}, "villager") == "farmer");
    REQUIRE(nameAfterRenames({{"villager", "farmer"}}, "explorer") == "explorer");
}
