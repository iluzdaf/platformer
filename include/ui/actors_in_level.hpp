#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/armed.hpp"

struct ActorMotionState;
struct ActorState;
struct NpcData;
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
    bool clearShownBeat = false;
    std::optional<std::string> addNpcOfType;
};

ActorAsked drawActorsInLevel(
    const Level &level,
    const std::vector<std::unique_ptr<Npc>> &npcs,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcTypes,
    ActorShown showing,
    std::optional<Armed> &armed);
