#include "game/player/player.hpp"

Player::Player(const PlayerData &data)
    : data(data),
      agent(data.agentData)
{
    animationManager.addAnimation(PlayerAnimationState::Idle, SpriteAnimation(data.idleSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Walk, SpriteAnimation(data.walkSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Dash, SpriteAnimation(data.dashSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Jump, SpriteAnimation(data.jumpSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::Fall, SpriteAnimation(data.fallSpriteAnimationData));
    animationManager.addAnimation(PlayerAnimationState::WallSlide, SpriteAnimation(data.wallSlideSpriteAnimationData));
}

void Player::preFixedUpdate()
{
    agent.resetCollisionAABB();
}

void Player::fixedUpdate(
    float deltaTime,
    const TileMap &tileMap,
    const InputIntentions &inputIntensions)
{
    agent.fixedUpdate(deltaTime, tileMap, inputIntensions);

    animationManager.update(deltaTime, agent.getState());

    updateState();

    emitSignals();
}

const PlayerState &Player::getState() const
{
    return playerState;
}

const Agent &Player::getAgent() const
{
    return agent;
}

void Player::setPosition(const glm::vec2 &position)
{
    agent.setPosition(position);
}

void Player::updateState()
{
    const AgentState &agentState = agent.getState();
    playerState.facingLeft = agentState.velocity.x > 0 ? false : (agentState.velocity.x < 0 ? true : playerState.facingLeft);

    playerState.currentAnimationUVStart = animationManager.getCurrentAnimation().getUVStart();
    playerState.currentAnimationUVEnd = animationManager.getCurrentAnimation().getUVEnd();
    playerState.currentAnimationState = animationManager.getCurrentState();
}

void Player::emitSignals()
{
    const AgentState &agentState = agent.getState();
    if (agentState.emitDash)
        onDash();

    if (agentState.emitWallJump)
        onWallJump();

    if (agentState.emitWallSliding)
        onWallSliding();

    if (!agentState.wasOnGround && agentState.onGround &&
        agentState.previousVelocity.y > data.fallFromHeightThreshold)
        onFallFromHeight();

    if (!agentState.wasHitCeiling && agentState.hitCeiling)
        onHitCeiling();
}