#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "input/input_intentions.hpp"

class NavigationGraph;

struct NpcBehaviorContext
{
    const NavigationGraph &navigationGraph;
    glm::vec2 worldPosition;
    glm::vec2 colliderSize;
};

class NpcBehavior
{
public:
    virtual ~NpcBehavior() = default;
    virtual void reset(const NpcBehaviorContext &context) = 0;
    virtual InputIntentions decide(float deltaTime, const NpcBehaviorContext &context) = 0;
    virtual std::optional<int> getCurrentNodeId() const = 0;
    virtual std::optional<int> getTargetNodeId() const = 0;
};
