#include <memory>
#include <optional>
#include <utility>
#include "npc/npc.hpp"
#include "actor/actor.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

Npc::Npc(
    const NpcSpawnData &spawn,
    const NpcData &npcData,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : Actor(npcData.actorData), spawn(spawn), npcData(npcData)
{
    takeBehaviorFrom(patrolBetween);
}

void Npc::takeBehaviorFrom(std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
{
    if (!npcData.stateMachineBehaviorData)
        return;

    setBehavior(
        std::make_unique<StateMachineBehavior>(
            npcData.stateMachineBehaviorData.value(), patrolBetween));
}

const NpcSpawnData &Npc::getSpawn() const
{
    return spawn;
}

void Npc::setSpawnTile(glm::ivec2 tilePosition)
{
    spawn.tilePosition = tilePosition;
}

void Npc::setPatrol(
    std::optional<PatrolData> patrol,
    std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
{
    spawn.patrol = patrol;
    takeBehaviorFrom(patrolBetween);
}
