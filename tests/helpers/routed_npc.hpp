#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "actor/actor_facts.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/state_script.hpp"
#include "game/level.hpp"
#include "input/input_intentions.hpp"
#include "navigation/route_walker.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "timing/fixed_time_step.hpp"

class RoutingTo : public StateScript
{
public:
    explicit RoutingTo(int destination) : destination(destination)
    {
    }

    void enter(const std::string &) override
    {
    }

    InputIntentions decide(
        const std::string &,
        RouteWalker &walker,
        const ActorFacts &facts,
        float deltaTime) override
    {
        if (!routed && walker.isAnchored())
        {
            walker.takeRouteTo(facts, destination);
            routed = true;
            firstLeg = walker.getTargetNodeId();
        }

        std::optional<int> at = walker.getCurrentNodeId();
        if (at && (passedThrough.empty() || passedThrough.back() != *at))
            passedThrough.push_back(*at);

        arrived = routed && walker.routeFinished() && at == destination;
        return walker.follow(deltaTime, facts);
    }

    void exit(const std::string &) override
    {
    }

    int destination;
    bool routed = false;
    bool arrived = false;
    std::optional<int> firstLeg;
    std::vector<int> passedThrough;
};

struct RouteTaken
{
    bool arrived = false;
    float seconds = 0.0f;
    std::optional<int> firstLeg;
    std::vector<int> passedThrough;
    glm::vec2 feet = glm::vec2(0.0f);
};

inline NpcData routedBy(const ActorData &actorData)
{
    BehaviorStateData going;
    going.name = "go";
    going.does = ScriptedBehaviorData{"go"};

    NpcData npcData;
    npcData.actorData = actorData;
    npcData.stateMachineBehaviorData = StateMachineBehaviorData{{going}, {}};
    npcData.script.path = "routed by the test";
    return npcData;
}

inline RouteTaken takeTheRoute(
    const Level &level,
    const ActorData &actorData,
    glm::vec2 from,
    int destination,
    int steps = 1000)
{
    NpcSpawnData spawn{"routed", from, std::nullopt};
    Npc npc(spawn, routedBy(actorData));
    auto script = std::make_unique<RoutingTo>(destination);
    RoutingTo &routing = *script;
    npc.scriptStatesWith(std::move(script));

    int step = 0;
    for (; step < steps && !routing.arrived; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(PhysicsStep, level);
    }

    return {
        routing.arrived,
        static_cast<float>(step) * PhysicsStep,
        routing.firstLeg,
        routing.passedThrough,
        npc.feet()};
}
