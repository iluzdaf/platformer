#include "game/player/player.hpp"

Player::Player(const PlayerData &data)
    : Actor(data.agentData, data.animationData),
      data(data)
{
}

void Player::setInputIntentions(const InputIntentions &intentions)
{
    inputIntentions = intentions;
}

InputIntentions Player::decideIntentions(float, const TileMap &)
{
    return inputIntentions;
}

void Player::postFixedUpdate()
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
