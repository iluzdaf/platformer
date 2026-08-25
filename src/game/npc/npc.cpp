#include "game/npc/npc.hpp"
#include "game/actor/behaviors/patrol_behavior.hpp"

Npc::Npc(const NpcData &data)
    : Actor(data.actorData)
{
    if (data.patrolBehaviorData)
        setBehavior(std::make_unique<PatrolBehavior>(data.patrolBehaviorData.value()));
}
