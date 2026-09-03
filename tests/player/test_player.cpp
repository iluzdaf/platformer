#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <glaze/glaze.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/actor_motion_state.hpp"
#include "actor/actor_animation_state.hpp"
#include <vector>
#include "timing/fixed_time_step.hpp"
#include "input/input_intentions.hpp"
#include "player/player.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/test_player_utils.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "test_helpers/asset_path.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile_data.hpp"

using Catch::Approx;

namespace
{
    void simulatePlayer(
        Player &player,
        ScriptedIntentions &input,
        TileMap &tileMap,
        float totalTime,
        InputIntentions intentions = InputIntentions(),
        float step = 0.01f,
        const TilePalette &palette = getDefaultTileDataMap())
    {
        LevelData levelData;
        levelData.playerStart = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData = tileMap.toTileMapData();

        Level level(levelData, palettesFrom(palette), setupPlayerData(), {}, {});

        FixedTimeStep timeStepper(step);
        input.set(intentions);
        timeStepper.run(
            totalTime,
            [&](float dt)
            {
                player.fixedUpdate(dt, level);
                player.postFixedUpdate();
            });
    }
}

TEST_CASE("Player falls under normal gravity", "[Player]")
{
    ScriptedIntentions input;
    Player player = setupPlayer(input);
    const float gravity = GravityAbilityData().gravity;
    TileMap tileMap = setupTileMap(1, static_cast<int>(gravity / 16.0f) + 2);
    simulatePlayer(player, input, tileMap, 1.0f);
    const ActorMotionState &state = player.getMotion().getState();
    REQUIRE(state.velocity.y == Approx(gravity));
    REQUIRE(player.getPosition().y == Approx(0.5f * gravity).margin(5));
}

TEST_CASE("Player sets onGround correctly", "[Player]")
{
    TileMap tileMap = setupTileMap();
    ScriptedIntentions input;
    Player player = setupPlayer(input);

    SECTION("Player lands on solid tile")
    {
        tileMap.setTileIndex(glm::ivec2(0, 5), 1);
        simulatePlayer(player, input, tileMap, 1.0f);
        float expectedY = static_cast<float>(4 * tileMap.getTileSize());
        const ActorMotionState &state = player.getMotion().getState();
        REQUIRE(player.getPosition().y == Approx(expectedY));
        REQUIRE(state.contacts.onGround);
        REQUIRE(state.velocity.y == Approx(0.0f).margin(0.01f));
    }

    SECTION("Player walks off a ledge and is no longer onGround")
    {
        tileMap.setTileIndex(glm::ivec2(1, 5), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition({2 * 16, 4 * 16});
        simulatePlayer(player, input, tileMap, 0.1f);
        const ActorMotionState &state = player.getMotion().getState();
        REQUIRE(state.contacts.onGround);
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, input, tileMap, 0.2f, inputIntentions);
        REQUIRE_FALSE(state.contacts.onGround);
    }
}

TEST_CASE("Player uses correct animation state", "[Player]")
{
    TileMap tileMap = setupTileMap();
    ScriptedIntentions input;
    Player player = setupPlayer(input);
    player.setPosition({5 * 16, 9 * 16});

    SECTION("Player is idle by default")
    {
        simulatePlayer(player, input, tileMap, 0.1f);
        REQUIRE(player.getState().currentAnimationState == ActorAnimationState::Idle);
    }

    SECTION("Player walking triggers walk animation")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(player.getState().currentAnimationState == ActorAnimationState::Walk);
        simulatePlayer(player, input, tileMap, 0.1f);
        REQUIRE(player.getState().currentAnimationState == ActorAnimationState::Idle);
        inputIntentions = InputIntentions();
        inputIntentions.direction.x = -1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(player.getState().currentAnimationState == ActorAnimationState::Walk);
    }

    SECTION("Animation frame advances over time")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        int frameBefore = player.getState().currentFrame;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        int frameAfter = player.getState().currentFrame;
        REQUIRE(frameBefore != frameAfter);
    }
}

