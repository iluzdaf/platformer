#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player.hpp"
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
    Player player(glm::vec2(0.0f, -Player::gravity));

    float deltaTime = 1.0f;
    player.update(deltaTime);

    glm::vec2 vel = player.getVelocity();
    glm::vec2 pos = player.getPosition();

    REQUIRE(vel.y == Approx(Player::gravity));
    REQUIRE(pos.y == Approx(0));
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