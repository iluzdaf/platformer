#pragma once

#include "game/actor/actor_data.hpp"

struct PlayerData
{
    ActorData actorData;

    float fallFromHeightThreshold = 400;
};
