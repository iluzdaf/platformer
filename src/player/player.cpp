#include "player/player.hpp"
#include "player/player_data.hpp"
#include "input/intention_source.hpp"
#include "actor/actor.hpp"
#include <memory>
#include "actor/behaviors/input_behavior.hpp"
#include "actor/actor_motion_state.hpp"

Player::Player(const PlayerData &data, const IntentionSource &intentionSource)
    : Actor(data.actorData), data(data)
{
    setBehavior(std::make_unique<InputBehavior>(intentionSource));
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
