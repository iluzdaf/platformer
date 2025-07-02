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
    TileMap &tileMap,
    float totalTime,
    InputIntentions intentions = InputIntentions(),
    float step = 0.01f)
{
    FixedTimeStep timeStepper(step);
    timeStepper.run(totalTime, [&](float dt)
                    { player.fixedUpdate(dt, tileMap, intentions); });
}

TEST_CASE("Player falls under normal gravity", "[Player]")
{
    Player player = setupPlayer();
    TileMap tileMap = setupTileMap(1, static_cast<int>(980.0f / 16.0f) + 2);
    simulatePlayer(player, tileMap, 1.0f);
    const PlayerState &playerState = player.getPlayerState();
    REQUIRE(playerState.velocity.y == Approx(980));
    REQUIRE(playerState.position.y == Approx(0.5f * 980).margin(5));
}

TEST_CASE("Player sets onGround correcly", "[Player]")
{
    TileMap tileMap = setupTileMap();
    Player player = setupPlayer();

    SECTION("Player lands on solid tile")
    {
        tileMap.setTileIndex(glm::ivec2(0, 5), 1);
        simulatePlayer(player, tileMap, 1.0f);
        float expectedY = 4 * tileMap.getTileSize();
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.position.y == Approx(expectedY));
        REQUIRE(playerState.onGround);
        REQUIRE(playerState.velocity.y == Approx(0.0f).margin(0.01f));
    }

    SECTION("Player walks off a ledge and is no longer onGround")
    {
        tileMap.setTileIndex(glm::ivec2(1, 5), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition({2 * 16, 4 * 16});
        simulatePlayer(player, tileMap, 0.1f);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.onGround);
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, tileMap, 0.2f, inputIntentions);
        REQUIRE_FALSE(playerState.onGround);
    }
}

TEST_CASE("Player uses correct animation state", "[Player]")
{
    TileMap tileMap = setupTileMap();
    Player player = setupPlayer();
    player.setPosition({5 * 16, 9 * 16});

    SECTION("Player is idle by default")
    {
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Idle);
    }

    SECTION("Player walking triggers walk animation")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Walk);
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Idle);
        inputIntentions = InputIntentions();
        inputIntentions.direction.x = -1;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Walk);
    }

    SECTION("Animation frame advances over time")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        glm::vec2 uvBefore = player.getPlayerState().currentAnimationUVStart;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        glm::vec2 uvAfter = player.getPlayerState().currentAnimationUVStart;
        REQUIRE(uvBefore.x != Approx(uvAfter.x));
    }
}

TEST_CASE("Player sets facingLeft flag correctly", "[Player]")
{
    Player player = setupPlayer();
    TileMap tileMap = setupTileMap();

    SECTION("Starts facing right")
    {
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE_FALSE(playerState.facingLeft);
    }

    SECTION("Moves left and faces left")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = -1;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.facingLeft);
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(playerState.facingLeft);
    }

    SECTION("Moves right and faces right")
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE_FALSE(playerState.facingLeft);
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE_FALSE(playerState.facingLeft);
    }
}

TEST_CASE("Player and tilemap bounds", "[Player]")
{
    TileMap tileMap = setupTileMap();
    Player player = setupPlayer();
    const PlayerState &playerState = player.getPlayerState();

    SECTION("Player that spawns outside of the tileMap is clamped inside the tileMap")
    {
        player.setPosition(glm::vec2(10.0f, 1000.0f));
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(playerState.position.x <= tileMap.getWorldWidth());
        REQUIRE(playerState.position.y <= tileMap.getWorldHeight());

        SECTION("Player can jump after being clamped")
        {
            InputIntentions inputIntentions;
            inputIntentions.jumpRequested = true;
            simulatePlayer(player, tileMap, 0.1f, inputIntentions);
            REQUIRE(playerState.velocity.y < 0.0f);
        }
    }

    SECTION("Player stays within bounds")
    {
        player.setPosition(glm::vec2(16, 32));

        SECTION("While falling")
        {
            simulatePlayer(player, tileMap, 10.0f);
            REQUIRE(playerState.position.x <= tileMap.getWorldWidth());
            REQUIRE(playerState.position.y <= tileMap.getWorldHeight());
        }

        SECTION("While moving left")
        {
            InputIntentions inputIntentions;
            inputIntentions.direction.x = -1;
            simulatePlayer(player, tileMap, 10.0f, inputIntentions);
            REQUIRE(playerState.position.x <= tileMap.getWorldWidth());
            REQUIRE(playerState.position.y <= tileMap.getWorldHeight());
        }

        SECTION("While moving right")
        {
            InputIntentions inputIntentions;
            inputIntentions.direction.x = 1;
            simulatePlayer(player, tileMap, 10.0f, inputIntentions);
            REQUIRE(playerState.position.x <= tileMap.getWorldWidth());
            REQUIRE(playerState.position.y <= tileMap.getWorldHeight());
        }
    }
}

