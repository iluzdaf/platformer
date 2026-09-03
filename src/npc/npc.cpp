#include <memory>
#include <optional>
#include <utility>
#include "npc/npc.hpp"
#include "actor/actor.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

Npc::Npc(const NpcSpawnData &spawn, const NpcData &npcData)
    : Actor(npcData.actorData), spawn(spawn), npcData(npcData)
{
    if (npcData.stateMachineBehaviorData)
    {
        std::optional<std::pair<glm::vec2, glm::vec2>> walk;
        if (this->spawn.patrol)
            walk = std::pair(this->spawn.patrol->from, this->spawn.patrol->to);

        setBehavior(
            std::make_unique<StateMachineBehavior>(npcData.stateMachineBehaviorData.value(), walk));
    }

    setPosition(this->spawn.position - getPhysicsBody().getBottomCenterOffset());
}

const NpcSpawnData &Npc::getSpawn() const
{
    return spawn;
}
