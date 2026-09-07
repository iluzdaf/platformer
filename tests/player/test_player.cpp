#include <utility>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "input/input_intentions.hpp"
#include "player/player.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "timing/fixed_time_step.hpp"

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
        const TilePaletteData &palette = aPaletteWithASolidTile())
    {
        LevelData levelData;
        levelData.playerStart = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData = tileMap.toTileMapData();

        Level level(levelData, theOnlyPalette(palette), playerDataWithEveryAbility(), {}, {});

        FixedTimeStep timeStepper(step);
        input.set(intentions);
        runFor(player, level, totalTime, timeStepper);
    }
}

TEST_CASE("A player with nothing under it falls at its gravity", "[Player]")
{
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
    const float gravity = GravityAbilityData().gravity;
    TileMap tileMap = aTileMap({}, 1, static_cast<int>(gravity / 16.0f) + 2);
    simulatePlayer(player, input, tileMap, 1.0f);
    const ActorMotionState &state = player.getMotion().getState();
    REQUIRE(state.velocity.y == Approx(gravity));
    REQUIRE(player.getPosition().y == Approx(0.5f * gravity).margin(5));
}

TEST_CASE("A player knows when it is on the ground and when it has walked off", "[Player]")
{
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);

    SECTION("Player lands on solid tile")
    {
        TileMap tileMap = aTileMap({{{0, 5}, 1}});
        simulatePlayer(player, input, tileMap, 1.0f);
        float expectedY = static_cast<float>(4 * tileMap.getTileSize());
        const ActorMotionState &state = player.getMotion().getState();
        REQUIRE(player.getPosition().y == Approx(expectedY));
        REQUIRE(state.contacts.onGround);
        REQUIRE(state.velocity.y == Approx(0.0f).margin(0.01f));
    }

    SECTION("Player walks off a ledge and is no longer onGround")
    {
        TileMap tileMap = aTileMap({{{1, 5}, 1}, {{2, 5}, 1}});
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

TEST_CASE("A player's animation follows what it is doing", "[Player]")
{
    TileMap tileMap = aTileMap();
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
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

TEST_CASE("A player faces the way it last moved", "[Player]")
{
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
    TileMap tileMap = aTileMap();

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

TEST_CASE("A player is kept inside the map", "[Player]")
{
    TileMap tileMap = aTileMap();
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);

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

TEST_CASE("A player beside a wall knows which side it is on", "[Player]")
{
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
    const ActorMotionState &state = player.getMotion().getState();

    SECTION("Touching right wall")
    {
        std::vector<std::pair<glm::ivec2, int>> wall;
        for (int y = 0; y < 10; ++y)
            wall.push_back({glm::ivec2(6, y), 1});

        TileMap tileMap = aTileMap(wall);
        player.setPosition(glm::vec2(5 * 16.0f, 16.0f));
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(state.contacts.touchingRightWall);
        REQUIRE_FALSE(state.contacts.touchingLeftWall);
    }

    SECTION("Touching left wall")
    {
        std::vector<std::pair<glm::ivec2, int>> wall;
        for (int y = 0; y < 10; ++y)
            wall.push_back({glm::ivec2(3, y), 1});

        TileMap tileMap = aTileMap(wall);
        player.setPosition(glm::vec2(4 * 16.0f, 16.0f));
        InputIntentions inputIntentions;
        inputIntentions.direction.x = -1;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        REQUIRE(state.contacts.touchingLeftWall);
        REQUIRE_FALSE(state.contacts.touchingRightWall);
    }
}

TEST_CASE("A player raises the events for what it does", "[Player]")
{
    std::vector<std::pair<glm::ivec2, int>> laid;
    for (int x = 0; x < 10; ++x)
        laid.push_back({glm::ivec2(x, 19), 1});

    TileMap tileMap = aTileMap(laid, 10, 20);
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);

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
        TileMap ceiling = aTileMap({{{2, 2}, 1}, {{2, 5}, 1}});
        player.setPosition({2 * 16, 4 * 16});
        simulatePlayer(player, input, ceiling, 0.01f);
        bool hitCeilingTriggered = false;
        player.onHitCeiling.connect([&] { hitCeilingTriggered = true; });
        InputIntentions inputIntentions;
        inputIntentions.jumpRequested = true;
        simulatePlayer(player, input, ceiling, 0.1f, inputIntentions);
        REQUIRE(hitCeilingTriggered);
    }

    SECTION("onDash")
    {
        player.setPosition(glm::vec2(16, 19 * 16 - 16));
        simulatePlayer(player, input, tileMap, 0.1f);
        bool dashTriggered = false;
        player.onDash.connect([&] { dashTriggered = true; });
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1.0f;
        inputIntentions.dashRequested = true;
        simulatePlayer(player, input, tileMap, 0.01f, inputIntentions);
        REQUIRE(dashTriggered);
    }
}

TEST_CASE("A player cannot move or jump into a solid tile", "[Player]")
{
    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
    InputIntentions inputIntentions;

    SECTION("Player cannot move into solid tile")
    {
        TileMap tileMap = aTileMap({{{3, 5}, 1}, {{2, 4}, 1}, {{1, 4}, 1}, {{1, 5}, 1}});
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
        TileMap tileMap = aTileMap({{{ceilingTileX, ceilingTileY}, 1}, {{2, 5}, 1}});
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
    constexpr int LedgeRow = 5;
    constexpr int LedgeLastTile = 6;
    std::vector<std::pair<glm::ivec2, int>> laid;
    for (int x = 0; x <= LedgeLastTile; ++x)
        laid.push_back({glm::ivec2(x, LedgeRow), 1});
    for (int x = 0; x < 20; ++x)
        laid.push_back({glm::ivec2(x, 12), 1});

    TileMap tileMap = aTileMap(laid, 20, 20);

    LevelData levelData;

    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    levelData.tileMapData = tileMap.toTileMapData();
    Level level(
        levelData, theOnlyPalette(aPaletteWithASolidTile()), playerDataWithEveryAbility(), {}, {});

    ScriptedIntentions input;
    Player player(playerDataWithEveryAbility(), input);

    float ledgeRight = static_cast<float>(LedgeLastTile + 1) * 16.0f;
    float ledgeTop = static_cast<float>(LedgeRow) * 16.0f;
    player.setPosition(
        glm::vec2(ledgeRight - player.getPhysicsBody().colliderOffset().x, ledgeTop - 2.0f));

    InputIntentions intentions;
    intentions.direction.x = -1.0f;
    for (int step = 0; step < 300; ++step)
    {
        input.set(intentions);
        player.fixedUpdate(0.01f, level);
        player.postFixedUpdate();
    }

    float colliderTop = player.getPosition().y + player.getPhysicsBody().colliderOffset().y;
    INFO(
        "collider top ended at " << colliderTop << ", the ledge spans " << ledgeTop << " to "
                                 << ledgeTop + 16.0f);
    REQUIRE(colliderTop > ledgeTop + 16.0f);
}

TEST_CASE("A player can climb a wall and get onto the ledge", "[Player][Mantle]")
{
    std::vector<std::pair<glm::ivec2, int>> laid;
    for (int x = 0; x < 12; ++x)
        laid.push_back({glm::ivec2(x, 8), 1});
    for (int x = 5; x < 8; ++x)
        for (int y = 5; y < 8; ++y)
            laid.push_back({glm::ivec2(x, y), 1});

    TileMap tileMap = aTileMap(laid, 12, 10);

    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
    const ActorMotionState &state = player.getMotion().getState();

    player.setPosition(glm::vec2(4 * 16.0f + 4.0f, 8 * 16.0f - 16.0f));

    InputIntentions holdingTheWallAndPressingUp;
    holdingTheWallAndPressingUp.climbRequested = true;
    holdingTheWallAndPressingUp.direction = glm::vec2(1.0f, -1.0f);
    simulatePlayer(player, input, tileMap, 0.8f, holdingTheWallAndPressingUp);
    simulatePlayer(player, input, tileMap, 0.5f);

    REQUIRE(state.contacts.onGround);
    REQUIRE(player.getPhysicsBody().aabb().bottomCenter().y == Approx(5 * 16.0f));
}

TEST_CASE("A player cannot hang on a wall it cannot grip", "[Player][Grip]")
{
    TilePaletteData palette = aPaletteWithASolidTile();
    TileData ungrippable;
    ungrippable.solid = true;
    ungrippable.grippable = false;
    palette.tiles[2] = ungrippable;

    std::vector<std::pair<glm::ivec2, int>> laid;
    for (int y = 0; y < 8; ++y)
        laid.push_back({glm::ivec2(5, y), 2});

    TileMap tileMap = aTileMap(laid, 10, 10, 16, palette);

    ScriptedIntentions input;
    Player player = aPlayerWithEveryAbility(input);
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

TEST_CASE("A ceiling bump is heard once, whichever step of the frame it lands in", "[Player]")
{
    std::vector<std::pair<glm::ivec2, int>> laid;
    for (int x = 0; x < 20; ++x)
    {
        laid.push_back({glm::ivec2(x, 10), 1});
        laid.push_back({glm::ivec2(x, 6), 1});
    }

    TileMap tileMap = aTileMap(laid, 20, 20);

    LevelData levelData;

    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    levelData.tileMapData = tileMap.toTileMapData();
    Level level(
        levelData, theOnlyPalette(aPaletteWithASolidTile()), playerDataWithEveryAbility(), {}, {});

    ScriptedIntentions input;
    Player player(playerDataWithEveryAbility(), input);
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
                player.postFixedUpdate();
                if (player.getMotion().getState().contacts.hitCeiling)
                    ++stepsTouchingCeiling;
            });

        const ActorContactState &contacts = player.getMotion().getState().contacts;
        if (contacts.hitCeiling)
            ++framesEndingWithHitCeiling;
        if (contacts.bumpedCeiling)
            ++framesEndingWithBumpedCeiling;
    }

    REQUIRE(stepsTouchingCeiling == 1);
    REQUIRE(framesEndingWithHitCeiling <= 1);
    REQUIRE(framesEndingWithBumpedCeiling == 1);
}

TEST_CASE("A player has no state name and stands on no node", "[Player]")
{
    Player player = aPlayerWithEveryAbility();

    REQUIRE(player.getStateName().empty());
    REQUIRE_FALSE(player.getCurrentNodeId());
    REQUIRE_FALSE(player.getTargetNodeId());
}
