#include "game/player/player.hpp"

Player::Player(const PlayerData &data)
    : Actor(data.motionData, data.animationData),
      data(data)
{
    std::unique_ptr<InputBehavior> newInputBehavior = std::make_unique<InputBehavior>();
    inputBehavior = newInputBehavior.get();
    setBehavior(std::move(newInputBehavior));
}

void Player::setInputIntentions(const InputIntentions &intentions)
{
    inputBehavior->setIntentions(intentions);
}

void Player::postFixedUpdate()
{
    const ActorMotionState &motionState = motion.getState();
    if (motionState.dash.emit)
        onDash();

    if (motionState.wallJump.emit)
        onWallJump();

    if (motionState.wallSlide.emit)
        onWallSliding();

    if (!motionState.contacts.wasOnGround && motionState.contacts.onGround &&
        motionState.previousVelocity.y > data.fallFromHeightThreshold)
        onFallFromHeight();

    if (!motionState.contacts.wasHitCeiling && motionState.contacts.hitCeiling)
        onHitCeiling();
}
