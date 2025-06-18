#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "physics/fixed_time_step.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/test_player_utils.hpp"
using Catch::Approx;

void simulatePlayer(Player &player, TileMap &tileMap, float totalTime, float step = 0.01f)
{
    FixedTimeStep timeStepper(step);
    timeStepper.run(totalTime, [&](float dt)
                    { player.fixedUpdate(dt, tileMap); });
    player.update(totalTime, tileMap);
}

TEST_CASE("Player starts with correct position and zero velocity", "[Player]")
{
    Player player = setupPlayer();
    REQUIRE(player.getPosition() == glm::vec2(0, 0));
    REQUIRE(player.getVelocity() == glm::vec2(0, 0));
}

TEST_CASE("Player and gravity", "[Player]")
{
    SECTION("Player falls under normal gravity")
    {
        float gravity = 980.0f;
        Player player = setupPlayer(gravity);
        TileMap tileMap = setupTileMap(1, gravity + 2);
        simulatePlayer(player, tileMap, 1.0f);
        glm::vec2 vel = player.getVelocity();
        glm::vec2 pos = player.getPosition();
        REQUIRE(vel.y == Approx(gravity));
        REQUIRE(pos.y == Approx(0.5f * gravity).margin(5));
    }
}

TEST_CASE("Player and tiles", "[Player]")
{
    TileMap tileMap = setupTileMap();
    Player player = setupPlayer();

    SECTION("Player lands on solid tile")
    {
        tileMap.setTileIndex(glm::ivec2(0, 5), 1);
        simulatePlayer(player, tileMap, 1.0f);
        float expectedY = 4 * tileMap.getTileSize();
        REQUIRE(player.getPosition().y == Approx(expectedY));
        REQUIRE(player.getPlayerState().onGround);
        REQUIRE(player.getVelocity().y == Approx(0.0f).margin(0.01f));
    }

    SECTION("Player walks off a ledge and is no longer onGround")
    {
        tileMap.setTileIndex(glm::ivec2(1, 5), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition(glm::vec2(2 * tileMap.getTileSize(), 4 * tileMap.getTileSize()));
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getPlayerState().onGround);
        player.moveRight();
        simulatePlayer(player, tileMap, 0.2f);
        REQUIRE_FALSE(player.getPlayerState().onGround);
    }

    SECTION("Player cannot move into solid tile")
    {
        tileMap.setTileIndex(glm::ivec2(3, 5), 1);
        tileMap.setTileIndex(glm::ivec2(2, 4), 1);
        tileMap.setTileIndex(glm::ivec2(1, 4), 1);
        player.setPosition(glm::vec2(32, 80));

        SECTION("Moving right into solid tile")
        {
            player.moveRight();
            simulatePlayer(player, tileMap, 0.1f);
            REQUIRE(player.getVelocity().x == Approx(0));
            REQUIRE(player.getPosition().x <= Approx(3 * tileMap.getTileSize()));
        }

        SECTION("Moving left into solid tile")
        {
            player.moveLeft();
            simulatePlayer(player, tileMap, 0.1f);
            REQUIRE(player.getVelocity().x == Approx(0));
            REQUIRE(player.getPosition().x >= Approx(tileMap.getTileSize()));
        }
    }

    SECTION("Player cannot jump through solid tile", "[Player]")
    {
        int ceilingTileX = 2;
        int ceilingTileY = 2;
        tileMap.setTileIndex(glm::ivec2(ceilingTileX, ceilingTileY), 1);
        tileMap.setTileIndex(glm::ivec2(2, 5), 1);
        player.setPosition({32, 64});
        player.jump();
        simulatePlayer(player, tileMap, 1.0f);
        float playerTopY = player.getPosition().y;
        float ceilingBottomY = (ceilingTileY + 1);
        REQUIRE(playerTopY >= Approx(ceilingBottomY).margin(0.1f));
        REQUIRE(player.getVelocity().y >= Approx(0.0f));
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
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Walk);
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Idle);
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getPlayerState().currentAnimationState == PlayerAnimationState::Walk);
    }

    SECTION("Animation frame advances over time")
    {
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        glm::vec2 uvBefore = player.getPlayerState().currentAnimationUVStart;
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
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
        REQUIRE(player.facingLeft() == false);
    }

    SECTION("Moves left and faces left")
    {
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.facingLeft() == true);
    }

    SECTION("Moves right and faces right")
    {
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.facingLeft() == false);
    }
}

