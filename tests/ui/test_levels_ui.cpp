#include <algorithm>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "game/levels_data.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/temporary_levels.hpp"
#include "ui/editor_commands.hpp"
#include "assets/asset_paths.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include "helpers/headless_imgui.hpp"
#include "helpers/palettes.hpp"
#include "rendering/texture_cache.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "ui/armed.hpp"
#include "ui/levels_ui.hpp"
#include "ui/tile_palettes_ui.hpp"
#include "ui/switching_level.hpp"

namespace
{
    const std::optional<std::string> Nothing;
    const std::optional<std::string> Another{"levels/level3.json"};
    const std::optional<std::string> Held{"levels/level5.json"};
}

TEST_CASE("Picking a level with nothing unsaved goes there", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, false, Nothing, false, false);

    REQUIRE(decided.loadNow == Another);
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Picking a level with unsaved changes waits rather than going", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, true, Nothing, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE(decided.waitingOn == Another);
}

TEST_CASE("Switching goes to the level that was waiting", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, true, Held, true, false);

    REQUIRE(decided.loadNow == Held);
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Cancelling goes nowhere and stops waiting", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, true, Held, false, true);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Waiting carries on until something is pressed", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, true, Held, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE(decided.waitingOn == Held);
}

TEST_CASE("Saving while it waits stops it waiting", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Nothing, false, Held, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

TEST_CASE("Picking again while it waits waits on the new one", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, true, Held, false, false);

    REQUIRE_FALSE(decided.loadNow.has_value());
    REQUIRE(decided.waitingOn == Another);
}

TEST_CASE("Saving then picking goes straight there", "[LevelsUi]")
{
    SwitchingLevel decided = switching(Another, false, Held, false, false);

    REQUIRE(decided.loadNow == Another);
    REQUIRE_FALSE(decided.waitingOn.has_value());
}

namespace
{
    LevelData aLevelWhoseNextIs(const std::string &nextLevel)
    {
        LevelData levelData = aFloorLevelPlacing({});
        levelData.nextLevel = nextLevel;
        return levelData;
    }

    struct Editing
    {
        TemporaryLevels files{"levels_ui"};
        std::string directory = files.directory.generic_string();
        LevelsUi levelsUi{directory, [this](const LevelsData &written) { saved = written; }};
        EditorCommands commands;
        LevelsData levels;
        std::optional<LevelsData> saved;
        std::optional<std::string> loaded;
        std::vector<std::pair<std::string, LevelData>> played;

        Editing()
        {
            files.write("level1.json", aLevelWhoseNextIs(named("level2.json")));
            files.write("level2.json", aLevelWhoseNextIs(named("level1.json")));
            levels.first = named("level1.json");
            REQUIRE_FALSE(levelsUi.unsavedSince(levels));
            std::ignore =
                commands.onLoadLevel.connect([this](const std::string &path) { loaded = path; });
            std::ignore = commands.onPlayLevel.connect(
                [this](const std::string &path, const LevelData &levelData)
                { played.emplace_back(path, levelData); });
        }

        std::string named(const std::string &file) const
        {
            return assets::underRoot(files.pathOf(file));
        }

        LevelData playing() const
        {
            return readLevelData(named("level1.json"));
        }
    };
}

TEST_CASE("A level added is an empty one under a name nobody has taken", "[LevelsUi]")
{
    Editing editing;

    std::string made = editing.levelsUi.add(editing.playing(), TestTileSize, editing.commands);
    editing.commands.drain();

    REQUIRE(made != editing.named("level1.json"));
    REQUIRE_FALSE(readLevelDataIfYouCan(made));
    REQUIRE(editing.played.size() == 1);
    REQUIRE(editing.played.front().first == made);
    REQUIRE(editing.played.front().second.tileMapData.indices[FloorLevelRow][0] == 0);
}

TEST_CASE("A level added is offered before it has a file", "[LevelsUi]")
{
    Editing editing;

    std::string made = editing.levelsUi.add(editing.playing(), TestTileSize, editing.commands);
    std::vector<std::string> offers = editing.levelsUi.offered();

    REQUIRE(std::find(offers.begin(), offers.end(), made) != offers.end());
}

TEST_CASE("Two levels added do not take the same name", "[LevelsUi]")
{
    Editing editing;

    std::string first = editing.levelsUi.add(editing.playing(), TestTileSize, editing.commands);
    std::string second = editing.levelsUi.add(editing.playing(), TestTileSize, editing.commands);

    REQUIRE(first != second);
}

