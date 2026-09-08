#pragma once

#include <string>

#include "actor/actor_data.hpp"

struct PlayerData
{
    ActorData actorData;

    float fallFromHeightThreshold = 400;
    std::string script;
};
