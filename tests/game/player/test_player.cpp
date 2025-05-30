#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "game/player/player.hpp"
#include "game/player/player_data.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_map_data.hpp"
#include "physics/fixed_time_step.hpp"
#include "physics/physics_data.hpp"
using Catch::Approx;

void simulatePlayer(Player &player, TileMap &tileMap, float totalTime, float step = 0.01f)
{
    FixedTimeStep timestepper(step);
    timestepper.run(totalTime, [&](float dt)
                    { player.fixedUpdate(dt, tileMap); });
    player.update(totalTime, tileMap);
}

TileMap setupTileMap(int width = 10, int height = 10, int tileSize = 16, const std::unordered_map<int, TileData> &tileDataMap = {})
{
    TileMapData tileMapData;
    tileMapData.size = tileSize;
    tileMapData.width = width;
    tileMapData.height = height;
    tileMapData.tileData = tileDataMap;
    return TileMap(tileMapData);
}

Player setupPlayer(glm::vec2 startPosition = glm::vec2(0, 0), float gravity = 980.0f, float moveSpeed = 160.0f)
{
    PlayerData playerData;
    playerData.startPosition = startPosition;
    playerData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
    playerData.moveSpeed = moveSpeed;
    PhysicsData physicsData;
    physicsData.gravity = gravity;
    return Player(playerData, physicsData);
}

TEST_CASE("Player starts with correct position and zero velocity", "[Player]")
{
    glm::vec2 startPosition(100, 200);
    Player player = setupPlayer(startPosition);
    REQUIRE(player.getPosition() == startPosition);
    REQUIRE(player.getVelocity() == glm::vec2(0, 0));
}

TEST_CASE("Player and gravity", "[Player]")
{
    SECTION("Player falls under normal gravity")
    {
        float gravity = 980.0f;
        Player player = setupPlayer(glm::vec2(0, 0), gravity);
        TileMap tileMap = setupTileMap(1, gravity + 2);
        simulatePlayer(player, tileMap, 1.0f);
        glm::vec2 vel = player.getVelocity();
        glm::vec2 pos = player.getPosition();
        REQUIRE(vel.y == Approx(gravity));
        REQUIRE(pos.y == Approx(0.5f * gravity).margin(5));
    }
}

TEST_CASE("Player can multi-jump", "[Player]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, {{1, TileData{TileKind::Solid}}});
    tileMap.setTileIndex(1, 9, 1);
    Player player = setupPlayer(glm::vec2(16, 128));
    simulatePlayer(player, tileMap, 0.1f);

    SECTION("Player multi-jump works correctly")
    {
        auto jumpAbility = player.getJumpAbility();
        REQUIRE(jumpAbility != nullptr);
        int maxJumpCount = jumpAbility->getMaxJumpCount();
        int jumpTries = maxJumpCount + 1;

        std::vector<float> jumpVelocities;
        for (int i = 0; i < jumpTries; ++i)
        {
            player.jump();
            jumpVelocities.push_back(player.getVelocity().y);
            simulatePlayer(player, tileMap, 0.1f);
        }

        REQUIRE(jumpVelocities[0] == jumpAbility->getJumpSpeed());

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
        auto jumpAbility = player.getJumpAbility();
        REQUIRE(jumpAbility != nullptr);
        for (int i = 0; i < jumpAbility->getMaxJumpCount(); ++i)
        {
            player.jump();
            simulatePlayer(player, tileMap, 0.1f);
        }

        simulatePlayer(player, tileMap, 1.0f);

        player.jump();
        REQUIRE(player.getVelocity().y == jumpAbility->getJumpSpeed());
    }
}

TEST_CASE("Player can move", "[Player]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, {{1, TileData{TileKind::Solid}}});
    for (int i = 0; i < 10; ++i)
    {
        tileMap.setTileIndex(i, 9, 1);
    }
    Player player = setupPlayer(glm::vec2(80, 128));
    simulatePlayer(player, tileMap, 0.1f);

    SECTION("Player can move left and not drift")
    {
        player.moveLeft();
        REQUIRE(player.getVelocity().x == -player.getMoveSpeed());
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getVelocity().x == 0);
    }

    SECTION("Player can move right and not drift")
    {
        player.moveRight();
        REQUIRE(player.getVelocity().x == player.getMoveSpeed());
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getVelocity().x == 0);
    }
}

