#pragma once

#include "actor/actor_data.hpp"
#include "scripting/script_path_data.hpp"

struct PlayerData
{
    ActorData actorData;

    float fallFromHeightThreshold = 400;
    float heardAfterFalling = 2;
    ScriptPathData script;
};
