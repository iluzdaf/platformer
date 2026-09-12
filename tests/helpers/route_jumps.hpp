#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "input/input_intentions.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

inline std::optional<glm::vec2> whereARouteJumpLands(
    const Level &level,
    const ActorData &actorData,
    glm::vec2 takeOff,
    float direction,
    float holdFor)
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

    float heldFor = 0.0f;
    for (int step = 0; step < 1000; ++step)
    {
        InputIntentions running;
        running.direction.x = direction;
        if (heldFor < holdFor)
        {
            running.jumpRequested = true;
            running.jumpHeld = true;
            heldFor += PhysicsStep;
        }
        input.set(running);
        player.beginFrame();
        player.fixedUpdate(PhysicsStep, level);
        if (step > 0 && player.onGround())
            return player.feet();
    }

    return std::nullopt;
}