TEST_CASE("Player and solid tiles", "[Player]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, {{1, TileData{TileKind::Solid}}});

    SECTION("Player lands on solid tile")
    {
        tileMap.setTileIndex(0, 5, 1);
        Player player = setupPlayer();
        simulatePlayer(player, tileMap, 1.0f);
        float expectedY = 4 * tileMap.getTileSize();
        REQUIRE(player.getPosition().y == Approx(expectedY));
        REQUIRE(player.getVelocity().y == Approx(0.0f));
    }

    SECTION("Player cannot move into solid tile")
    {
        tileMap.setTileIndex(3, 5, 1);
        tileMap.setTileIndex(2, 4, 1);
        tileMap.setTileIndex(1, 4, 1);
        Player player = setupPlayer(glm::vec2(32, 80));

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
        tileMap.setTileIndex(ceilingTileX, ceilingTileY, 1);
        tileMap.setTileIndex(2, 5, 1);
        Player player = setupPlayer(glm::vec2(2, 4));
        player.jump();
        simulatePlayer(player, tileMap, 1.0f);
        float playerTopY = player.getPosition().y;
        float ceilingBottomY = (ceilingTileY + 1);
        REQUIRE(playerTopY >= Approx(ceilingBottomY).margin(0.1f));
        REQUIRE(player.getVelocity().y == Approx(0.0f));
    }
}

TEST_CASE("Player uses correct animation state", "[Player]")
{
    TileMap tileMap = setupTileMap();
    Player player = setupPlayer(glm::vec2(80, 144));

    SECTION("Player is idle by default")
    {
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getAnimationState() == PlayerAnimationState::Idle);
    }

    SECTION("Player walking triggers walk animation")
    {
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getAnimationState() == PlayerAnimationState::Walk);
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getAnimationState() == PlayerAnimationState::Idle);
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(player.getAnimationState() == PlayerAnimationState::Walk);
    }

    SECTION("Animation frame advances over time")
    {
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        glm::vec2 uvBefore = player.getCurrentAnimation().getUVStart();
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        glm::vec2 uvAfter = player.getCurrentAnimation().getUVStart();
        REQUIRE(uvBefore.x != Approx(uvAfter.x));
    }
}

TEST_CASE("Player sets facingLeft flag correctly", "[Player]")
{
    Player player = setupPlayer();

    SECTION("Starts facing right")
    {
        REQUIRE(player.facingLeft() == false);
    }

    SECTION("Moves left and faces left")
    {
        player.moveLeft();
        REQUIRE(player.facingLeft() == true);
    }

    SECTION("Moves right and faces right")
    {
        player.moveRight();
        REQUIRE(player.facingLeft() == false);
    }
}

