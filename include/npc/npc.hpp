#pragma once

#include <optional>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "actor/actor.hpp"

class Npc : public Actor
{
public:
    Npc(const NpcSpawnData &spawn,
        const NpcData &npcData,
        std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween = std::nullopt);

    const NpcSpawnData &getSpawn() const;
    void setSpawnTile(glm::ivec2 tilePosition);
    void setPatrol(
        std::optional<PatrolData> patrol,
        std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween);

private:
    void takeBehaviorFrom(std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween);

    NpcSpawnData spawn;
    NpcData npcData;
};
