#include "player/player.hpp"
#include "actor/actor_contact_state.hpp"
#include "player/player_data.hpp"
#include "input/intention_source.hpp"
#include "actor/actor.hpp"
#include <memory>
#include "actor/behaviors/input_behavior.hpp"
#include "actor/decided.hpp"

Player::Player(const PlayerData &data, const IntentionSource &intentionSource)
    : Actor(data.actorData), data(data)
{
    setBehavior(std::make_unique<InputBehavior>(intentionSource));
}

void Player::completeLevel()
{
    if (levelCompleted)
        return;

    levelCompleted = true;
    onLevelComplete();
}

void Player::hurt()
{
    onHurt();
}

void Player::died()
{
    onDeath();
}

void Player::postFixedUpdate()
{
    const ActorContactState &contacts = observed().contacts;
    if (decided().dash.emit)
        onDash();

    if (decided().wallJump.emit)
        onWallJump();

    if (decided().wallSlide.emit)
        onWallSliding();

    if (!contacts.wasOnGround && contacts.onGround &&
        observed().previousVelocity.y > data.fallFromHeightThreshold)
        onFallFromHeight();

    if (!contacts.wasHitCeiling && contacts.hitCeiling)
        onHitCeiling();
}
