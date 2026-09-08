#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "actor/actor.hpp"

class Npc : public Actor
{
public:
    Npc(const NpcSpawnData &spawn, const NpcData &npcData);

    const NpcSpawnData &getSpawn() const;
    const std::string &type() const;
    int contactDamage() const;

private:
    void died() override;
    NpcSpawnData spawn;
    NpcData npcData;
};
