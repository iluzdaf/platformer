#pragma once

#include "game/npc/npc_behavior.hpp"
#include "game/npc/behaviors/patrol_behavior_data.hpp"

class PatrolBehavior : public NpcBehavior
{
public:
    explicit PatrolBehavior(const PatrolBehaviorData &data);
    void reset(const NpcBehaviorContext &context) override;
    InputIntentions decide(float deltaTime, const NpcBehaviorContext &context) override;
    std::optional<int> getCurrentNodeId() const override;
    std::optional<int> getTargetNodeId() const override;

private:
    PatrolBehaviorData data;
    std::optional<int> currentNodeId, targetNodeId, previousNodeId;

    void pickTarget(const NpcBehaviorContext &context);
    bool hasArrived(const NpcBehaviorContext &context) const;
};
