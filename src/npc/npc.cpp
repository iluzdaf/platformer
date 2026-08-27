#include "npc/npc.hpp"
#include "actor/behaviors/patrol_behavior.hpp"

Npc::Npc(const NpcData &data)
    : Actor(data.actorData)
{
    if (data.patrolBehaviorData)
        setBehavior(std::make_unique<PatrolBehavior>(data.patrolBehaviorData.value()));
}
