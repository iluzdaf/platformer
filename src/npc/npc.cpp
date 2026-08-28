#include "npc/npc.hpp"
#include "actor/behaviors/patrol_behavior.hpp"

Npc::Npc(const NpcData &data, std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : Actor(data.actorData)
{
    if (data.patrolBehaviorData)
        setBehavior(
            std::make_unique<PatrolBehavior>(data.patrolBehaviorData.value(), patrolBetween));
}
