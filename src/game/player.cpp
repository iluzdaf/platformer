#include <cassert>
#include <algorithm>
#include "game/player.hpp"
#include "game/tile.hpp"

Player::Player(const PlayerData &playerData, const PhysicsData &physicsData) : position(playerData.startPosition),
                                                                               moveSpeed(playerData.moveSpeed),
                                                                               jumpSpeed(playerData.jumpSpeed),
                                                                               size(playerData.size),
                                                                               idleAnim(SpriteAnimation(playerData.idleSpriteAnimationData)),
                                                                               walkAnim(SpriteAnimation(playerData.walkSpriteAnimationData))
{
    currentAnim = &idleAnim;
    gravity = physicsData.gravity;
}

void Player::fixedUpdate(float deltaTime, TileMap &tileMap)
{
    velocity.y += gravity * deltaTime;
    glm::vec2 nextPosition = position + velocity * deltaTime;
    resolveVerticalCollision(nextPosition.y, velocity.y, tileMap);
    resolveHorizontalCollision(nextPosition.x, velocity.x, tileMap, nextPosition.y);
    handlePickup(tileMap);
    position = nextPosition;
}

void Player::update(float deltaTime, TileMap &tileMap)
{
    clampToTileMapBounds(tileMap);
    updateAnimation(deltaTime);
    velocity.x = 0.0f;
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
        velocity.y = jumpSpeed;
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
    
    float centerX = position.x + size.x / 2.0f;
    float verticalEdgeY = (velY > 0.0f) ? nextY + size.y : nextY;
    bool collidesWithSolidTile = tileMap.getTile(glm::vec2(centerX, verticalEdgeY)).isSolid();

    if (collidesWithSolidTile)
    {
        int tileSize = tileMap.getTileSize();
        int tileY = static_cast<int>(verticalEdgeY) / tileSize;
        nextY = snapToTileEdge(tileY, tileSize, velY > 0.0f, size.y);
        velY = 0.0f;
    }
}

void Player::resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, float nextY)
{
    if (std::abs(velX) < 0.0001f)
        return;

    float bottomY = nextY + size.y - 1.0f;
    float leadingEdgeX = (velX > 0) ? nextX + size.x : nextX;
    bool collidesWithSolidTile = tileMap.getTile(glm::vec2(leadingEdgeX, bottomY)).isSolid();

    if (collidesWithSolidTile)
    {
        int tileSize = tileMap.getTileSize();
        int tileX = static_cast<int>(leadingEdgeX) / tileSize;
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

    const Tile &tile = tileMap.getTile(position);
    if (tile.isSolid())
    {
        throw std::runtime_error("Trying to clamp player into a solid tile");
    }
}

void Player::handlePickup(TileMap &tileMap)
{
    const Tile &tile = tileMap.getTile(position);
    if (tile.isPickup())
    {
        auto replaceIndexOpt = tile.getPickupReplaceIndex();
        assert(replaceIndexOpt.has_value());
        int tileSize = tileMap.getTileSize();
        int tileX = static_cast<int>((position.x)) / tileSize;
        int tileY = static_cast<int>((position.y)) / tileSize;
        tileMap.setTileIndex(tileX, tileY, replaceIndexOpt.value());
    }
}

glm::vec2 Player::getSize() const
{
    return size;
}

float Player::getMoveSpeed() const
{
    return moveSpeed;
}

float Player::getJumpSpeed() const
{
    return jumpSpeed;
}