#include "player/player.hpp"
#include "player/player_data.hpp"
#include "input/intention_source.hpp"
#include "actor/actor.hpp"
#include <memory>
#include "actor/behaviors/input_behavior.hpp"

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
