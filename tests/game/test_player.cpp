#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player.hpp"
#include "game/fixed_time_step.hpp"
using Catch::Approx;

// player starts in solid tile
// player picks up from left and right
// player falls and picks up
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

TileMap setupSimpleTileMap(int width = 10, int height = 10, int tileSize = 16) {
    TileMapData tileMapData;
    tileMapData.size = tileSize;
    tileMapData.width = width;
    tileMapData.height = height;
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    return tileMap;
}

TEST_CASE("Player starts with correct position and zero velocity", "[Player]")
{
    glm::vec2 startingPosition(100.0f, 200.0f);
    Player player(startingPosition);
    REQUIRE(player.getPosition() == startingPosition);
    REQUIRE(player.getVelocity() == glm::vec2(0.0f, 0.0f));
}

TEST_CASE("Player falls under gravity", "[Player]")
{
    Player player(glm::vec2(0, 0));
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 1;
    tileMapData.height = player.gravity / tileMapData.size + 2;
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    simulatePlayer(player, tileMap, 1.0f);

    glm::vec2 vel = player.getVelocity();
    glm::vec2 pos = player.getPosition();
    REQUIRE(vel.y == Approx(Player::gravity));
    REQUIRE(pos.y == Approx(0.5f * Player::gravity).margin(5));
}

TEST_CASE("Player jumps upward", "[Player]")
{
    Player player(glm::vec2(0.0f, 300.0f));
    player.jump();
    REQUIRE(player.getVelocity().y == Player::jumpVelocity);
}

TEST_CASE("Player moves left and right", "[Player]")
{
    Player player(glm::vec2(0.0f, 0.0f));
    player.moveLeft();
    REQUIRE(player.getVelocity().x == -Player::moveSpeed);

    player.moveRight();
    REQUIRE(player.getVelocity().x == Player::moveSpeed);
}

TEST_CASE("Player lands on solid tile", "[Player]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 1;
    tileMapData.height = 6;
    tileMapData.data = {{1, TileData{TileKind::Solid}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    tileMap.setTileIndex(0, 5, 1);
    Player player(glm::vec2(0.0f, 0.0f));
    simulatePlayer(player, tileMap, 1.0f);
    float expectedY = 5 * tileMap.getTileSize() - player.size.y;

    REQUIRE(player.getPosition().y == Approx(expectedY));
    REQUIRE(player.getVelocity().y == Approx(0.0f));
}

TEST_CASE("Player cannot move into solid wall", "[Player]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 10;
    tileMapData.height = 10;
    tileMapData.data = {{1, TileData{TileKind::Solid}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    tileMap.setTileIndex(3, 5, 1);
    tileMap.setTileIndex(2, 4, 1);
    tileMap.setTileIndex(1, 5, 1);
    Player player(glm::vec2(32.0f, 80.0f));

    player.moveRight();
    simulatePlayer(player, tileMap, 1.0f);
    REQUIRE(player.getVelocity().x == Approx(0));
    REQUIRE(player.getPosition().x <= Approx(144));

    player.moveLeft();
    simulatePlayer(player, tileMap, 1.0f);
    REQUIRE(player.getVelocity().x == Approx(0));
    REQUIRE(player.getPosition().x <= Approx(0));
}

TEST_CASE("Player cannot jump through solid ceiling", "[Player]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 10;
    tileMapData.height = 10;
    tileMapData.data = {{1, {TileKind::Solid}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    int ceilingTileX = 2;
    int ceilingTileY = 2;
    tileMap.setTileIndex(ceilingTileX, ceilingTileY, 1);
    tileMap.setTileIndex(2, 5, 1);
    Player player({32, 64});
    player.jump();
    simulatePlayer(player, tileMap, 1.0f);
    float playerTopY = player.getPosition().y;
    float ceilingBottomY = (ceilingTileY + 1) * 16.0f;

    REQUIRE(playerTopY >= Approx(ceilingBottomY).margin(0.1f));
    REQUIRE(player.getVelocity().y == Approx(0.0f));
}

TEST_CASE("Player uses correct animation state", "[Player]")
{
    TileMap tileMap = setupSimpleTileMap();
    Player player({5, 0});

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
        glm::vec2 uv1 = player.getCurrentAnimation().getUVStart();
        player.moveRight();
        simulatePlayer(player, tileMap, 0.1f);
        glm::vec2 uv2 = player.getCurrentAnimation().getUVStart();

        REQUIRE(uv1.x != Approx(uv2.x));
    }
}

TEST_CASE("Player sets facingLeft flag correctly", "[Player]")
{
    Player player({0.0f, 0.0f});

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

TEST_CASE("Player that spawns outside of the tileMap is clamped inside the tileMap", "[Player]")
{
    TileMap tileMap = setupSimpleTileMap(5, 5);
    Player player(glm::vec2(10.0f, 1000.0f));
    simulatePlayer(player, tileMap, 0.1f);
    glm::vec2 position = player.getPosition();

    REQUIRE(position.x <= tileMap.getWorldWidth());
    REQUIRE(position.y <= tileMap.getWorldHeight());
}

TEST_CASE("Player stays within bounds of tilemap", "[Player]")
{
    TileMap tileMap = setupSimpleTileMap(3, 3);
    Player player(glm::vec2(16, 32));

    simulatePlayer(player, tileMap, 1.0f);
    glm::vec2 position = player.getPosition();
    REQUIRE(position.x <= tileMap.getWorldWidth());
    REQUIRE(position.y <= tileMap.getWorldHeight());

    player.moveLeft();
    simulatePlayer(player, tileMap, 1.0f);
    position = player.getPosition();
    REQUIRE(position.x <= tileMap.getWorldWidth());
    REQUIRE(position.y <= tileMap.getWorldHeight());

    player.moveRight();
    simulatePlayer(player, tileMap, 1.0f);
    position = player.getPosition();
    REQUIRE(position.x <= tileMap.getWorldWidth());
    REQUIRE(position.y <= tileMap.getWorldHeight());
}

TEST_CASE("Player can jump after being clamped to bottom of tilemap", "[Player]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 5;
    tileMapData.height = 5;
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    Player player(glm::vec2(10.0f, 1000.0f));
    simulatePlayer(player, tileMap, 0.1f);
    player.jump();
    simulatePlayer(player, tileMap, 0.1f);

    REQUIRE(player.getVelocity().y < 0.0f);
}

TEST_CASE("Player collects pickup and tile is replaced", "[Player]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 3;
    tileMapData.height = 3;
    tileMapData.data = {{0, {TileKind::Empty}},
                        {5, {TileKind::Pickup, std::nullopt, 0}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    tileMap.setTileIndex(1, 1, 5);
    Player player(glm::vec2(16.0f, 16.0f));
    simulatePlayer(player, tileMap, 0.1f);
    int tileIndexAfter = tileMap.getTileIndex(1, 1);

    REQUIRE(tileIndexAfter == 0);
}

TEST_CASE("Player does not do anything to a non-pickup tile", "[Player]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 3;
    tileMapData.height = 3;
    tileMapData.data = {{0, {TileKind::Empty}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    tileMap.setTileIndex(1, 1, 0);
    Player player(glm::vec2(16.0f, 16.0f));
    simulatePlayer(player, tileMap, 0.1f);
    int tileIndexAfter = tileMap.getTileIndex(1, 1);

    REQUIRE(tileIndexAfter == 0);
}