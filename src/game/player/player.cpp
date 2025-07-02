#include "game/player/player.hpp"
#include "game/player/player_data.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "game/tile_map/tile_map.hpp"
#include "physics/physics_data.hpp"

Player::Player(PlayerData playerData, PhysicsData physicsData)
{
    initFromData(playerData, physicsData);
}

void Player::preFixedUpdate()
{
    playerState.collisionAABBX = AABB();
    playerState.collisionAABBY = AABB();
}

void Player::fixedUpdate(
    float deltaTime,
    TileMap &tileMap,
    InputIntentions inputIntensions)
{
    physicsBody.applyGravity(deltaTime);

    playerState.targetVelocity = physicsBody.getVelocity();

    movementSystem.fixedUpdate(playerState, inputIntensions, deltaTime);

    physicsBody.setVelocity(playerState.targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    animationManager.update(deltaTime, playerState);

    updatePlayerPhysicsState(tileMap);
    updatePlayerState();

    if (!playerState.wasOnGround && playerState.onGround &&
        playerState.previousVelocity.y > fallFromHeightThreshold)
        onFallFromHeight();
    if (!playerState.wasHitCeiling && playerState.hitCeiling)
        onHitCeiling();

    physicsBody.setVelocity(glm::vec2(0, physicsBody.getVelocity().y));
}

void Player::updatePlayerPhysicsState(const TileMap &tileMap)
{
    playerState.wasOnGround = playerState.onGround;
    playerState.onGround = physicsBody.contactWithGround(tileMap);
    playerState.wasHitCeiling = playerState.hitCeiling;
    playerState.hitCeiling = physicsBody.contactWithCeiling(tileMap);
    playerState.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    playerState.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
    if (playerState.touchingLeftWall)
        playerState.wasLastWallLeft = true;
    else if (playerState.touchingRightWall)
        playerState.wasLastWallLeft = false;

    if (!physicsBody.getCollisionAABBX().isEmpty())
        playerState.collisionAABBX.expandToInclude(physicsBody.getCollisionAABBX());
    if (!physicsBody.getCollisionAABBY().isEmpty())
        playerState.collisionAABBY.expandToInclude(physicsBody.getCollisionAABBY());
}

void Player::updatePlayerState()
{
    playerState.position = physicsBody.getPosition();
    playerState.previousVelocity = playerState.velocity;
    playerState.velocity = physicsBody.getVelocity();
    playerState.colliderSize = physicsBody.getColliderSize();
    playerState.colliderOffset = physicsBody.getColliderOffset();

    playerState.size = size;
    playerState.facingLeft = playerState.velocity.x > 0 ? false : (playerState.velocity.x < 0 ? true : playerState.facingLeft);

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
    playerState.position = position;
}

AABB Player::getAABB() const
{
    return physicsBody.getAABB();
}

void Player::reset()
{
    physicsBody.reset();

    animationManager.reset();

    playerState = PlayerState();
    updatePlayerState();
}

void Player::initFromData(PlayerData playerData, PhysicsData physicsData)
{
    size = playerData.size;
    fallFromHeightThreshold = playerData.fallFromHeightThreshold;

    physicsBody.setColliderSize(playerData.colliderSize);
    physicsBody.setColliderOffset(playerData.colliderOffset);
    physicsBody.setGravity(physicsData.gravity);

    movementSystem.clear();
    if (playerData.moveAbilityData)
        movementSystem.addAbility(std::make_unique<MoveAbility>(*playerData.moveAbilityData));
    if (playerData.jumpAbilityData)
        movementSystem.addAbility(std::make_unique<JumpAbility>(*playerData.jumpAbilityData));
    if (playerData.dashAbilityData)
        movementSystem.addAbility(std::make_unique<DashAbility>(*playerData.dashAbilityData));
    if (playerData.wallSlideAbilityData)
        movementSystem.addAbility(std::make_unique<WallSlideAbility>(*playerData.wallSlideAbilityData));
    if (playerData.wallJumpAbilityData)
        movementSystem.addAbility(std::make_unique<WallJumpAbility>(*playerData.wallJumpAbilityData));
    if (playerData.climbAbilityData)
        movementSystem.addAbility(std::make_unique<ClimbAbility>(*playerData.climbAbilityData));
    if (playerData.climbMoveAbilityData)
        movementSystem.addAbility(std::make_unique<ClimbMoveAbility>(*playerData.climbMoveAbilityData));

    animationManager.clear();
    animationManager.addAnimation(PlayerAnimationState::Idle, SpriteAnimation(playerData.idleSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Walk, SpriteAnimation(playerData.walkSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Dash, SpriteAnimation(playerData.dashSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Jump, SpriteAnimation(playerData.jumpSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Fall, SpriteAnimation(playerData.fallSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::WallSlide, SpriteAnimation(playerData.wallSlideSpriteAnimationData));
}

MovementSystem &Player::getMovementSystem()
{
    return movementSystem;
}