TEST_CASE("Player and tilemap bounds", "[Player]")
{
    TileMap tileMap = setupTileMap(3, 3, 16, {{1, TileData{TileKind::Solid}}});

    SECTION("Player that spawns outside of the tileMap is clamped inside the tileMap")
    {
        Player player = setupPlayer(glm::vec2(10.0f, 1000.0f));
        simulatePlayer(player, tileMap, 0.1f);
        glm::vec2 position = player.getPosition();
        REQUIRE(position.x <= tileMap.getWorldWidth());
        REQUIRE(position.y <= tileMap.getWorldHeight());

        SECTION("Player can jump after being clamped")
        {
            player.jump();
            simulatePlayer(player, tileMap, 0.1f);
            REQUIRE(player.getVelocity().y < 0.0f);
        }
    }

    SECTION("Player stays within bounds")
    {
        Player player = setupPlayer(glm::vec2(1, 2));

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

    SECTION("Throws if player is clamped into a solid tile")
    {
        tileMap.setTileIndex(0, 0, 1);
        tileMap.setTileIndex(0, 1, 1);
        tileMap.setTileIndex(0, 2, 1);
        tileMap.setTileIndex(1, 2, 1);
        tileMap.setTileIndex(2, 2, 1);
        tileMap.setTileIndex(2, 1, 1);
        tileMap.setTileIndex(2, 0, 1);
        tileMap.setTileIndex(1, 0, 1);
        Player player = setupPlayer(glm::vec2(10.0f, 1000.0f));
        REQUIRE_THROWS_WITH(simulatePlayer(player, tileMap, 0.1f), "Trying to clamp player into a solid tile");
    }
}

TEST_CASE("Player and pickups", "[Player]")
{
    TileMap tileMap = setupTileMap(3, 3, 16, {{0, {TileKind::Empty}}, {5, {TileKind::Pickup, std::nullopt, 0}}});

    SECTION("Player collects pickup and tile is replaced")
    {
        tileMap.setTileIndex(1, 2, 5);

        SECTION("Player spawns on a pickup")
        {
            Player player = setupPlayer(glm::vec2(16, 32));
            simulatePlayer(player, tileMap, 0.1f);
            REQUIRE(tileMap.getTileIndex(1, 2) == 0);
        }

        SECTION("Player falls on a pickup")
        {
            Player player = setupPlayer(glm::vec2(16, 0));
            simulatePlayer(player, tileMap, 1.0f);
            REQUIRE(tileMap.getTileIndex(1, 2) == 0);
        }

        SECTION("Player moves right on a pickup")
        {
            Player player = setupPlayer(glm::vec2(0, 32));
            player.moveRight();
            simulatePlayer(player, tileMap, 1.0f);
            REQUIRE(tileMap.getTileIndex(1, 2) == 0);
        }

        SECTION("Player moves left on a pickup")
        {
            Player player = setupPlayer(glm::vec2(32, 32));
            player.moveLeft();
            simulatePlayer(player, tileMap, 1.0f);
            REQUIRE(tileMap.getTileIndex(1, 2) == 0);
        }
    }
}

TEST_CASE("Player and empty or invalid tiles", "[Player]")
{
    TileMap tileMap = setupTileMap(3, 3, 16, {{0, {TileKind::Empty}}});

    SECTION("Player doesn't do anything to empty tiles")
    {
        tileMap.setTileIndex(1, 1, 0);
        Player player = setupPlayer(glm::vec2(16.0f, 16.0f));
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(tileMap.getTileIndex(1, 1) == 0);
    }

    SECTION("Player doesn't do anything to invalid tiles")
    {
        Player player = setupPlayer(glm::vec2(16.0f, 16.0f));
        simulatePlayer(player, tileMap, 0.1f);
        REQUIRE(tileMap.getTileIndex(1, 1) == -1);
    }
}

TEST_CASE("Player can dash", "[Player]")
{
    Player player = setupPlayer(glm::vec2(80, 144));
    TileMap tileMap = setupTileMap();
    simulatePlayer(player, tileMap, 0.01f);

    SECTION("Player dashes from stationary and does not drift")
    {
        float initialX = player.getPosition().x;
        player.dash();
        REQUIRE(player.dashing());
        simulatePlayer(player, tileMap, player.getDashDuration());
        REQUIRE(player.getPosition().x > initialX);
        REQUIRE_FALSE(player.dashing());
        REQUIRE(player.getVelocity().x == Approx(0));
    }

    SECTION("Player cannot jump while dashing")
    {
        player.dash();
        simulatePlayer(player, tileMap, 0.01f);
        float initialVelY = player.getVelocity().y;
        player.jump();
        simulatePlayer(player, tileMap, 0.01f);
        REQUIRE(player.getVelocity().y == Approx(initialVelY));
    }

    SECTION("Player cannot move while dashing")
    {
        player.dash();
        float totalTime = player.getDashDuration() - 0.1f;
        FixedTimeStep timestepper(0.01f);
        timestepper.run(totalTime, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        float initialVelX = player.getVelocity().x;
        player.update(totalTime, tileMap);
        player.moveLeft();
        timestepper.run(0.1f, [&](float dt)
                        { player.fixedUpdate(dt, tileMap); });
        REQUIRE(player.getVelocity().x == Approx(initialVelX));
        player.update(0.1f, tileMap);
    }

    SECTION("Player can jump and dash")
    {
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        float initialX = player.getPosition().x;
        player.dash();
        REQUIRE(player.dashing());
        simulatePlayer(player, tileMap, player.getDashDuration());
        REQUIRE(player.getPosition().x > initialX);
        REQUIRE_FALSE(player.dashing());
    }

    SECTION("Gravity is not applied while dashing")
    {
        player.jump();
        simulatePlayer(player, tileMap, 0.1f);
        float initialY = player.getPosition().y;
        player.dash();
        simulatePlayer(player, tileMap, player.getDashDuration());
        REQUIRE(player.getPosition().y == Approx(initialY));
    }

    SECTION("Player can move right and dash")
    {
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        float initialX = player.getPosition().x;
        player.dash();
        REQUIRE(player.dashing());
        simulatePlayer(player, tileMap, player.getDashDuration());
        REQUIRE(player.getPosition().x > initialX);
        REQUIRE_FALSE(player.dashing());
    }

    SECTION("Player can move left and dash")
    {
        player.moveLeft();
        simulatePlayer(player, tileMap, 0.1f);
        float initialX = player.getPosition().x;
        player.dash();
        REQUIRE(player.dashing());
        simulatePlayer(player, tileMap, player.getDashDuration());
        REQUIRE(player.getPosition().x < initialX);
        REQUIRE_FALSE(player.dashing());
    }
}