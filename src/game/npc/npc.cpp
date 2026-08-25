#include "game/npc/npc.hpp"
#include "game/tile_map/tile_map.hpp"

Npc::Npc(const NpcData &data)
    : Actor(data.motionData, data.animationData)
{
    if (data.patrolBehaviorData)
    {
        std::unique_ptr<PatrolBehavior> newPatrolBehavior =
            std::make_unique<PatrolBehavior>(data.patrolBehaviorData.value());
        patrolBehavior = newPatrolBehavior.get();
        setBehavior(std::move(newPatrolBehavior));
    }
}

void Npc::spawnAt(
    const glm::vec2 &position,
    const NavigationGraph &navigationGraph)
{
    setPosition(position);

    if (!behavior)
        return;

    behavior->reset(behaviorContextAt(position, navigationGraph));
}

std::optional<int> Npc::getCurrentNodeId() const
{
    return patrolBehavior ? patrolBehavior->getCurrentNodeId() : std::nullopt;
}

std::optional<int> Npc::getTargetNodeId() const
{
    return patrolBehavior ? patrolBehavior->getTargetNodeId() : std::nullopt;
}
