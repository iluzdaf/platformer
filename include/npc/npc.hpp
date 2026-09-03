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
        glm::vec2 feet,
        std::optional<std::pair<glm::vec2, glm::vec2>> walkBetween = std::nullopt);

    const NpcSpawnData &getSpawn() const;
    const std::optional<std::pair<glm::vec2, glm::vec2>> &getWalk() const;
    void setSpawnTile(glm::ivec2 tilePosition, glm::vec2 feet);
    void setPatrol(PatrolData patrol, std::pair<glm::vec2, glm::vec2> walkBetween);
    void clearPatrol();

private:
    void takeBehaviorFrom(std::optional<std::pair<glm::vec2, glm::vec2>> walkBetween);

    NpcSpawnData spawn;
    std::optional<std::pair<glm::vec2, glm::vec2>> walk;
    NpcData npcData;
};
