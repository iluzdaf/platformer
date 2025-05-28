#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player.hpp"
#include "game/fixed_time_step.hpp"
#include "game/physics_data.hpp"
using Catch::Approx;

// player starts in solid tile
// player doesn't drift when moving left or right, also it's x velocity is reset every frame. Does this work with FixedTimeStep updates or do I need a PostUpdate to clean up?
// what if tile size and player size is different?
// what if player has super big move speed, gravity etc?

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

Player setupPlayer(glm::vec2 startPosition = glm::vec2(0, 0), float gravity = 980.0f)
{
    PlayerData playerData;
    playerData.startPosition = startPosition;
    playerData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
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

TEST_CASE("Player falls under gravity", "[Player]")
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

TEST_CASE("Player jumps upward", "[Player]")
{
    Player player = setupPlayer(glm::vec2(0, 300));
    player.jump();
    REQUIRE(player.getVelocity().y == player.getJumpSpeed());
}

TEST_CASE("Player moves left and right", "[Player]")
{
    Player player = setupPlayer();
    player.moveLeft();
    REQUIRE(player.getVelocity().x == -player.getMoveSpeed());
    player.moveRight();
    REQUIRE(player.getVelocity().x == player.getMoveSpeed());
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

        SECTION("Moving right")
        {
            player.moveRight();
            simulatePlayer(player, tileMap, 0.1f);
            REQUIRE(player.getVelocity().x == Approx(0));
            REQUIRE(player.getPosition().x <= Approx(3 * tileMap.getTileSize()));
        }

        SECTION("Moving left")
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
        REQUIRE(player.isFacingLeft() == false);
    }

    SECTION("Moves left and faces left")
    {
        player.moveLeft();
        REQUIRE(player.isFacingLeft() == true);
    }

    SECTION("Moves right and faces right")
    {
        player.moveRight();
        REQUIRE(player.isFacingLeft() == false);
    }
}

TEST_CASE("Player and tilemap bounds", "[Player]")
{
    TileMap tileMap = setupTileMap(3, 3);

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