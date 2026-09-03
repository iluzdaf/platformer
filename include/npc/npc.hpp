#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "actor/actor.hpp"

class Npc : public Actor
{
public:
    Npc(const NpcSpawnData &spawn, const NpcData &npcData);

    const NpcSpawnData &getSpawn() const;
    void moveTo(glm::vec2 position);
    void setPatrol(PatrolData patrol);
    void clearPatrol();

private:
    void takeBehaviorFromPatrol();

    NpcSpawnData spawn;
    NpcData npcData;
};
