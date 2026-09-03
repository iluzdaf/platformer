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
    takeBehaviorFromPatrol();
    moveTo(this->spawn.position);
}

void Npc::takeBehaviorFromPatrol()
{
    if (!npcData.stateMachineBehaviorData)
        return;

    std::optional<std::pair<glm::vec2, glm::vec2>> walk;
    if (spawn.patrol)
        walk = std::pair(spawn.patrol->from, spawn.patrol->to);

    setBehavior(
        std::make_unique<StateMachineBehavior>(npcData.stateMachineBehaviorData.value(), walk));
}

const NpcSpawnData &Npc::getSpawn() const
{
    return spawn;
}

void Npc::moveTo(glm::vec2 position)
{
    spawn.position = position;
    setPosition(position - getPhysicsBody().getBottomCenterOffset());
}

void Npc::setPatrol(PatrolData patrol)
{
    spawn.patrol = patrol;
    takeBehaviorFromPatrol();
}

void Npc::clearPatrol()
{
    spawn.patrol.reset();
    takeBehaviorFromPatrol();
}
