#include <cassert>
#include <algorithm>
#include "game/player/player.hpp"
#include "game/tile_map/tile.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player_data.hpp"
#include "physics/physics_data.hpp"

Player::Player(const PlayerData &playerData, const PhysicsData &physicsData) : position(playerData.startPosition),
                                                                               moveSpeed(playerData.moveSpeed),
                                                                               size(playerData.size),
                                                                               idleAnim(SpriteAnimation(playerData.idleSpriteAnimationData)),
                                                                               walkAnim(SpriteAnimation(playerData.walkSpriteAnimationData)),
                                                                               dashSpeed(playerData.dashSpeed),
                                                                               dashDuration(playerData.dashDuration),
                                                                               dashCooldown(playerData.dashCooldown)
{
    currentAnim = &idleAnim;
    gravity = physicsData.gravity;
    movementAbilities.emplace_back(std::make_unique<JumpAbility>(playerData.maxJumpCount, playerData.jumpSpeed));
}

void Player::fixedUpdate(float deltaTime, TileMap &tileMap)
{
    for (auto &ability : movementAbilities)
    {
        ability->update(*this, deltaTime);
    }

    if (!canDash())
    {
        dashCooldownLeft -= deltaTime;
    }

    if (dashing())
    {
        dashTimeLeft -= deltaTime;
        if (dashTimeLeft > 0.0f)
        {
            velocity.x = dashSpeed * dashDirection;
        }
    }
    else
    {
        velocity.y += gravity * deltaTime;
    }

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
    for (auto &ability : movementAbilities)
    {
        ability->tryJump(*this);
    }
}

void Player::moveLeft()
{
    velocity.x = -moveSpeed;
    isFacingLeft = true;
}

void Player::moveRight()
{
    velocity.x = moveSpeed;
    isFacingLeft = false;
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

    bool isFalling = (velY > 0.0f);
    float centerX = position.x + size.x / 2.0f;
    float verticalEdgeY = isFalling ? nextY + size.y : nextY;
    bool collidesWithSolidTile = tileMap.getTile(glm::vec2(centerX, verticalEdgeY)).isSolid();

    if (collidesWithSolidTile)
    {
        if (isFalling)
            isOnGround = true;
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

bool Player::facingLeft() const
{
    return isFacingLeft;
}

void Player::clampToTileMapBounds(const TileMap &tileMap)
{
    const int mapWidth = tileMap.getWorldWidth();
    const int mapHeight = tileMap.getWorldHeight();

    glm::vec2 newPos = position;
    glm::vec2 newVel = velocity;
    bool clamped = false;

    if (newPos.x < 0.0f)
    {
        newPos.x = 0.0f;
        newVel.x = 0.0f;
        clamped = true;
    }
    else if (newPos.x + size.x > mapWidth)
    {
        newPos.x = mapWidth - size.x;
        newVel.x = 0.0f;
        clamped = true;
    }

    if (newPos.y < 0.0f)
    {
        newPos.y = 0.0f;
        newVel.y = 0.0f;
        clamped = true;
    }
    else if (newPos.y + size.y > mapHeight)
    {
        newPos.y = mapHeight - size.y;
        newVel.y = 0.0f;
        isOnGround = true;
        clamped = true;
    }

    if (clamped)
    {
        const Tile &tile = tileMap.getTile(newPos);
        if (tile.isSolid())
        {
            throw std::runtime_error("Trying to clamp player into a solid tile");
        }

        position = newPos;
        velocity = newVel;
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

void Player::dash()
{
    if (canDash() && !dashing())
    {
        velocity.y = 0;
        dashTimeLeft = dashDuration;
        dashCooldownLeft = dashCooldown;
        dashDirection = isFacingLeft ? -1 : 1;
    }
}

bool Player::dashing() const
{
    return dashTimeLeft > 0.0f;
}

float Player::getDashDuration() const
{
    return dashDuration;
}

bool Player::canDash() const
{
    return dashCooldownLeft <= 0.0f;
}

bool Player::onGround() const
{
    return isOnGround;
}

JumpAbility *Player::getJumpAbility()
{
    for (auto &ability : movementAbilities)
    {
        if (auto *jump = dynamic_cast<JumpAbility *>(ability.get()))
        {
            return jump;
        }
    }
    return nullptr;
}

void Player::setOnGround(bool isOnGround)
{
    this->isOnGround = isOnGround;
}

void Player::setVelocity(const glm::vec2 &velocity)
{
    this->velocity = velocity;
}