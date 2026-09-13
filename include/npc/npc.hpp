#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include "actor/behaviors/state_script.hpp"
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
    void scriptStatesWith(std::unique_ptr<StateScript> script);

private:
    NpcSpawnData spawn;
    NpcData npcData;
    std::unique_ptr<StateScript> stateScript;
};
