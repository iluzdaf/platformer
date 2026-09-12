#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/armed.hpp"

struct AnimatorData;
struct Observed;
struct Appearance;
struct NpcData;
class Level;
class Npc;

struct ActorShown
{
    enum class What
    {
        None,
        Player,
        Npc,
        Pickup
    };

    What what = What::None;
    std::size_t index = 0;

    bool operator==(const ActorShown &) const = default;
};

struct ActorAsked
{
    ActorShown show;
    bool removeShown = false;
    bool clearShownBeat = false;
};

std::optional<std::string> npcsThatCannotGetBack(const Level &level);

ActorAsked drawActorsInLevel(
    const Level &level,
    const std::optional<AnimatorData> &playerAnimations,
    const Observed &playerObserved,
    const glm::vec2 &playerFeet,
    const Appearance &playerAppearance,
    const std::map<std::string, NpcData> &npcTypes,
    ActorShown showing,
    std::optional<Armed> &armed);
