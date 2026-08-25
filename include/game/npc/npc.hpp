#pragma once

#include <memory>
#include <optional>
#include "game/npc/npc_data.hpp"
#include "game/npc/npc_behavior.hpp"
#include "game/actor/actor.hpp"

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

protected:
    InputIntentions decideIntentions(float deltaTime, const TileMap &tileMap) override;

private:
    std::unique_ptr<NpcBehavior> behavior;
};