TEST_CASE("Player sets wall touch flags correctly", "[Player]")
{
    TileMap tileMap = setupTileMap();
    Player player = setupPlayer();
    const PlayerState &playerState = player.getPlayerState();

    SECTION("Touching right wall")
    {
        for (int y = 0; y < 10; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(6, y), 1);
        }
        player.setPosition(glm::vec2(5 * 16.0f, 16.0f));
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        REQUIRE(playerState.touchingRightWall);
        REQUIRE_FALSE(playerState.touchingLeftWall);
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
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        REQUIRE(playerState.touchingLeftWall);
        REQUIRE_FALSE(playerState.touchingRightWall);
    }
}

TEST_CASE("Player event callbacks are triggered", "[Player]")
{
    TileMap tileMap = setupTileMap(10, 20);
    for (int x = 0; x < 10; ++x)
    {
        tileMap.setTileIndex(glm::ivec2(x, 19), 1);
    }
    Player player = setupPlayer();

    SECTION("onFallFromHeight")
    {
        player.setPosition(glm::vec2(0, 0));
        bool fallFromHeightTriggered = false;
        player.onFallFromHeight.connect([&]
                                        { fallFromHeightTriggered = true; });
        simulatePlayer(player, tileMap, 1.0f);
        REQUIRE(fallFromHeightTriggered);
    }

    SECTION("onHitCeiling")
    {
        tileMap.setTileIndex(glm::ivec2(2, 2), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition({2 * 16, 4 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        bool hitCeilingTriggered = false;
        player.onHitCeiling.connect([&]
                                    { hitCeilingTriggered = true; });
        InputIntentions inputIntentions;
        inputIntentions.jumpRequested = true;
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        REQUIRE(hitCeilingTriggered);
    }
}

TEST_CASE("Player movement ability integration", "[Player]")
{
    TileMap tileMap = setupTileMap(20, 10);
    Player player = setupPlayer();
    InputIntentions inputIntentions;
    const PlayerState &playerState = player.getPlayerState();

    SECTION("Player cannot move into solid tile")
    {
        tileMap.setTileIndex(glm::ivec2(3, 5), 1);
        tileMap.setTileIndex(glm::ivec2(2, 4), 1);
        tileMap.setTileIndex(glm::ivec2(1, 4), 1);
        player.setPosition(glm::vec2(2 * tileMap.getTileSize(), 5 * tileMap.getTileSize()));

        SECTION("Moving right into solid tile")
        {
            inputIntentions.direction.x = 1;
            simulatePlayer(player, tileMap, 0.1f, inputIntentions);
            REQUIRE(playerState.position.x <= Approx(3 * tileMap.getTileSize()));
        }

        SECTION("Moving left into solid tile")
        {
            inputIntentions.direction.x = -1;
            simulatePlayer(player, tileMap, 0.1f, inputIntentions);
            REQUIRE(playerState.position.x >= Approx(1 * tileMap.getTileSize()));
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
        simulatePlayer(player, tileMap, 0.1f, inputIntentions);
        float playerTopY = playerState.position.y;
        float ceilingBottomY = (ceilingTileY + 1);
        REQUIRE(playerTopY >= Approx(ceilingBottomY).margin(0.1f));
    }
}