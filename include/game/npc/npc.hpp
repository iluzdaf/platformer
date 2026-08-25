#pragma once

#include <optional>
#include "game/npc/npc_data.hpp"
#include "game/actor/actor.hpp"
#include "game/actor/behaviors/patrol_behavior.hpp"

class NavigationGraph;

class Npc : public Actor
{
public:
    explicit Npc(const NpcData &data);
    void spawnAt(
        const glm::vec2 &position,
        const NavigationGraph &navigationGraph);
    std::optional<int> getCurrentNodeId() const;
    std::optional<int> getTargetNodeId() const;

private:
    PatrolBehavior *patrolBehavior = nullptr;
};
