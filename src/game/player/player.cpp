#include <cassert>
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
    physicsBody.setColliderSize(playerData.colliderSize);
    physicsBody.setColliderOffset(playerData.colliderOffset);
    physicsBody.setGravity(physicsData.gravity);

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
    physicsBody.applyGravity(deltaTime);

    for (auto &ability : movementAbilities)
    {
        ability->fixedUpdate(*this, playerState, deltaTime);
    }

    physicsBody.stepPhysics(deltaTime, tileMap);

    updatePlayerState(tileMap);
}

void Player::update(float deltaTime, TileMap &tileMap)
{
    for (auto &ability : movementAbilities)
    {
        ability->update(*this, playerState, deltaTime);
    }

    updateAnimation(deltaTime);

    physicsBody.setVelocity(glm::vec2(0, physicsBody.getVelocity().y));
}

void Player::updateAnimation(float deltaTime)
{
    bool isWalking = std::abs(physicsBody.getVelocity().x) > 0.001f;
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
    return physicsBody.getPosition();
}

glm::vec2 Player::getVelocity() const
{
    return physicsBody.getVelocity();
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

void Player::setVelocity(const glm::vec2 &velocity)
{
    physicsBody.setVelocity(velocity);
}

void Player::setFacingLeft(bool isFacingLeft)
{
    this->isFacingLeft = isFacingLeft;
}

void Player::updatePlayerState(const TileMap &tileMap)
{
    playerState.position = physicsBody.getPosition();
    playerState.velocity = physicsBody.getVelocity();
    playerState.colliderSize = physicsBody.getColliderSize();
    playerState.onGround = physicsBody.contactWithGround(tileMap);
    playerState.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    playerState.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);

    playerState.size = size;
    playerState.facingLeft = facingLeft();

    for (const auto &ability : movementAbilities)
    {
        ability->syncState(playerState);
    }
}

const PlayerState &Player::getPlayerState() const
{
    return playerState;
}

void Player::setPosition(const glm::vec2 &position)
{
    this->physicsBody.setPosition(position);
}

AABB Player::getAABB() const
{
    return physicsBody.getAABB();
}