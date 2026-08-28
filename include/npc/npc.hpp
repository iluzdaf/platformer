#pragma once

#include <optional>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "npc/npc_data.hpp"
#include "actor/actor.hpp"

class Npc : public Actor
{
public:
    explicit Npc(
        const NpcData &data,
        std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween = std::nullopt);
};
