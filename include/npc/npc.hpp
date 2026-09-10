#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <string>
#include "actor/hurting.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "actor/actor.hpp"

class Npc : public Actor
{
public:
    Npc(const NpcSpawnData &spawn, const NpcData &npcData);

    const NpcSpawnData &getSpawn() const;
    const NpcData &builtFrom() const;
    const std::string &type() const;
    float tuning(const std::string &name) const;
    std::optional<Hurting> hurting() const override;

private:
    void died() override;
    NpcSpawnData spawn;
    NpcData npcData;
};
