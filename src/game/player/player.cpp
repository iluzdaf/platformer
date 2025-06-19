#include "game/player/player.hpp"
#include "game/tile_map/tile.hpp"
#include "game/tile_map/tile_map.hpp"
#include "game/player/player_data.hpp"
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "physics/physics_data.hpp"

Player::Player(const PlayerData &playerData, const PhysicsData &physicsData)
{
    initFromData(playerData, physicsData);
}

void Player::preFixedUpdate()
{
    playerState.collisionAABBX = AABB();
    playerState.collisionAABBY = AABB();
}

void Player::fixedUpdate(float deltaTime, TileMap &tileMap)
{
    physicsBody.applyGravity(deltaTime);

    for (auto &ability : movementAbilities)
    {
        ability->fixedUpdate(*this, playerState, deltaTime);
        ability->syncState(playerState);
    }

    physicsBody.stepPhysics(deltaTime, tileMap);

    animationManager.update(deltaTime, playerState);

    updatePlayerPhysicsState(tileMap);
    updatePlayerState();

    if (!playerState.wasOnGround && playerState.onGround &&
        playerState.previousVelocity.y > fallFromHeightThreshold)
        onFallFromHeight();
    if (!playerState.wasHitCeiling && playerState.hitCeiling)
        onHitCeiling();
}

void Player::update(float deltaTime, TileMap &tileMap)
{
    for (auto &ability : movementAbilities)
        ability->update(*this, playerState, deltaTime);

    physicsBody.setVelocity(glm::vec2(0, physicsBody.getVelocity().y));
}

void Player::jump()
{
    for (auto &ability : movementAbilities)
        ability->tryJump(*this, playerState);
}

void Player::moveLeft()
{
    for (auto &ability : movementAbilities)
        ability->tryMoveLeft(*this, playerState);
}

void Player::moveRight()
{
    for (auto &ability : movementAbilities)
        ability->tryMoveRight(*this, playerState);
}

glm::vec2 Player::getPosition() const
{
    return physicsBody.getPosition();
}

glm::vec2 Player::getVelocity() const
{
    return physicsBody.getVelocity();
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
        ability->tryDash(*this, playerState);
}

void Player::setVelocity(const glm::vec2 &velocity)
{
    physicsBody.setVelocity(velocity);
}

void Player::setFacingLeft(bool isFacingLeft)
{
    this->isFacingLeft = isFacingLeft;
}

void Player::updatePlayerPhysicsState(const TileMap &tileMap)
{
    playerState.wasOnGround = playerState.onGround;
    playerState.onGround = physicsBody.contactWithGround(tileMap);
    playerState.wasHitCeiling = playerState.hitCeiling;
    playerState.hitCeiling = physicsBody.contactWithCeiling(tileMap);
    playerState.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    playerState.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);

    if (!physicsBody.getCollisionAABBX().isEmpty())
        playerState.collisionAABBX.expandToInclude(physicsBody.getCollisionAABBX());
    if (!physicsBody.getCollisionAABBY().isEmpty())
        playerState.collisionAABBY.expandToInclude(physicsBody.getCollisionAABBY());
}

void Player::updatePlayerState()
{
    playerState.size = size;
    playerState.facingLeft = facingLeft();

    playerState.position = physicsBody.getPosition();
    playerState.previousVelocity = playerState.velocity;
    playerState.velocity = physicsBody.getVelocity();
    playerState.colliderSize = physicsBody.getColliderSize();
    playerState.colliderOffset = physicsBody.getColliderOffset();
    
    playerState.currentAnimationUVStart = animationManager.getCurrentAnimation().getUVStart();
    playerState.currentAnimationUVEnd = animationManager.getCurrentAnimation().getUVEnd();
    playerState.currentAnimationState = animationManager.getCurrentState();
}

const PlayerState &Player::getPlayerState() const
{
    return playerState;
}

void Player::setPosition(const glm::vec2 &position)
{
    physicsBody.setPosition(position);
}

AABB Player::getAABB() const
{
    return physicsBody.getAABB();
}

void Player::reset()
{
    physicsBody.reset();

    isFacingLeft = false;

    for (const auto &ability : movementAbilities)
        ability->reset();

    animationManager.reset();

    playerState = PlayerState();
    updatePlayerState();
}

void Player::emitWallJump()
{
    onWallJump();
}

void Player::emitDoubleJump()
{
    onDoubleJump();
}

void Player::emitDash()
{
    onDash();
}

void Player::emitWallSliding()
{
    onWallSliding();
}

void Player::initFromData(const PlayerData &playerData, const PhysicsData &physicsData)
{
    size = playerData.size;
    fallFromHeightThreshold = playerData.fallFromHeightThreshold;

    physicsBody.setColliderSize(playerData.colliderSize);
    physicsBody.setColliderOffset(playerData.colliderOffset);
    physicsBody.setGravity(physicsData.gravity);

    movementAbilities.clear();
    if (playerData.jumpAbilityData)
        movementAbilities.emplace_back(std::make_unique<JumpAbility>(*playerData.jumpAbilityData));
    if (playerData.dashAbilityData)
        movementAbilities.emplace_back(std::make_unique<DashAbility>(*playerData.dashAbilityData));
    if (playerData.moveAbilityData)
        movementAbilities.emplace_back(std::make_unique<MoveAbility>(*playerData.moveAbilityData));
    if (playerData.wallSlideAbilityData)
        movementAbilities.emplace_back(std::make_unique<WallSlideAbility>(*playerData.wallSlideAbilityData));
    if (playerData.wallJumpAbilityData)
        movementAbilities.emplace_back(std::make_unique<WallJumpAbility>(*playerData.wallJumpAbilityData));
    if (playerData.climbAbilityData)
        movementAbilities.emplace_back(std::make_unique<ClimbAbility>(*playerData.climbAbilityData));
    if (playerData.climbMoveAbilityData)
        movementAbilities.emplace_back(std::make_unique<ClimbMoveAbility>(*playerData.climbMoveAbilityData));

    animationManager.clear();
    animationManager.addAnimation(PlayerAnimationState::Idle, SpriteAnimation(playerData.idleSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Walk, SpriteAnimation(playerData.walkSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Dash, SpriteAnimation(playerData.dashSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Jump, SpriteAnimation(playerData.jumpSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Fall, SpriteAnimation(playerData.fallSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::WallSlide, SpriteAnimation(playerData.wallSlideSpriteAnimationData));
}

void Player::climb()
{
    for (auto &ability : movementAbilities)
        ability->tryClimb(*this, playerState);
}

void Player::ascend()
{
    for (auto &ability : movementAbilities)
        ability->tryAscend(*this, playerState);
}

void Player::descend()
{
    for (auto &ability : movementAbilities)
        ability->tryDescend(*this, playerState);
}