TEST_CASE("Player and tilemap bounds", "[Player]")
{
    SECTION("Player that spawns outside of the tileMap is clamped inside the tileMap")
    {
        TileMap tileMap = setupTileMap();
        Player player = setupPlayer();
        player.setPosition(glm::vec2(10.0f, 1000.0f));
        simulatePlayer(player, tileMap, 0.1f);
        glm::vec2 position = player.getPosition();
        REQUIRE(position.x <= tileMap.getWorldWidth());
        REQUIRE(position.y <= tileMap.getWorldHeight());

        SECTION("Player can jump after being clamped")
        {
            player.jump();
            simulatePlayer(player, tileMap, 0.1f);
            REQUIRE(player.getVelocity().y < 0.0f);
            player.jump();
            simulatePlayer(player, tileMap, 1.0f);
            player.jump();
            REQUIRE(player.getVelocity().y < 0.0f);
        }
    }

    SECTION("Player stays within bounds")
    {
        TileMap tileMap = setupTileMap();
        Player player = setupPlayer();
        player.setPosition(glm::vec2(16, 32));

        SECTION("While falling")
        {
            simulatePlayer(player, tileMap, 1.0f);
            glm::vec2 position = player.getPosition();
            REQUIRE(position.x <= tileMap.getWorldWidth());
            REQUIRE(position.y <= tileMap.getWorldHeight());
        }

        SECTION("While moving left")
        {
            player.moveLeft();
            simulatePlayer(player, tileMap, 1.0f);
            glm::vec2 position = player.getPosition();
            REQUIRE(position.x <= tileMap.getWorldWidth());
            REQUIRE(position.y <= tileMap.getWorldHeight());
        }

        SECTION("While moving right")
        {
            player.moveRight();
            simulatePlayer(player, tileMap, 1.0f);
            glm::vec2 position = player.getPosition();
            REQUIRE(position.x <= tileMap.getWorldWidth());
            REQUIRE(position.y <= tileMap.getWorldHeight());
        }
    }
}

TEST_CASE("Player sets wall touch flags correctly", "[Player]")
{
    TileMap tileMap = setupTileMap();

    SECTION("Touching right wall")
    {
        for (int y = 0; y < 10; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(6, y), 1);
        }

        Player player = setupPlayer();
        player.setPosition(glm::vec2(5 * 16.0f, 16.0f));
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);

        PlayerState playerState = player.getPlayerState();
        REQUIRE(playerState.touchingRightWall);
        REQUIRE_FALSE(playerState.touchingLeftWall);
    }

    SECTION("Touching left wall")
    {
        for (int y = 0; y < 10; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(3, y), 1);
        }
        Player player = setupPlayer();
        player.setPosition(glm::vec2(4 * 16.0f, 16.0f));
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);

        PlayerState playerState = player.getPlayerState();
        REQUIRE(playerState.touchingLeftWall);
        REQUIRE_FALSE(playerState.touchingRightWall);
    }
}

