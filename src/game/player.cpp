#include "game/player.hpp"
#include <cassert>
#include "game/tile_type.hpp"

Player::Player(glm::vec2 startPos, glm::vec2 size) : position(startPos), velocity(0.0f, 0.0f), size(size)
{
    assert(size.x > 0);
    assert(size.y > 0);
}

void Player::update(float deltaTime, const TileMap &tileMap)
{
    velocity.y += gravity * deltaTime;
    glm::vec2 nextPosition = position + velocity * deltaTime;

    resolveVerticalCollision(nextPosition.y, velocity.y, tileMap, size);
    resolveHorizontalCollision(nextPosition.x, velocity.x, tileMap, size, nextPosition.y);

    position = nextPosition;
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

void Player::resolveVerticalCollision(float &nextY, float &velY, const TileMap &tileMap, const glm::vec2 &size)
{
    if (std::abs(velY) < 0.0001f)
        return;

    const int tileSize = tileMap.getTileSize();
    const float centerX = position.x + size.x / 2.0f;
    const float edgeY = (velY > 0.0f)
                            ? nextY + size.y
                            : nextY;

    const int tileX = static_cast<int>(centerX) / tileSize;
    const int tileY = static_cast<int>(edgeY) / tileSize;

    const int tile = tileMap.getTile(tileX, tileY);
    const bool blocked = tileMap.getTileType(tile).isSolid();

    if (blocked)
    {
        nextY = (velY > 0.0f)
                    ? tileY * tileSize - size.y
                    : (tileY + 1) * tileSize;
        velY = 0.0f;
    }
}

void Player::resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, const glm::vec2 &size, float nextY)
{
    if (std::abs(velX) < 0.0001f)
        return;

    const int tileSize = tileMap.getTileSize();
    const float footY = nextY + size.y - 1.0f;
    const float sideX = (velX > 0) ? nextX + size.x : nextX;

    const int tileX = static_cast<int>(sideX) / tileSize;
    const int tileY = static_cast<int>(footY) / tileSize;
    const int tile = tileMap.getTile(tileX, tileY);
    const bool blocked = tileMap.getTileType(tile).isSolid();

    if (blocked)
    {
        nextX = (velX > 0)
                    ? tileX * tileSize - size.x
                    : (tileX + 1) * tileSize;
        velX = 0.0f;
    }
}