TEST_CASE("A level added and removed before a save is offered no more", "[LevelsUi]")
{
    Editing editing;

    std::string made = editing.levelsUi.add(editing.playing(), TestTileSize, editing.commands);
    editing.levelsUi.remove(made, editing.commands);

    std::vector<std::string> offers = editing.levelsUi.offered();
    REQUIRE(std::find(offers.begin(), offers.end(), made) == offers.end());
    REQUIRE_FALSE(editing.levelsUi.unsavedSince(editing.levels));
}

TEST_CASE("A level removed stays on disk until the save, and another plays", "[LevelsUi]")
{
    Editing editing;
    REQUIRE_FALSE(editing.levelsUi.unsavedSince(editing.levels));

    editing.levelsUi.remove(editing.named("level2.json"), editing.commands);
    editing.commands.drain();

    REQUIRE(readLevelDataIfYouCan(editing.named("level2.json")));
    REQUIRE(editing.loaded == editing.named("level1.json"));
    REQUIRE(editing.levelsUi.unsavedSince(editing.levels));

    std::vector<std::string> offers = editing.levelsUi.offered();
    REQUIRE(std::find(offers.begin(), offers.end(), editing.named("level2.json")) == offers.end());
}

TEST_CASE("Saving a removal deletes the file and re-points what named it", "[LevelsUi]")
{
    Editing editing;
    editing.levelsUi.remove(editing.named("level2.json"), editing.commands);

    LevelData playing = editing.playing();
    REQUIRE(editing.levelsUi.save(editing.levels, playing));

    REQUIRE_FALSE(readLevelDataIfYouCan(editing.named("level2.json")));
    REQUIRE(
        readLevelData(editing.named("level1.json")).nextLevel ==
        LevelPathData(editing.named("level1.json")));
    REQUIRE(playing.nextLevel == LevelPathData(editing.named("level1.json")));
    REQUIRE_FALSE(editing.levelsUi.unsavedSince(editing.levels));
}

TEST_CASE("Removing the level the game starts on re-points it on the save", "[LevelsUi]")
{
    Editing editing;
    editing.levelsUi.remove(editing.named("level1.json"), editing.commands);

    LevelData playing = editing.playing();
    std::ignore = editing.levelsUi.save(editing.levels, playing);

    REQUIRE(editing.levels.first == LevelPathData(editing.named("level2.json")));
    REQUIRE(editing.saved->first == LevelPathData(editing.named("level2.json")));
}

TEST_CASE("Removing the only level refuses the save while it is the first", "[LevelsUi]")
{
    Editing editing;
    editing.levelsUi.remove(editing.named("level2.json"), editing.commands);
    LevelData playing = editing.playing();
    std::ignore = editing.levelsUi.save(editing.levels, playing);
    REQUIRE_FALSE(editing.levelsUi.cannotSaveBecause(editing.levels));

    editing.levelsUi.remove(editing.named("level1.json"), editing.commands);

    std::optional<std::string> cannot = editing.levelsUi.cannotSaveBecause(editing.levels);
    REQUIRE(cannot);
    REQUIRE_THAT(*cannot, Catch::Matchers::ContainsSubstring("level1"));
}

TEST_CASE("Reverting puts back a level that was removed", "[LevelsUi]")
{
    Editing editing;
    editing.levelsUi.remove(editing.named("level2.json"), editing.commands);

    std::string playing = editing.levelsUi.revert(editing.levels, editing.named("level1.json"));

    REQUIRE(playing == editing.named("level1.json"));
    std::vector<std::string> offers = editing.levelsUi.offered();
    REQUIRE(std::find(offers.begin(), offers.end(), editing.named("level2.json")) != offers.end());
    REQUIRE_FALSE(editing.levelsUi.unsavedSince(editing.levels));
}

TEST_CASE("Reverting an added level goes back to one that has a file", "[LevelsUi]")
{
    Editing editing;
    std::string made = editing.levelsUi.add(editing.playing(), TestTileSize, editing.commands);

    std::string playing = editing.levelsUi.revert(editing.levels, made);

    REQUIRE(playing != made);
    REQUIRE(readLevelDataIfYouCan(playing));
}

TEST_CASE("The levels section shares no item with the palettes beneath it", "[LevelsUi]")
{
    HeadlessImGui gui;
    Editing editing;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = theOnlyPalette(aPaletteWithASolidTile());
    TextureCache textures;
    std::optional<Armed> armed;
    LevelData playing = editing.playing();

    auto drawing = [&]
    {
        std::ignore = editing.levelsUi.draw(
            editing.levels,
            playing,
            editing.named("level1.json"),
            TestTileSize,
            editing.commands,
            false);
        tilePalettesUi.draw(palettes, textures, editing.commands, armed);
    };

    REQUIRE_NOTHROW(gui.frame(drawing));
    REQUIRE_NOTHROW(gui.frame(drawing));
}
