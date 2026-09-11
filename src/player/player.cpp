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

const PlayerData &Player::builtFrom() const
{
    return data;
}

void Player::completeLevel()
{
    if (levelCompleted)
        return;

    levelCompleted = true;
    onLevelComplete();
}

void Player::postFixedUpdate()
{
    const ActorContactState &contacts = observed().contacts;
    if (decided().dash.emit)
        onDash();

    if (decided().swing.emit)
        onAttack();

    if (decided().wallJump.emit)
        onWallJump();

    if (decided().wallSlide.emit)
        onWallSliding();

    if (observed().fell > data.fallFromHeightThreshold)
        onFallFromHeight();

    if (!contacts.wasHitCeiling && contacts.hitCeiling)
        onHitCeiling();
}
