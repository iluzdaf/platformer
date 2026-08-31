#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

struct ActorMotionState;
struct ActorState;
class Level;
class Npc;

struct ActorShown
{
    enum class What
    {
        None,
        Player,
        Npc
    };

    What what = What::None;
    std::size_t npcIndex = 0;

    bool operator==(const ActorShown &) const = default;
};

struct ActorAsked
{
    ActorShown show;
    bool removeShownNpc = false;
};

ActorAsked drawActorsInLevel(
    const Level &level,
    const std::vector<std::unique_ptr<Npc>> &npcs,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &playerState,
    ActorShown showing);
