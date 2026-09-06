#pragma once

#include "game/level.hpp"
#include "player/player.hpp"
#include "timing/fixed_time_step.hpp"

inline void runFor(Player &player, const Level &level, float seconds, FixedTimeStep &timestepper)
{
    player.preFixedUpdate();
    timestepper.run(
        seconds,
        [&](float dt)
        {
            player.fixedUpdate(dt, level);
            player.postFixedUpdate();
        });
}
