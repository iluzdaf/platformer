#include "game/npc/npc.hpp"
#include "game/npc/behaviors/patrol_behavior.hpp"
#include "game/tile_map/tile_map.hpp"

namespace
{
    std::unique_ptr<NpcBehavior> makeBehavior(const NpcData &data)
    {
        if (data.patrolBehaviorData)
            return std::make_unique<PatrolBehavior>(data.patrolBehaviorData.value());

        return nullptr;
    }
} // namespace

Npc::Npc(const NpcData &data)
    : Actor(data.agentData, data.animationData),
      behavior(makeBehavior(data))
{
}

InputIntentions Npc::decideIntentions(float deltaTime, const TileMap &tileMap)
{
    if (!behavior)
        return InputIntentions();

    NpcBehaviorContext context{
        tileMap.getNavigationGraph(),
        physicsBody.getPosition() + getFootOffset(),
        physicsBody.getColliderSize()};

    return behavior->decide(deltaTime, context);
}

void Npc::spawnAt(
    const glm::vec2 &position,
    const NavigationGraph &navigationGraph)
{
    setPosition(position);

    if (!behavior)
        return;

    NpcBehaviorContext context{
        navigationGraph,
        position + getFootOffset(),
        physicsBody.getColliderSize()};
    behavior->reset(context);
}

std::optional<int> Npc::getCurrentNodeId() const
{
    return behavior ? behavior->getCurrentNodeId() : std::nullopt;
}

std::optional<int> Npc::getTargetNodeId() const
{
    return behavior ? behavior->getTargetNodeId() : std::nullopt;
}
