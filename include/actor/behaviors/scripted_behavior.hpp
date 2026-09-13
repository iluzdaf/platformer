#pragma once

#include <optional>
#include <string>
#include "actor/actor_behavior.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

class StateScript;

class ScriptedBehavior : public ActorBehavior
{
public:
    explicit ScriptedBehavior(const ScriptedBehaviorData &data);
    void scriptWith(StateScript *newScript) override;
    void reset() override;
    void leave() override;
    InputIntentions decide(float deltaTime, const ActorFacts &context) override;
    std::optional<int> getCurrentNodeId() const override;
    std::optional<int> getTargetNodeId() const override;

private:
    std::string call;
    RouteWalker walker;
    StateScript *script = nullptr;
    bool entered = false;
};