TEST_CASE("Player movement ability integration", "[Player]")
{
    TileMap tileMap = setupTileMap(20, 10);
    Player player = setupPlayer();

    SECTION("Player can jump, wall slide, and wall jump")
    {
        for (int tileY = 0; tileY < 10; ++tileY)
        {
            tileMap.setTileIndex(glm::ivec2(2, tileY), 1);
            tileMap.setTileIndex(glm::ivec2(7, tileY), 1);
        }

        player.setPosition(glm::vec2(3 * 16, 5 * 16));
        player.moveLeft();
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);
        REQUIRE(player.getPlayerState().wallSliding);

        player.jump();
        FixedTimeStep timeStepper;
        timeStepper.run(0.1f, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        REQUIRE(player.getVelocity().x > 0);
        REQUIRE(player.getVelocity().y < 0);
    }

    SECTION("Player dashes into wall and performs wall jump")
    {
        for (int tileY = 0; tileY < 10; ++tileY)
            tileMap.setTileIndex(glm::ivec2(6, tileY), 1);

        player.setPosition(glm::vec2(4 * 16, 2 * 16));
        player.moveRight();
        player.dash();
        simulatePlayer(player, tileMap, 0.19f);
        PlayerState playerState = player.getPlayerState();
        REQUIRE(playerState.touchingRightWall);
        REQUIRE_FALSE(playerState.dashing);
        REQUIRE(playerState.wallSliding);

        player.jump();
        FixedTimeStep timeStepper;
        timeStepper.run(0.1f, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        REQUIRE(player.getVelocity().x < 0);
    }

    SECTION("Player cannot wall jump unless touching wall")
    {
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);

        glm::vec2 before = player.getVelocity();
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);

        glm::vec2 after = player.getVelocity();
        REQUIRE(after.x == Approx(before.x));
    }

    SECTION("Wall jump does not increment jump count")
    {
        for (int y = 0; y < 10; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(2, y), 1);
            tileMap.setTileIndex(glm::ivec2(5, y), 1);
        }

        player.setPosition(glm::vec2(3 * 16, 5 * 16));
        player.moveLeft();
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);

        int jumpsBeforeWallJump = player.getPlayerState().jumpCount;
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);
        int jumpsAfterWallJump = player.getPlayerState().jumpCount;
        REQUIRE(jumpsAfterWallJump == jumpsBeforeWallJump);

        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        jumpsAfterWallJump = player.getPlayerState().jumpCount;
        REQUIRE(jumpsAfterWallJump == jumpsBeforeWallJump);
    }

    SECTION("Player multi-jump works correctly")
    {
        tileMap.setTileIndex(glm::ivec2(1, 9), 1);
        player.setPosition({16, 8 * 16});
        simulatePlayer(player, tileMap, 0.1f);

        const PlayerState &playerState = player.getPlayerState();
        int jumpTries = playerState.maxJumpCount + 1;

        std::vector<float> jumpVelocities;
        for (int i = 0; i < jumpTries; ++i)
        {
            player.jump();
            jumpVelocities.push_back(player.getVelocity().y);
            simulatePlayer(player, tileMap, 0.1f);
        }

        REQUIRE(jumpVelocities[0] == playerState.jumpSpeed);

        float beforeJumpVelocity = jumpVelocities[0];
        for (int i = 1; i < jumpTries - 1; ++i)
        {
            REQUIRE(jumpVelocities[i] < 0.0f);
            REQUIRE(jumpVelocities[i] <= beforeJumpVelocity);
            beforeJumpVelocity = jumpVelocities[i];
        }

        REQUIRE(jumpVelocities.back() >= Approx(beforeJumpVelocity));
    }

    SECTION("Can multi-jump again after landing")
    {
        tileMap.setTileIndex(glm::ivec2(1, 9), 1);
        player.setPosition({16, 8 * 16});
        simulatePlayer(player, tileMap, 0.1f);

        const PlayerState &playerState = player.getPlayerState();
        for (int i = 0; i < playerState.maxJumpCount; ++i)
        {
            player.jump();
            simulatePlayer(player, tileMap, 0.1f);
        }

        simulatePlayer(player, tileMap, 1.0f);

        player.jump();
        REQUIRE(player.getVelocity().y == playerState.jumpSpeed);
    }

    SECTION("Player can move left and not drift")
    {
        for (int tileX = 0; tileX < 10; ++tileX)
        {
            tileMap.setTileIndex(glm::ivec2(tileX, 9), 1);
        }
        player.setPosition({5 * 16, 8 * 16});
        simulatePlayer(player, tileMap, 0.1f);

        player.moveLeft();
        FixedTimeStep timestepper(0.01f);
        timestepper.run(0.1f, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(player.getVelocity().x == -playerState.moveSpeed);
        player.update(0.1f, tileMap);
        REQUIRE(player.getVelocity().x == 0);
    }

    SECTION("Player can move right and not drift")
    {
        for (int tileX = 0; tileX < 10; ++tileX)
        {
            tileMap.setTileIndex(glm::ivec2(tileX, 9), 1);
        }
        player.setPosition({5 * 16, 8 * 16});
        simulatePlayer(player, tileMap, 0.1f);

        player.moveRight();
        FixedTimeStep timestepper(0.01f);
        timestepper.run(0.1f, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(player.getVelocity().x == playerState.moveSpeed);
        player.update(0.1f, tileMap);
        REQUIRE(player.getVelocity().x == 0);
    }

    SECTION("Player dashes from stationary and does not drift")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        float initialX = player.getPosition().x;
        player.dash();
        simulatePlayer(player, tileMap, 0.01f);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.dashing);
        simulatePlayer(player, tileMap, playerState.dashDuration - 0.01f);
        REQUIRE(player.getPosition().x > initialX);
        REQUIRE_FALSE(playerState.dashing);
        REQUIRE(player.getVelocity().x == Approx(0));
    }

    SECTION("Player cannot jump while dashing")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        player.dash();
        simulatePlayer(player, tileMap, 0.01f);
        float initialVelY = player.getVelocity().y;
        player.jump();
        simulatePlayer(player, tileMap, 0.01f);
        REQUIRE(player.getVelocity().y == Approx(initialVelY));
    }

    SECTION("Player cannot move while dashing")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        player.dash();
        const PlayerState &playerState = player.getPlayerState();
        float totalTime = playerState.dashDuration - 0.1f;
        FixedTimeStep timestepper(0.01f);
        timestepper.run(totalTime, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        float initialVelocityX = player.getVelocity().x;
        player.update(totalTime, tileMap);
        player.moveLeft();
        timestepper.run(0.1f, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        REQUIRE(player.getVelocity().x == Approx(initialVelocityX));
    }

    SECTION("Player can jump and dash")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        float initialX = player.getPosition().x;
        player.dash();
        simulatePlayer(player, tileMap, 0.01f);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.dashing);
        simulatePlayer(player, tileMap, playerState.dashDuration - 0.01f);
        REQUIRE(player.getPosition().x > initialX);
        REQUIRE_FALSE(playerState.dashing);
    }

    SECTION("Gravity is not applied while dashing")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        float initialY = player.getPosition().y;
        player.dash();
        const PlayerState &playerState = player.getPlayerState();
        simulatePlayer(player, tileMap, playerState.dashDuration);
        REQUIRE(player.getPosition().y == Approx(initialY));
    }

    SECTION("Player can move right and dash")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        float initialX = player.getPosition().x;
        player.dash();
        simulatePlayer(player, tileMap, 0.01f);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.dashing);
        simulatePlayer(player, tileMap, playerState.dashDuration - 0.01f);
        REQUIRE(player.getPosition().x > initialX);
        REQUIRE_FALSE(playerState.dashing);
    }

    SECTION("Player can move left and dash")
    {
        player.setPosition({5 * 16, 9 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);
        float initialX = player.getPosition().x;
        player.dash();
        simulatePlayer(player, tileMap, 0.01f);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.dashing);
        simulatePlayer(player, tileMap, playerState.dashDuration - 0.01f);
        REQUIRE(player.getPosition().x < initialX);
        REQUIRE_FALSE(playerState.dashing);
    }

    SECTION("Player can jump while grounded and touching wall")
    {
        for (int tileY = 0; tileY < 10; ++tileY)
        {
            tileMap.setTileIndex(glm::ivec2(2, tileY), 1);
        }

        player.setPosition(glm::vec2(3 * 16, 9 * 16));
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);
        const PlayerState &playerState = player.getPlayerState();
        REQUIRE(playerState.onGround);
        REQUIRE(playerState.touchingLeftWall);
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getVelocity().y < 0);
    }

    SECTION("Player cannot execute first jump if not on ground")
    {
        player.setPosition(glm::vec2(5 * 16, 5 * 16));
        simulatePlayer(player, tileMap, 0.01f);
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getVelocity().y > 0);
    }

    SECTION("Player jump buffering")
    {
        tileMap.setTileIndex(glm::ivec2(5, 9), 1);
        player.setPosition(glm::vec2(5 * 16, 7 * 16));
        simulatePlayer(player, tileMap, 0.1f);
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);
        REQUIRE(player.getVelocity().y < 0.0f);
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
        player.jump();
        bool hitCeilingTriggered = false;
        player.onHitCeiling.connect([&]
                                    { hitCeilingTriggered = true; });
        simulatePlayer(player, tileMap, 1.0f);
        REQUIRE(hitCeilingTriggered);
    }

    SECTION("onDash")
    {
        player.setPosition({5 * 16, 9 * 16});
        bool dashTriggered = false;
        player.onDash.connect([&]
                              { dashTriggered = true; });
        player.dash();
        simulatePlayer(player, tileMap, 0.2f);
        REQUIRE(dashTriggered);
    }

    SECTION("onDoubleJump")
    {
        player.setPosition({5 * 16, 18 * 16});
        simulatePlayer(player, tileMap, 0.01f);
        bool doubleJumpTriggered = false;
        player.onDoubleJump.connect([&]
                                    { doubleJumpTriggered = true; });
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE_FALSE(doubleJumpTriggered);
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);
        REQUIRE(doubleJumpTriggered);
    }

    SECTION("onWallSliding")
    {
        for (int y = 0; y < 20; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(2, y), 1);
        }
        player.setPosition(glm::vec2(3 * 16, 5 * 16));
        bool wallSlideTriggered = false;
        player.onWallSliding.connect([&]
                                     { wallSlideTriggered = true; });
        player.moveLeft();
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);
        REQUIRE(wallSlideTriggered);
    }

    SECTION("onWallJump")
    {
        for (int y = 0; y < 20; ++y)
        {
            tileMap.setTileIndex(glm::ivec2(2, y), 1);
        }
        player.setPosition(glm::vec2(3 * 16, 5 * 16));
        bool wallJumpTriggered = false;
        player.onWallJump.connect([&]
                                  { wallJumpTriggered = true; });
        player.moveLeft();
        player.jump();
        simulatePlayer(player, tileMap, 0.2f);
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(wallJumpTriggered);
    }
}