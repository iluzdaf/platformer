#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "game/level.hpp"
#include "actor/abilities/ability_states.hpp"
#include "helpers/actors.hpp"
#include "input/input_intentions.hpp"
#include "navigation/input_program.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "tile_map/touching_tiles.hpp"
#include "timing/fixed_time_step.hpp"

inline std::optional<glm::vec2> whereARouteJumpLands(
    const Level &level,
    const ActorData &actorData,
    glm::vec2 takeOff,
    const InputProgram &inputs,
    float towardsX,
    float wallDirection = 0.0f,
    bool touchingTiles = false)
{
    PlayerData playerData;
    playerData.actorData = actorData;
    ScriptedIntentions input;
    Player player(playerData, input);
    float offTheWall = wallDirection * actorData.physicsBodyData.colliderSize.x * 0.5f;
    player.standAt(takeOff - glm::vec2(offTheWall, 0.0f));
    InputIntentions holding;
    holding.climbRequested = true;
    holding.direction.x = wallDirection;
    input.set(wallDirection == 0.0f ? InputIntentions{} : holding);
    auto ready = [&]
    { return wallDirection == 0.0f ? player.onGround() : player.abilityStates().wallHang.active; };
    for (int settling = 0; settling < 30 && !ready(); ++settling)
    {
        player.beginFrame();
        player.fixedUpdate(PhysicsStep, level);
    }

    float elapsed = 0.0f;
    bool airborne = false;
    for (int step = 0; step < 1000; ++step)
    {
        float stride =
            actorData.abilities.move ? actorData.abilities.move->moveSpeed * PhysicsStep : 0.0f;
        input.set(replaying(inputs, elapsed, player.feet().x, towardsX, stride));
        elapsed += PhysicsStep;
        player.beginFrame();
        player.fixedUpdate(PhysicsStep, level);
        if (touchingTiles)
            touchTiles(player, level.getTileMap());
        if (!player.alive())
            return std::nullopt;
        if (!player.onGround())
            airborne = true;
        else if (airborne)
            return player.feet();
    }

    return std::nullopt;
}
