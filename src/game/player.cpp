#include "game/player.hpp"
#include "game/tile.hpp"
#include <cassert>
#include <algorithm>

Player::Player(glm::vec2 startPos) : position(startPos)
{
    idleAnim = SpriteAnimation({30}, 1.0f, size.x, size.y, 96);
    walkAnim = SpriteAnimation({34, 26, 35}, 0.1f, size.x, size.y, 96);
    currentAnim = &idleAnim;
}

void Player::update(float deltaTime, TileMap &tileMap)
{
    velocity.y += gravity * deltaTime;
    glm::vec2 nextPosition = position + velocity * deltaTime;

    resolveVerticalCollision(nextPosition.y, velocity.y, tileMap);
    resolveHorizontalCollision(nextPosition.x, velocity.x, tileMap, nextPosition.y);
    updateAnimation(deltaTime);

    position = nextPosition;
    velocity.x = 0.0f;

    clampToTileMapBounds(tileMap);
    handlePickup(tileMap);
}

void Player::updateAnimation(float deltaTime)
{
    bool isWalking = std::abs(velocity.x) > 0.01f;
    PlayerAnimationState newState = isWalking ? PlayerAnimationState::Walk : PlayerAnimationState::Idle;

    if (newState != animState)
    {
        animState = newState;
        currentAnim = (animState == PlayerAnimationState::Walk) ? &walkAnim : &idleAnim;
    }

    currentAnim->update(deltaTime);
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

    const int tileSize = tileMap.getTileSize();
    const float centerX = position.x + size.x / 2.0f;
    const float verticalEdgeY = (velY > 0.0f) ? nextY + size.y : nextY;
    const int tileX = static_cast<int>(centerX) / tileSize;
    const int tileY = static_cast<int>(verticalEdgeY) / tileSize;
    const int tileIndex = tileMap.getTileIndex(tileX, tileY);
    const bool collidesWithSolidTile = isTileSolid(tileMap, tileIndex);

    if (collidesWithSolidTile)
    {
        nextY = snapToTileEdge(tileY, tileSize, velY > 0.0f, size.y);
        velY = 0.0f;
    }
}

void Player::resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, float nextY)
{
    if (std::abs(velX) < 0.0001f)
        return;

    const int tileSize = tileMap.getTileSize();
    const float bottomY = nextY + size.y - 1.0f;
    const float leadingEdgeX = (velX > 0) ? nextX + size.x : nextX;
    const int tileX = static_cast<int>(leadingEdgeX) / tileSize;
    const int tileY = static_cast<int>(bottomY) / tileSize;
    const int tileIndex = tileMap.getTileIndex(tileX, tileY);
    const bool collidesWithSolidTile = isTileSolid(tileMap, tileIndex);

    if (collidesWithSolidTile)
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

bool Player::isTileSolid(const TileMap &tileMap, int tileIndex) const
{
    if (auto tileOpt = tileMap.getTile(tileIndex))
    {
        return tileOpt->get().isSolid();
    }
    return false;
}

void Player::clampToTileMapBounds(const TileMap &tileMap)
{
    const int width = tileMap.getWorldWidth();
    const int height = tileMap.getWorldHeight();

    if (position.x < 0.0f)
    {
        position.x = 0.0f;
        velocity.x = 0.0f;
    }
    else if (position.x + size.x > width)
    {
        position.x = width - size.x;
        velocity.x = 0.0f;
    }

    if (position.y < 0.0f)
    {
        position.y = 0.0f;
        velocity.y = 0.0f;
    }
    else if (position.y + size.y > height)
    {
        position.y = height - size.y;
        velocity.y = 0.0f;
    }
}

void Player::handlePickup(TileMap &tileMap)
{
    int tileX = static_cast<int>((position.x)) / tileMap.getTileSize();
    int tileY = static_cast<int>((position.y)) / tileMap.getTileSize();
    int tileIndex = tileMap.getTileIndex(tileX, tileY);
    auto tileOpt = tileMap.getTile(tileIndex);

    if (tileOpt && tileOpt->get().isPickup())
    {
        auto replaceIndexOpt = tileOpt->get().getPickupReplaceIndex();
        assert(replaceIndexOpt.has_value());
        tileMap.setTileIndex(tileX, tileY, replaceIndexOpt.value());
    }
}