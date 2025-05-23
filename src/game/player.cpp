#include "game/player.hpp"
#include <cassert>

Player::Player(glm::vec2 startPos, glm::vec2 size) : position(startPos), velocity(0.0f, 0.0f), size(size)
{
    assert(size.x > 0);
    assert(size.y > 0);
}

void Player::update(float deltaTime)
{
    velocity.y += gravity * deltaTime;
    position += velocity * deltaTime;

    if (position.y + size.y > 400.0f)
    {
        position.y = 400.0f - size.y;
        velocity.y = 0.0f;
    }

    velocity.x = 0.0f;
}

void Player::jump()
{
    if (velocity.y == 0.0f)
    {
        velocity.y = jumpVelocity;
    }
}

void Player::moveLeft()
{
    velocity.x = -moveSpeed;
}

void Player::moveRight()
{
    velocity.x = moveSpeed;
}

glm::vec2 Player::getPosition() const
{
    return position;
}

glm::vec2 Player::getSize() const
{
    return size;
}

glm::vec2 Player::getVelocity() const
{
    return velocity;
}