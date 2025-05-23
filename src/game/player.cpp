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

    const float playerFeetX = nextPosition.x + size.x / 2.0f;
    const float playerFeetY = nextPosition.y + size.y;
    const int tileSize = tileMap.getTileSize();
    const int tileX = static_cast<int>(playerFeetX) / tileSize;
    const int tileY = static_cast<int>(playerFeetY) / tileSize;
    const int tileIndex = tileMap.getTile(tileX, tileY);
    const TileType &tileType = tileMap.getTileType(tileIndex);
    if (tileY < tileMap.getHeight() && tileType.isSolid())
    {
        velocity.y = 0.0f;
        nextPosition.y = tileY * tileSize - size.y;
    }

    if (velocity.x != 0.0f)
    {
        float playerSideX = (velocity.x > 0)
                                ? nextPosition.x + size.x
                                : nextPosition.x;

        float playerFeetY = nextPosition.y + size.y - 1;
        const int tileX = static_cast<int>(playerSideX) / tileSize;
        const int tileY = static_cast<int>(playerFeetY) / tileSize;
        const int tileIndex = tileMap.getTile(tileX, tileY);
        const TileType &tileType = tileMap.getTileType(tileIndex);

        if (tileType.isSolid())
        {
            nextPosition.x = (velocity.x > 0)
                                 ? tileX * tileSize - size.x
                                 : (tileX + 1) * tileSize;
            velocity.x = 0.0f;
        }
    }

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