TEST_CASE("Player sets facingLeft flag correctly", "[Player]")
{
    ScriptedIntentions input;
    Player player = setupPlayer(input);
    TileMap tileMap = setupTileMap();

    SECTION("Starts facing right")
    {
        const ActorState &playerState = player.getState();
        REQUIRE_FALSE(playerState.facingLeft);
    }

    SECTION("Moves left and faces left")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = -1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        const ActorState &playerState = player.getState();
        REQUIRE(playerState.facingLeft);
        simulatePlayer(player, input, tileMap, 0.1f);
        REQUIRE(playerState.facingLeft);
    }

    SECTION("Moves right and faces right")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        const ActorState &playerState = player.getState();
        REQUIRE_FALSE(playerState.facingLeft);
        simulatePlayer(player, input, tileMap, 0.1f);
        REQUIRE_FALSE(playerState.facingLeft);
    }
}

TEST_CASE("Player and tilemap bounds", "[Player]")
{
    TileMap tileMap = setupTileMap();
    ScriptedIntentions input;
    Player player = setupPlayer(input);

    SECTION("Player stays within bounds")
    {
        player.setPosition(glm::vec2(16, 32));

        SECTION("While falling")
        {
            simulatePlayer(player, input, tileMap, 10.0f);
            REQUIRE(player.getPosition().x <= tileMap.getWorldWidth());
            REQUIRE(player.getPosition().y <= tileMap.getWorldHeight());
        }

        SECTION("While moving left")
        {
            InputIntentions inputIntentions;
            inputIntentions.direction.x = -1;
            simulatePlayer(player, input, tileMap, 10.0f, inputIntentions);
            REQUIRE(player.getPosition().x <= tileMap.getWorldWidth());
            REQUIRE(player.getPosition().y <= tileMap.getWorldHeight());
        }

        SECTION("While moving right")
        {
            InputIntentions inputIntentions;
            inputIntentions.direction.x = 1;
            simulatePlayer(player, input, tileMap, 10.0f, inputIntentions);
            REQUIRE(player.getPosition().x <= tileMap.getWorldWidth());
            REQUIRE(player.getPosition().y <= tileMap.getWorldHeight());
        }
    }
}

TEST_CASE("Player sets wall touch flags correctly", "[Player]")
{
    TileMap tileMap = setupTileMap();
    ScriptedIntentions input;
    Player player = setupPlayer(input);
    const ActorMotionState &state = player.getMotion().getState();

    SECTION("Touching right wall")
    {
        for (int y = 0; y < 10; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(6, y), 1);
        }
        player.setPosition(glm::vec2(5 * 16.0f, 16.0f));
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(state.contacts.touchingRightWall);
        REQUIRE_FALSE(state.contacts.touchingLeftWall);
    }

    SECTION("Touching left wall")
    {
        for (int y = 0; y < 10; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(3, y), 1);
        }
        player.setPosition(glm::vec2(4 * 16.0f, 16.0f));
        InputIntentions inputIntentions;
        inputIntentions.direction.x = -1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(state.contacts.touchingLeftWall);
        REQUIRE_FALSE(state.contacts.touchingRightWall);
    }
}

