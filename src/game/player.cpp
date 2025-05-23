#include "game/player.hpp"
#include <cassert>
#include "game/tile_type.hpp"

Player::Player(glm::vec2 startPos, glm::vec2 size) : position(startPos), size(size)
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

    int tileSize = tileMap.getTileSize();
    float centerX = position.x + size.x / 2.0f;
    float edgeY = (velY > 0.0f)
                            ? nextY + size.y
                            : nextY;
    int tileX = static_cast<int>(centerX) / tileSize;
    int tileY = static_cast<int>(edgeY) / tileSize;
    int tile = tileMap.getTile(tileX, tileY);
    bool blocked = tileMap.getTileType(tile).isSolid();

    if (blocked)
    {
        nextY = snapToTileEdge(tileY, tileSize, velY > 0.0f, size.y);
        velY = 0.0f;
    }
}

void Player::resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, const glm::vec2 &size, float nextY)
{
    if (std::abs(velX) < 0.0001f)
        return;

    int tileSize = tileMap.getTileSize();
    float footY = nextY + size.y - 1.0f;
    float sideX = (velX > 0) ? nextX + size.x : nextX;

    int tileX = static_cast<int>(sideX) / tileSize;
    int tileY = static_cast<int>(footY) / tileSize;
    int tile = tileMap.getTile(tileX, tileY);
    bool blocked = tileMap.getTileType(tile).isSolid();

    if (blocked)
    {
        nextX = snapToTileEdge(tileX, tileSize, velX > 0.0f, size.x);
        velX = 0.0f;
    }
}

inline float Player::snapToTileEdge(int tile, int tileSize, bool positive, float entitySize)
{
    return positive
               ? tile * tileSize - entitySize
               : (tile + 1) * tileSize;
}