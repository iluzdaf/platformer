#pragma once

#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "timing/fixed_time_step.hpp"

inline void runAFrame(Player &player, const Level &level, FixedTimeStep &timestepper)
{
    player.preFixedUpdate();
    timestepper.run(
        1.0f / 60.0f,
        [&](float dt)
        {
            player.fixedUpdate(dt, level);
            player.postFixedUpdate();
        });
}

inline GameData shippedGameData()
{
    GameData gameData = loadGameData();
    return gameData;
}
