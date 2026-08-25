#include "game/player/player.hpp"

Player::Player(const PlayerData &data)
    : Actor(data.motionData, data.animationData),
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
    const ActorMotionState &motionState = motion.getState();
    if (motionState.emitDash)
        onDash();

    if (motionState.emitWallJump)
        onWallJump();

    if (motionState.emitWallSliding)
        onWallSliding();

    if (!motionState.contacts.wasOnGround && motionState.contacts.onGround &&
        motionState.previousVelocity.y > data.fallFromHeightThreshold)
        onFallFromHeight();

    if (!motionState.contacts.wasHitCeiling && motionState.contacts.hitCeiling)
        onHitCeiling();
}
