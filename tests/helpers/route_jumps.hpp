#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "navigation/input_program.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

inline std::optional<glm::vec2> whereARouteJumpLands(
    const Level &level,
    const ActorData &actorData,
    glm::vec2 takeOff,
    const InputProgram &inputs,
    float towardsX)
{
    PlayerData playerData;
    playerData.actorData = actorData;
    ScriptedIntentions input;
    Player player(playerData, input);
    player.standAt(takeOff);
    for (int settling = 0; settling < 30 && !player.onGround(); ++settling)
    {
        player.beginFrame();
        player.fixedUpdate(PhysicsStep, level);
    }

    float elapsed = 0.0f;
    for (int step = 0; step < 1000; ++step)
    {
        input.set(replaying(inputs, elapsed, player.feet().x, towardsX));
        elapsed += PhysicsStep;
        player.beginFrame();
        player.fixedUpdate(PhysicsStep, level);
        if (step > 0 && player.onGround())
            return player.feet();
    }

    return std::nullopt;
}
