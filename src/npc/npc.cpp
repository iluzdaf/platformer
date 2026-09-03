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
    glm::vec2 feet,
    std::optional<std::pair<glm::vec2, glm::vec2>> walkBetween)
    : Actor(npcData.actorData), spawn(spawn), npcData(npcData)
{
    takeBehaviorFrom(walkBetween);
    setPosition(feet - getPhysicsBody().getBottomCenterOffset());
}

void Npc::takeBehaviorFrom(std::optional<std::pair<glm::vec2, glm::vec2>> walkBetween)
{
    walk = walkBetween;
    if (!npcData.stateMachineBehaviorData)
        return;

    setBehavior(
        std::make_unique<StateMachineBehavior>(
            npcData.stateMachineBehaviorData.value(), walkBetween));
}

const NpcSpawnData &Npc::getSpawn() const
{
    return spawn;
}

const std::optional<std::pair<glm::vec2, glm::vec2>> &Npc::getWalk() const
{
    return walk;
}

void Npc::setSpawnTile(glm::ivec2 tilePosition, glm::vec2 feet)
{
    spawn.tilePosition = tilePosition;
    setPosition(feet - getPhysicsBody().getBottomCenterOffset());
}

void Npc::setPatrol(PatrolData patrol, std::pair<glm::vec2, glm::vec2> walkBetween)
{
    spawn.patrol = patrol;
    takeBehaviorFrom(walkBetween);
}

void Npc::clearPatrol()
{
    spawn.patrol.reset();
    takeBehaviorFrom(std::nullopt);
}
