#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "physics/fixed_time_step.hpp"
#include "input/input_intentions.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/test_player_utils.hpp"

using Catch::Approx;

void simulatePlayer(
    Player &player,
    ScriptedIntentions &input,
    TileMap &tileMap,
    float totalTime,
    InputIntentions intentions = InputIntentions(),
    float step = 0.01f)
{
    FixedTimeStep timeStepper(step);
    input.set(intentions);
    timeStepper.run(totalTime, [&](float dt)
                    { player.fixedUpdate(dt, tileMap); player.postFixedUpdate(); });
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
        glm::vec2 uvBefore = player.getState().currentAnimationUVStart;
        simulatePlayer(player, input, tileMap, 0.1f, inputIntentions);
        glm::vec2 uvAfter = player.getState().currentAnimationUVStart;
        REQUIRE(uvBefore.x != Approx(uvAfter.x));
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
        player.onFallFromHeight.connect([&]
                                        { fallFromHeightTriggered = true; });
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
        player.onHitCeiling.connect([&]
                                    { hitCeilingTriggered = true; });
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
