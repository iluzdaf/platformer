#include <cassert>
#include <algorithm>
#include "game/player/player.hpp"
#include "game/tile_map/tile.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player_data.hpp"
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "physics/physics_data.hpp"

Player::Player(const PlayerData &playerData, const PhysicsData &physicsData)
    : size(playerData.size),
      idleAnim(SpriteAnimation(playerData.idleSpriteAnimationData)),
      walkAnim(SpriteAnimation(playerData.walkSpriteAnimationData))
{
    gravity = physicsData.gravity;

    if (playerData.jumpAbilityData)
    {
        movementAbilities.emplace_back(std::make_unique<JumpAbility>(*playerData.jumpAbilityData));
    }

    if (playerData.dashAbilityData)
    {
        movementAbilities.emplace_back(std::make_unique<DashAbility>(*playerData.dashAbilityData));
    }

    if (playerData.moveAbilityData)
    {
        movementAbilities.emplace_back(std::make_unique<MoveAbility>(*playerData.moveAbilityData));
    }

    if (playerData.wallSlideAbilityData)
    {
        movementAbilities.emplace_back(std::make_unique<WallSlideAbility>(*playerData.wallSlideAbilityData));
    }

    if (playerData.wallJumpAbilityData)
    {
        movementAbilities.emplace_back(std::make_unique<WallJumpAbility>(*playerData.wallJumpAbilityData));
    }
}

void Player::fixedUpdate(float deltaTime, TileMap &tileMap)
{
    resetTransientState();

    checkWallContact(tileMap);

    velocity.y += gravity * deltaTime;

    for (auto &ability : movementAbilities)
    {
        ability->fixedUpdate(*this, playerState, deltaTime);
    }

    glm::vec2 nextPosition = position + velocity * deltaTime;
    resolveVerticalCollision(nextPosition.y, velocity.y, tileMap);
    resolveHorizontalCollision(nextPosition.x, velocity.x, tileMap, nextPosition.y);
    handlePickup(tileMap);
    handleSpikes(tileMap);

    position = nextPosition;

    clampToTileMapBounds(tileMap);

    updatePlayerState();
}

void Player::update(float deltaTime, TileMap &tileMap)
{
    for (auto &ability : movementAbilities)
    {
        ability->update(*this, playerState, deltaTime);
    }

    updateAnimation(deltaTime);

    velocity.x = 0.0f;

    updatePlayerState();
}

void Player::updateAnimation(float deltaTime)
{
    bool isWalking = std::abs(velocity.x) > 0.01f;
    PlayerAnimationState newState = isWalking ? PlayerAnimationState::Walk : PlayerAnimationState::Idle;
    if (newState != animState)
    {
        animState = newState;
    }
    getCurrentAnimation().update(deltaTime);
}

void Player::jump()
{
    for (auto &ability : movementAbilities)
    {
        ability->tryJump(*this, playerState);
    }
}

void Player::moveLeft()
{
    for (auto &ability : movementAbilities)
    {
        ability->tryMoveLeft(*this, playerState);
    }
}

void Player::moveRight()
{
    for (auto &ability : movementAbilities)
    {
        ability->tryMoveRight(*this, playerState);
    }
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
        nextY = snapToTileEdge(tileY, tileSize, isFalling, size.y);
        velY = 0.0f;
    }
}

void Player::resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, float nextY)
{
    if (std::abs(velX) < 0.0001f)
        return;

    bool isMovingRight = (velX > 0);
    float bottomY = nextY + size.y - 1.0f;
    float leadingEdgeX = isMovingRight ? nextX + size.x : nextX;
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

SpriteAnimation &Player::getCurrentAnimation()
{
    switch (animState)
    {
    case PlayerAnimationState::Idle:
        return idleAnim;
    case PlayerAnimationState::Walk:
        return walkAnim;
    }
    return idleAnim;
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
        onLevelComplete();
    }
}

glm::vec2 Player::getSize() const
{
    return size;
}

void Player::dash()
{
    for (auto &ability : movementAbilities)
    {
        ability->tryDash(*this, playerState);
    }
}

bool Player::onGround() const
{
    return isOnGround;
}

void Player::setOnGround(bool isOnGround)
{
    this->isOnGround = isOnGround;
}

void Player::setVelocity(const glm::vec2 &velocity)
{
    this->velocity = velocity;
}

void Player::setFacingLeft(bool isFacingLeft)
{
    this->isFacingLeft = isFacingLeft;
}

void Player::updatePlayerState()
{
    playerState.position = position;
    playerState.velocity = velocity;
    playerState.size = size;
    playerState.onGround = onGround();
    playerState.facingLeft = facingLeft();
    playerState.touchingRightWall = touchingRightWall();
    playerState.touchingLeftWall = touchingLeftWall();

    for (const auto &ability : movementAbilities)
    {
        ability->syncState(playerState);
    }
}

const PlayerState &Player::getPlayerState() const
{
    return playerState;
}

bool Player::touchingLeftWall() const
{
    return isTouchingLeftWall;
}

bool Player::touchingRightWall() const
{
    return isTouchingRightWall;
}

void Player::checkWallContact(const TileMap &tileMap)
{
    float probeOffset = 0.05f;
    if (tileMap.getTile(glm::vec2(position.x - probeOffset, position.y + size.y * 0.5f)).isSolid())
    {
        isTouchingLeftWall = true;
    }

    if (tileMap.getTile(glm::vec2(position.x + size.x + probeOffset, position.y + size.y * 0.5f)).isSolid())
    {
        isTouchingRightWall = true;
    }
}

void Player::resetTransientState()
{
    isTouchingLeftWall = false;
    isTouchingRightWall = false;
}

void Player::setPosition(const glm::vec2 &position)
{
    this->position = position;
}

void Player::handleSpikes(TileMap &tileMap)
{
    const Tile &tile = tileMap.getTile(position);
    if (tile.isSpikes())
    {
        onDeath();
    }
}