#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player.hpp"
#include "game/fixed_time_step.hpp"
using Catch::Approx;

TEST_CASE("Player starts with correct position and zero velocity", "[player]")
{
    glm::vec2 startingPosition(100.0f, 200.0f);
    Player player(startingPosition);

    REQUIRE(player.getPosition() == startingPosition);
    REQUIRE(player.getVelocity() == glm::vec2(0.0f, 0.0f));
}

TEST_CASE("Player falls under gravity", "[player]")
{
    Player player(glm::vec2(0.0f, 0));
    TileMap tileMap(1, Player::gravity);

    player.update(1.0f, tileMap);

    glm::vec2 vel = player.getVelocity();
    glm::vec2 pos = player.getPosition();

    REQUIRE(vel.y == Approx(Player::gravity));
    REQUIRE(pos.y == Approx(Player::gravity));
}

TEST_CASE("Player jumps upward", "[player]")
{
    Player player(glm::vec2(0.0f, 300.0f));

    player.jump();

    REQUIRE(player.getVelocity().y == Player::jumpVelocity);
}

TEST_CASE("Player moves left and right", "[player]")
{
    Player player(glm::vec2(0.0f, 0.0f));

    player.moveLeft();
    REQUIRE(player.getVelocity().x == -Player::moveSpeed);

    player.moveRight();
    REQUIRE(player.getVelocity().x == Player::moveSpeed);
}

TEST_CASE("Player lands on solid tile", "[player]")
{
    TileMap tileMap(1, Player::gravity);
    tileMap.setTiles({{1, TileKind::Solid}});
    tileMap.setTileIndex(0, 5, 1);

    Player player(glm::vec2(0.0f, 0.0f));
    FixedTimeStep timestepper(0.01f);
    timestepper.run(1.0f, [&](float dt)
                    { player.update(dt, tileMap); });

    float expectedY = 5 * tileMap.getTileSize() - player.size.y;
    REQUIRE(player.getPosition().y == Approx(expectedY));
    REQUIRE(player.getVelocity().y == Approx(0.0f));
}

TEST_CASE("Player cannot move into solid wall", "[player]")
{
    TileMap tileMap(10, 10);
    tileMap.setTiles({{1, TileKind::Solid}});
    tileMap.setTileIndex(3, 5, 1);
    tileMap.setTileIndex(2, 4, 1);

    Player player(glm::vec2(32.0f, 80.0f));
    player.moveRight();
    FixedTimeStep timestep(0.01f);
    timestep.run(0.1f, [&](float dt)
                 { player.update(dt, tileMap); });

    REQUIRE(player.getVelocity().x == Approx(0.0f));
    REQUIRE(player.getPosition().x <= Approx(3 * tileMap.getTileSize() - player.size.x));
}

TEST_CASE("Player cannot jump through solid ceiling", "[player]")
{
    TileMap map(10, 10);
    map.setTiles({{1, TileKind::Solid}});

    int ceilingTileX = 2;
    int ceilingTileY = 2;
    map.setTileIndex(ceilingTileX, ceilingTileY, 1);

    map.setTileIndex(2, 5, 1);

    glm::vec2 playerStartPos(32.0f, 64.0f);
    Player player(playerStartPos);

    player.jump();

    FixedTimeStep stepper(0.01f);
    stepper.run(1.0f, [&](float dt)
                { player.update(dt, map); });

    float playerTopY = player.getPosition().y;
    float ceilingBottomY = (ceilingTileY + 1) * 16.0f;

    REQUIRE(playerTopY >= Approx(ceilingBottomY).margin(0.1f));
    REQUIRE(player.getVelocity().y == Approx(0.0f));
}

TEST_CASE("Player uses correct animation state", "[PlayerAnimation]")
{
    TileMap tileMap(10, 10, 16);
    Player player({0, 0});

    SECTION("Player is idle by default")
    {
        player.update(0.1f, tileMap);
        REQUIRE(player.getAnimationState() == PlayerAnimationState::Idle);
    }

    SECTION("Player walking triggers walk animation")
    {
        player.moveRight();
        player.update(0.1f, tileMap);
        REQUIRE(player.getAnimationState() == PlayerAnimationState::Walk);
    }

    SECTION("Animation frame advances over time")
    {
        player.moveRight();
        player.update(0.1f, tileMap);
        glm::vec2 uv1 = player.getCurrentAnimation().getUVStart();
        player.moveRight();
        player.update(0.1f, tileMap);
        glm::vec2 uv2 = player.getCurrentAnimation().getUVStart();

        REQUIRE(uv1.x != Approx(uv2.x));
    }
}

TEST_CASE("Player sets facingLeft flag", "[Player]")
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