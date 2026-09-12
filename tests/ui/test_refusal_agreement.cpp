#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <exception>
#include <optional>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/texture_path_data.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/leaf_walk.hpp"
#include "helpers/small_game.hpp"
#include "ui/editor_section.hpp"
#include "ui/editor_ui.hpp"

namespace
{
    std::optional<std::string> whatTheGameSaysAbout(
        const GameData &gameData,
        const LevelData &levelData)
    {
        try
        {
            Camera2D camera(gameData.cameraData, 800, 600);
            Level level(
                levelData,
                gameData.tilePalettes,
                gameData.playerData,
                gameData.npcData,
                gameData.pickupData);
        }
        catch (const std::exception &refused)
        {
            return refused.what();
        }

        return std::nullopt;
    }

    std::optional<std::string> whatTheEditorSaysAbout(EditorUi &editorUi, const EditorSubject &of)
    {
        for (const auto &[section, name] : EditorSections)
            if (std::optional<std::string> because = editorUi.savingIn(section, of).cannotBecause)
                return because;

        return std::nullopt;
    }

    struct Asking
    {
        std::size_t refused = 0;
        std::vector<std::string> missed;
    };
}

TEST_CASE("What is suspicious of a value is never the value", "[RefusalAgreement]")
{
    auto neverItself = [](const auto &value)
    {
        for (const auto &bad : leaves::suspicious(value))
            if (bad == value)
                return false;

        return !leaves::suspicious(value).empty();
    };

    REQUIRE(neverItself(0.0f));
    REQUIRE(neverItself(-1.0f));
    REQUIRE(neverItself(0));
    REQUIRE(neverItself(-1));
    REQUIRE(neverItself(true));
    REQUIRE(neverItself(std::string()));
    REQUIRE(neverItself(std::string("nowhere")));
    REQUIRE(neverItself(glm::vec2(0.0f)));
    REQUIRE(neverItself(glm::ivec2(-1)));
    REQUIRE(neverItself(TexturePathData("nowhere")));
}

TEST_CASE("Nothing the game refuses to build can be saved", "[RefusalAgreement]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(whatTheGameSaysAbout(editing.gameData, editing.levelData));
    REQUIRE_FALSE(whatTheEditorSaysAbout(editorUi, subject));

    Asking asking;
    leaves::eachLeafOf(
        editing.gameData,
        [&](const std::string &at, auto &leaf)
        {
            auto was = leaf;
            for (const auto &bad : leaves::suspicious(leaf))
            {
                leaf = bad;
                if (std::optional<std::string> game =
                        whatTheGameSaysAbout(editing.gameData, editing.levelData))
                {
                    ++asking.refused;
                    if (!whatTheEditorSaysAbout(editorUi, subject))
                        asking.missed.push_back(at + ": " + *game);
                }
            }

            leaf = was;
        });

    std::string report = "fields the game refused: " + std::to_string(asking.refused);
    for (const std::string &missed : asking.missed)
        report += "\nsaved anyway: " + missed;

    INFO(report);
    REQUIRE(asking.refused > 0);
    REQUIRE(asking.missed.empty());
}
