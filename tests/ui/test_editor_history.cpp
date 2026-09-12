#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "helpers/floor_level.hpp"
#include "ui/editor_history.hpp"
#include "ui/editor_section.hpp"

namespace
{
    LevelData aLevelCalled(const std::string &nextLevel)
    {
        LevelData levelData = aFloorLevelPlacing({});
        levelData.nextLevel.path = nextLevel;
        return levelData;
    }

    EditorStep aCastStep(const std::string &gameData)
    {
        return EditorStep{EditorSection::Level, gameData, std::nullopt, glm::vec2(0.0f)};
    }
}

TEST_CASE("A history nobody has told anything has nothing to undo", "[EditorHistory]")
{
    EditorHistory history;

    REQUIRE_FALSE(history.anythingToUndo());
    REQUIRE_FALSE(history.stepBack());
}

TEST_CASE("A step back is the level as it was", "[EditorHistory]")
{
    EditorHistory history;
    history.remembers(aLevelCalled("levels/level1.json"));

    REQUIRE(history.anythingToUndo());

    std::optional<EditorStep> back = history.stepBack();
    REQUIRE(back);
    REQUIRE(back->section == EditorSection::Level);
    REQUIRE(back->levelData->nextLevel.path == "levels/level1.json");
    REQUIRE_FALSE(back->gameData);
    REQUIRE(back->movingThePlayerBack == glm::vec2(0.0f));
    REQUIRE_FALSE(history.anythingToUndo());
}

TEST_CASE("A step back is the game data as it was, and where it was edited", "[EditorHistory]")
{
    EditorHistory history;
    history.remembers(aCastStep("{\"npcData\":{}}"));

    std::optional<EditorStep> back = history.stepBack();
    REQUIRE(back);
    REQUIRE(back->section == EditorSection::Level);
    REQUIRE(back->gameData == "{\"npcData\":{}}");
    REQUIRE_FALSE(back->levelData);
}

TEST_CASE("Steps of either kind come back in the order they were taken", "[EditorHistory]")
{
    EditorHistory history;
    history.remembers(aLevelCalled("levels/level1.json"));
    history.remembers(aCastStep("the cast"));
    history.remembers(aLevelCalled("levels/level2.json"));

    REQUIRE(history.stepBack()->levelData->nextLevel.path == "levels/level2.json");
    REQUIRE(history.stepBack()->gameData == "the cast");
    REQUIRE(history.stepBack()->levelData->nextLevel.path == "levels/level1.json");
    REQUIRE_FALSE(history.stepBack());
}

TEST_CASE("A step remembers how far the player has to move back", "[EditorHistory]")
{
    EditorHistory history;
    history.remembers(aFloorLevelPlacing({}), glm::vec2(-16.0f, 0.0f));

    REQUIRE(history.stepBack()->movingThePlayerBack == glm::vec2(-16.0f, 0.0f));
}

TEST_CASE("The oldest step is the one forgotten when there are too many", "[EditorHistory]")
{
    EditorHistory history;
    for (std::size_t step = 0; step < StepsRemembered + 1; ++step)
        history.remembers(aLevelCalled("levels/level" + std::to_string(step) + ".json"));

    for (std::size_t step = StepsRemembered; step > 0; --step)
        REQUIRE(
            history.stepBack()->levelData->nextLevel.path ==
            "levels/level" + std::to_string(step) + ".json");

    REQUIRE_FALSE(history.anythingToUndo());
}

TEST_CASE("A history that forgets has nothing to undo", "[EditorHistory]")
{
    EditorHistory history;
    history.remembers(aFloorLevelPlacing({}));

    history.forgets();

    REQUIRE_FALSE(history.anythingToUndo());
}
