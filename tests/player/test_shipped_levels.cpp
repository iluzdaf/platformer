#include <string>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/tiles.hpp"
#include "input/input_intentions.hpp"
#include "player/player.hpp"
#include "tile_map/tile_map.hpp"
#include "timing/fixed_time_step.hpp"

TEST_CASE("Level4's gap is a dash, and only a dash", "[Player][Tuning]")
{
    GameData gameData = loadGameData();
    LevelData levelData;
    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level4.json"), std::string{}));
    Level level(
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    constexpr float GapLeft = 5 * 16.0f;
    constexpr float GapRight = 8 * 16.0f;
    glm::vec2 start = levelData.playerStart;

    auto runsAtItWith = [&](bool useDash)
    {
        int takeOffPointsThatWork = 0;
        for (float triggerAt = GapLeft - 60.0f; triggerAt <= GapLeft + 14.0f; triggerAt += 1.0f)
        {
            ScriptedIntentions input;
            Player player(gameData.playerData, input);
            player.setPosition(start - player.getPhysicsBody().getBottomCenterOffset());

            FixedTimeStep timestepper;
            bool triggered = false;
            int frameTriggered = 0;

            for (int frame = 0; frame < 240; ++frame)
            {
                InputIntentions intentions;
                intentions.direction.x = 1.0f;
                if (!triggered && player.getPosition().x + 8.0f >= triggerAt)
                {
                    triggered = true;
                    frameTriggered = frame;
                    intentions.dashRequested = useDash;
                    intentions.jumpRequested = !useDash;
                }
                else if (triggered && !useDash)
                    intentions.jumpHeld = frame - frameTriggered < 20;
                input.set(intentions);

                runFor(player, level, 1.0f / 60.0f, timestepper);

                glm::vec2 position = player.getPosition();
                if (position.y + 16.0f > 7 * 16.0f)
                    break;
                if (player.getMotion().getState().contacts.onGround && position.x + 4.0f > GapRight)
                {
                    ++takeOffPointsThatWork;
                    break;
                }
            }
        }
        return takeOffPointsThatWork;
    };

    REQUIRE(runsAtItWith(true) > 10);

    REQUIRE(runsAtItWith(false) == 0);
}

TEST_CASE("Level1 fits on screen, so the portal is in sight from the start", "[Level]")
{
    GameData gameData = loadGameData();
    LevelData levelData;
    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level1.json"), std::string{}));
    Level level(
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    float inView = static_cast<float>(gameData.settings.windowWidth) / gameData.cameraData.zoom;
    INFO("level is " << level.getTileMap().getWorldWidth() << "px, the camera shows " << inView);
    REQUIRE(static_cast<float>(level.getTileMap().getWorldWidth()) <= inView);

    bool hasPortal = false;
    const TileMap &tileMap = level.getTileMap();
    for (int x = 0; x < tileMap.getWidth(); ++x)
        for (int y = 0; y < tileMap.getHeight(); ++y)
            if (tileMap.getTileAtTilePosition(glm::ivec2(x, y)).isPortal())
                hasPortal = true;

    REQUIRE(hasPortal);
}