TEST_CASE("Player event callbacks are triggered", "[Player]")
{
    TileMap tileMap = setupTileMap(10, 20);
    for (int x = 0; x < 10; ++x)
    {
        tileMap.setTileIndex(glm::ivec2(x, 19), 1);
    }
    ScriptedIntentions input;
    Player player = setupPlayer(input);

    SECTION("onFallFromHeight")
    {
        player.setPosition(glm::vec2(0, 0));
        bool fallFromHeightTriggered = false;
        player.onFallFromHeight.connect([&] { fallFromHeightTriggered = true; });
        simulatePlayer(player, input, tileMap, 1.5f);
        REQUIRE(fallFromHeightTriggered);
    }

    SECTION("onHitCeiling")
    {
        tileMap.setTileIndex(glm::ivec2(2, 2), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition({2 * 16, 4 * 16});
        simulatePlayer(player, input, tileMap, 0.01f);
        bool hitCeilingTriggered = false;
        player.onHitCeiling.connect([&] { hitCeilingTriggered = true; });
        InputIntentions inputIntentions;
        inputIntentions.jumpRequested = true;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(hitCeilingTriggered);
    }
}

TEST_CASE("Player movement ability integration", "[Player]")
{
    TileMap tileMap = setupTileMap(20, 10);
    ScriptedIntentions input;
    Player player = setupPlayer(input);
    InputIntentions inputIntentions;

    SECTION("Player cannot move into solid tile")
    {
        tileMap.setTileIndex(glm::ivec2(3, 5), 1);
        tileMap.setTileIndex(glm::ivec2(2, 4), 1);
        tileMap.setTileIndex(glm::ivec2(1, 4), 1);
        tileMap.setTileIndex(glm::ivec2(1, 5), 1);
        player.setPosition(glm::vec2(2 * tileMap.getTileSize(), 5 * tileMap.getTileSize()));

        SECTION("Moving right into solid tile")
        {
            inputIntentions.direction.x = 1;
            simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
            REQUIRE(player.getPosition().x <= Approx(3 * tileMap.getTileSize()));
        }

        SECTION("Moving left into solid tile")
        {
            inputIntentions.direction.x = -1;
            simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
            REQUIRE(player.getPosition().x >= Approx(1 * tileMap.getTileSize()));
        }
    }

    SECTION("Player cannot jump through solid tile")
    {
        int ceilingTileX = 2;
        int ceilingTileY = 2;
        tileMap.setTileIndex(glm::ivec2(ceilingTileX, ceilingTileY), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition({2 * tileMap.getTileSize(), 4 * tileMap.getTileSize()});
        inputIntentions.jumpRequested = true;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        float playerTopY = player.getPosition().y;
        float ceilingBottomY = static_cast<float>(ceilingTileY + 1);
        REQUIRE(playerTopY >= Approx(ceilingBottomY).margin(0.1f));
    }
}

TEST_CASE("Sliding into the bottom corner of a wall does not wedge the player", "[Player]")
{
    TileMap tileMap = setupTileMap(20, 20);
    constexpr int LedgeRow = 5;
    constexpr int LedgeLastTile = 6;
    for (int x = 0; x <= LedgeLastTile; ++x)
        tileMap.setTileIndex(glm::ivec2(x, LedgeRow), 1);
    for (int x = 0; x < 20; ++x)
        tileMap.setTileIndex(glm::ivec2(x, 12), 1);

    LevelData levelData;

    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    levelData.tileMapData = tileMap.toTileMapData();
    Level level(levelData, palettesFrom(getDefaultTileDataMap()), setupPlayerData(), {}, {});

    ScriptedIntentions input;
    Player player(setupPlayerData(), input);

    float ledgeRight = static_cast<float>(LedgeLastTile + 1) * 16.0f;
    float ledgeTop = static_cast<float>(LedgeRow) * 16.0f;
    player.setPosition(
        glm::vec2(ledgeRight - player.getPhysicsBody().getColliderOffset().x, ledgeTop - 2.0f));

    InputIntentions intentions;
    intentions.direction.x = -1.0f;
    for (int step = 0; step < 300; ++step)
    {
        input.set(intentions);
        player.fixedUpdate(0.01f, level);
        player.postFixedUpdate();
    }

    float colliderTop = player.getPosition().y + player.getPhysicsBody().getColliderOffset().y;
    INFO(
        "collider top ended at " << colliderTop << ", the ledge spans " << ledgeTop << " to "
                                 << ledgeTop + 16.0f);
    REQUIRE(colliderTop > ledgeTop + 16.0f);
}

namespace
{
    constexpr int PitMapWidth = 30;
    constexpr int PitMapHeight = 12;
    constexpr int PitFloorRow = 8;
    constexpr int PitHazardRow = 7;
    constexpr int PitStart = 12;

    enum class Pit
    {
        Spikes,
        Hole,
        StepUp
    };

    GameData shippedGameData()
    {
        GameData gameData = loadGameData();
        return gameData;
    }

    TileMapData pitOf(int tiles, Pit kind, const TilePalettes &tilePalettes)
    {
        int solidTile = aSolidTileIn(tilePalettes.at("default"));
        int spikeTile = aDeadlyTileIn(tilePalettes.at("default"));

        std::vector<std::vector<int>> rows(PitMapHeight, std::vector<int>(PitMapWidth, 0));
        for (int x = 0; x < PitMapWidth; ++x)
            rows[PitFloorRow][x] = solidTile;

        if (kind == Pit::StepUp)
            for (int x = PitStart; x < PitMapWidth; ++x)
                for (int y = std::max(0, PitFloorRow - tiles); y < PitMapHeight; ++y)
                    rows[y][x] = solidTile;
        else
            for (int x = PitStart; x < PitStart + tiles; ++x)
            {
                if (kind == Pit::Spikes)
                    rows[PitHazardRow][x] = spikeTile;
                else
                    rows[PitFloorRow][x] = 0;
            }

        TileMapData tileMapData;
        tileMapData.indices = rows;
        tileMapData.tilePalette = "default";
        return tileMapData;
    }

    bool getsAcross(const GameData &gameData, int tiles, Pit kind, bool jump, bool dash)
    {
        LevelData levelData;
        levelData.playerStart = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData = pitOf(tiles, kind, gameData.tilePalettes);
        Level level(
            levelData,
            gameData.tilePalettes,
            gameData.playerData,
            gameData.npcData,
            gameData.pickupData);

        float pitLeft = static_cast<float>(PitStart) * 16.0f;
        float pitRight = static_cast<float>(PitStart + tiles) * 16.0f;
        float floorY = static_cast<float>(PitFloorRow) * 16.0f;

        for (float triggerAt = pitLeft - 80.0f; triggerAt <= pitLeft; triggerAt += 1.0f)
            for (float dashAfter = 0.0f; dashAfter <= (jump && dash ? 0.45f : 0.0f);
                 dashAfter += 0.025f)
            {
                ScriptedIntentions input;
                Player player(gameData.playerData, input);
                player.setPosition(glm::vec2(4.0f * 16.0f, floorY - 16.0f));

                FixedTimeStep timestepper;
                float triggered = -1.0f;

                for (int frame = 0; frame < 240; ++frame)
                {
                    float now = frame / 60.0f;
                    InputIntentions intentions;
                    intentions.direction.x = 1.0f;

                    if (triggered < 0.0f && player.getPosition().x + 8.0f >= triggerAt)
                    {
                        triggered = now;
                        intentions.jumpRequested = jump;
                        intentions.dashRequested = dash && dashAfter <= 0.0f;
                    }
                    else if (triggered >= 0.0f)
                    {
                        intentions.jumpHeld = jump && now - triggered < 0.25f;
                        intentions.dashRequested = dash && dashAfter > 0.0f &&
                                                   now - triggered >= dashAfter &&
                                                   now - triggered < dashAfter + 1.0f / 60.0f;
                    }
                    input.set(intentions);

                    player.preFixedUpdate();
                    timestepper.run(1.0f / 60.0f, [&](float dt) { player.fixedUpdate(dt, level); });
                    player.postFixedUpdate();

                    glm::vec2 position = player.getPosition();
                    if (kind == Pit::StepUp)
                    {
                        float ledgeY = static_cast<float>(PitFloorRow - tiles) * 16.0f;
                        if (player.getMotion().getState().contacts.onGround &&
                            position.y + 16.0f <= ledgeY + 0.5f)
                            return true;
                        if (position.y > floorY)
                            break;
                        continue;
                    }

                    bool onSpikes = kind == Pit::Spikes && position.x + 12.0f > pitLeft &&
                                    position.x + 4.0f < pitRight &&
                                    position.y + 16.0f > static_cast<float>(PitHazardRow) * 16.0f &&
                                    position.y + 3.0f < floorY;
                    if (onSpikes || position.y > floorY)
                        break;

                    if (player.getMotion().getState().contacts.onGround &&
                        position.x + 4.0f > pitRight)
                        return true;
                }
            }

        return false;
    }
}

TEST_CASE("The shipped player's jump is worth three tiles", "[Player][Tuning]")
{
    GameData gameData = shippedGameData();

    REQUIRE(getsAcross(gameData, 3, Pit::StepUp, true, false));
    REQUIRE_FALSE(getsAcross(gameData, 4, Pit::StepUp, true, false));

    REQUIRE(getsAcross(gameData, 3, Pit::Hole, true, false));
    REQUIRE_FALSE(getsAcross(gameData, 4, Pit::Hole, true, false));

    REQUIRE(getsAcross(gameData, 2, Pit::Spikes, true, false));
    REQUIRE_FALSE(getsAcross(gameData, 3, Pit::Spikes, true, false));
}

TEST_CASE("The shipped player's dash is worth four tiles, and no spikes", "[Player][Tuning]")
{
    GameData gameData = shippedGameData();

    REQUIRE(getsAcross(gameData, 4, Pit::Hole, false, true));
    REQUIRE_FALSE(getsAcross(gameData, 5, Pit::Hole, false, true));

    REQUIRE_FALSE(getsAcross(gameData, 1, Pit::Spikes, false, true));
}

TEST_CASE("The shipped player's jump and dash together are worth five tiles", "[Player][Tuning]")
{
    GameData gameData = shippedGameData();

    REQUIRE(getsAcross(gameData, 5, Pit::Hole, true, true));
    REQUIRE_FALSE(getsAcross(gameData, 6, Pit::Hole, true, true));

    REQUIRE(getsAcross(gameData, 4, Pit::Spikes, true, true));
    REQUIRE_FALSE(getsAcross(gameData, 5, Pit::Spikes, true, true));

    REQUIRE(gameData.playerData.actorData.motionData.dashAbilityData->airborneFraction < 1.0f);
}

TEST_CASE("The shipped player can climb every step of level6", "[Player][Tuning]")
{
    GameData gameData = shippedGameData();
    LevelData levelData;
    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));
    Level level(
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    struct Step
    {
        const char *what;
        float edgeX, standOn, towards, intoPlatform, landOn;
    };

    for (Step step :
         {Step{"floor to the lowest platform", 192.0f, 192.0f, 1.0f, -1.0f, 160.0f},
          Step{"lowest to the middle platform", 192.0f, 160.0f, -1.0f, 1.0f, 128.0f},
          Step{"middle to the highest platform", 160.0f, 128.0f, -1.0f, -1.0f, 96.0f}})
    {
        int takeOffPointsThatWork = 0;
        for (float back = 0.0f; back <= 44.0f; back += 2.0f)
        {
            ScriptedIntentions input;
            Player player(gameData.playerData, input);
            player.setPosition(
                glm::vec2(step.edgeX + step.intoPlatform * back - 8.0f, step.standOn - 16.0f));

            FixedTimeStep timestepper;
            for (int frame = 0; frame < 150; ++frame)
            {
                InputIntentions intentions;
                intentions.jumpRequested = frame == 0;
                intentions.jumpHeld = frame < 20;
                intentions.direction.x = step.towards;
                input.set(intentions);

                player.preFixedUpdate();
                timestepper.run(1.0f / 60.0f, [&](float dt) { player.fixedUpdate(dt, level); });
                player.postFixedUpdate();

                if (player.getMotion().getState().contacts.onGround &&
                    std::abs(player.getPosition().y + 16.0f - step.landOn) < 0.5f)
                {
                    ++takeOffPointsThatWork;
                    break;
                }
            }
        }

        INFO(step.what << " worked from " << takeOffPointsThatWork << " take off points");
        REQUIRE(takeOffPointsThatWork >= 5);
    }
}

TEST_CASE("Level4's gap is a dash, and only a dash", "[Player][Tuning]")
{
    GameData gameData = shippedGameData();
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

                player.preFixedUpdate();
                timestepper.run(1.0f / 60.0f, [&](float dt) { player.fixedUpdate(dt, level); });
                player.postFixedUpdate();

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
    GameData gameData = shippedGameData();
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

TEST_CASE("A player can climb a wall and get onto the ledge", "[Player][Mantle]")
{
    TileMap tileMap = setupTileMap(12, 10);
    for (int x = 0; x < 12; ++x)
        tileMap.setTileIndex(glm::ivec2(x, 8), 1);
    for (int x = 5; x < 8; ++x)
        for (int y = 5; y < 8; ++y)
            tileMap.setTileIndex(glm::ivec2(x, y), 1);

    ScriptedIntentions input;
    Player player = setupPlayer(input);
    const ActorMotionState &state = player.getMotion().getState();

    player.setPosition(glm::vec2(4 * 16.0f + 4.0f, 8 * 16.0f - 16.0f));

    InputIntentions holdingTheWallAndPressingUp;
    holdingTheWallAndPressingUp.climbRequested = true;
    holdingTheWallAndPressingUp.direction = glm::vec2(1.0f, -1.0f);
    simulatePlayer(player, input, tileMap, 0.8f, holdingTheWallAndPressingUp);
    simulatePlayer(player, input, tileMap, 0.5f);

    REQUIRE(state.contacts.onGround);
    REQUIRE(player.getPhysicsBody().getAABB().bottomCenter().y == Approx(5 * 16.0f));
}

TEST_CASE("A player cannot hang on a wall it cannot grip", "[Player][Grip]")
{
    TilePalette palette = getDefaultTileDataMap();
    TileData ungrippable;
    ungrippable.solid = true;
    ungrippable.grippable = false;
    palette.tiles[2] = ungrippable;

    TileMap tileMap = setupTileMap(10, 10, 16, palette);
    for (int y = 0; y < 8; ++y)
        tileMap.setTileIndex(glm::ivec2(5, y), 2);

    ScriptedIntentions input;
    Player player = setupPlayer(input);
    const ActorMotionState &state = player.getMotion().getState();
    player.setPosition(glm::vec2(4 * 16.0f + 4.0f, 16.0f));

    InputIntentions holdingTheWall;
    holdingTheWall.climbRequested = true;
    holdingTheWall.direction = glm::vec2(1.0f, -1.0f);
    simulatePlayer(player, input, tileMap, 0.5f, holdingTheWall, 0.01f, palette);

    REQUIRE(state.contacts.touchingRightWall);
    REQUIRE_FALSE(state.contacts.grippableRightWall);
    REQUIRE_FALSE(state.wallHang.active);
    REQUIRE(state.velocity.y > 0.0f);
}

TEST_CASE("A ceiling bump is over before the frame it happened in ends", "[Player]")
{
    TileMap tileMap = setupTileMap(20, 20);
    for (int x = 0; x < 20; ++x)
    {
        tileMap.setTileIndex(glm::ivec2(x, 10), 1);
        tileMap.setTileIndex(glm::ivec2(x, 6), 1);
    }

    LevelData levelData;

    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    levelData.tileMapData = tileMap.toTileMapData();
    Level level(levelData, palettesFrom(getDefaultTileDataMap()), setupPlayerData(), {}, {});

    ScriptedIntentions input;
    Player player(setupPlayerData(), input);
    player.setPosition(glm::vec2(5 * 16.0f, 10 * 16.0f - 16.0f));

    FixedTimeStep timestepper;
    int stepsTouchingCeiling = 0;
    int framesEndingWithHitCeiling = 0;
    int framesEndingWithBumpedCeiling = 0;

    for (int frame = 0; frame < 180; ++frame)
    {
        InputIntentions intentions;
        intentions.jumpRequested = frame == 30;
        intentions.jumpHeld = frame >= 30 && frame < 45;
        input.set(intentions);

        player.preFixedUpdate();
        timestepper.run(
            1.0f / 60.0f,
            [&](float dt)
            {
                player.fixedUpdate(dt, level);
                if (player.getMotion().getState().contacts.hitCeiling)
                    ++stepsTouchingCeiling;
            });
        player.postFixedUpdate();

        const ActorContactState &contacts = player.getMotion().getState().contacts;
        if (contacts.hitCeiling)
            ++framesEndingWithHitCeiling;
        if (contacts.bumpedCeiling)
            ++framesEndingWithBumpedCeiling;
    }

    REQUIRE(stepsTouchingCeiling == 1);
    REQUIRE(framesEndingWithHitCeiling == 0);
    REQUIRE(framesEndingWithBumpedCeiling == 1);
}
