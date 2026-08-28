#include "npc/npc.hpp"
#include "actor/behaviors/patrol_behavior.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"

Npc::Npc(const NpcData &data, std::optional<std::pair<glm::vec2, glm::vec2>> patrolBetween)
    : Actor(data.actorData)
{
    if (data.stateMachineBehaviorData)
        setBehavior(
            std::make_unique<StateMachineBehavior>(
                data.stateMachineBehaviorData.value(), patrolBetween));
    else if (data.patrolBehaviorData)
        setBehavior(
            std::make_unique<PatrolBehavior>(data.patrolBehaviorData.value(), patrolBetween));
}
