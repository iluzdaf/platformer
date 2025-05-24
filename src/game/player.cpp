#include "game/player.hpp"
#include <cassert>
#include "game/tile_type.hpp"

Player::Player(glm::vec2 startPos) : position(startPos)
{
    assert(size.x > 0);
    assert(size.y > 0);

    idleAnim = SpriteAnimation({30}, 1.0f, size.x, size.y, 96);
    walkAnim = SpriteAnimation({34, 26, 35}, 0.1f, size.x, size.y, 96);
    currentAnim = &idleAnim;
}

void Player::update(float deltaTime, const TileMap &tileMap)
{
    velocity.y += gravity * deltaTime;
    glm::vec2 nextPosition = position + velocity * deltaTime;

    resolveVerticalCollision(nextPosition.y, velocity.y, tileMap);
    resolveHorizontalCollision(nextPosition.x, velocity.x, tileMap, nextPosition.y);

    bool isWalking = std::abs(velocity.x) > 0.01f;
    PlayerAnimationState newState = isWalking ? PlayerAnimationState::Walk : PlayerAnimationState::Idle;
    if (newState != animState)
    {
        animState = newState;
        currentAnim = (animState == PlayerAnimationState::Walk) ? &walkAnim : &idleAnim;
    }
    currentAnim->update(deltaTime);

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
    facingLeft = true;
}

void Player::moveRight()
{
    velocity.x = moveSpeed;
    facingLeft = false;
}

glm::vec2 Player::getPosition() const
{
    return position;
}

glm::vec2 Player::getVelocity() const
{
    return velocity;
}

void Player::resolveVerticalCollision(float &nextY, float &velY, const TileMap &tileMap)
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

void Player::resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, float nextY)
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

const SpriteAnimation &Player::getCurrentAnimation() const
{
    assert(currentAnim);
    return *currentAnim;
}

PlayerAnimationState Player::getAnimationState() const
{
    return animState;
}

bool Player::isFacingLeft() const
{
    return facingLeft;
}