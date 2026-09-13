#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "actor/actor_facts.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "actor/behaviors/scripted_behavior.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_script.hpp"
#include "helpers/actor_facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    struct Recording : StateScript
    {
        std::vector<std::string> heard;
        InputIntentions wanted;
        std::function<void(RouteWalker &)> peek;

        void enter(const std::string &call) override
        {
            heard.push_back("enter " + call);
        }

        InputIntentions decide(
            const std::string &call,
            RouteWalker &walker,
            const ActorFacts &,
            float) override
        {
            heard.push_back("decide " + call);
            if (peek)
                peek(walker);
            return wanted;
        }

        void exit(const std::string &call) override
        {
            heard.push_back("exit " + call);
        }
    };

    ScriptedBehavior scriptedBy(Recording &script, const std::string &call = "lurk")
    {
        ScriptedBehavior behavior(ScriptedBehaviorData{call});
        behavior.scriptWith(&script);
        return behavior;
    }
}

TEST_CASE("A scripted state must say what it calls", "[ScriptedBehavior]")
{
    REQUIRE_THROWS(ScriptedBehavior(ScriptedBehaviorData{}));
}

TEST_CASE("A scripted state with no script yet asks for nothing", "[ScriptedBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ScriptedBehavior behavior(ScriptedBehaviorData{"lurk"});

    InputIntentions asked = behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));

    REQUIRE(asked.direction.x == 0.0f);
}

TEST_CASE("What its script decides is what a scripted state asks for", "[ScriptedBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    Recording script;
    script.wanted.direction.x = 1.0f;
    script.wanted.attack = "bite";
    ScriptedBehavior behavior = scriptedBy(script);

    InputIntentions asked = behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));

    REQUIRE(asked.direction.x == 1.0f);
    REQUIRE(asked.attack == "bite");
}

TEST_CASE("A scripted state is entered before its first decide, once a visit", "[ScriptedBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    Recording script;
    ScriptedBehavior behavior = scriptedBy(script);

    for (int tick = 0; tick < 3; ++tick)
        behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));
    REQUIRE(
        script.heard ==
        std::vector<std::string>{"enter lurk", "decide lurk", "decide lurk", "decide lurk"});

    behavior.reset();
    behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));
    REQUIRE(script.heard.size() == 6);
    REQUIRE(script.heard[4] == "enter lurk");
}

TEST_CASE("A scripted state says it is leaving only once it has entered", "[ScriptedBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    Recording script;
    ScriptedBehavior behavior = scriptedBy(script);

    behavior.leave();
    REQUIRE(script.heard.empty());

    behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));
    behavior.leave();
    behavior.leave();
    REQUIRE(script.heard == std::vector<std::string>{"enter lurk", "decide lurk", "exit lurk"});
}

TEST_CASE("A scripted state's walker keeps in step before its script decides", "[ScriptedBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    Recording script;
    std::optional<bool> anchoredWhenAsked;
    script.peek = [&](RouteWalker &walker) { anchoredWhenAsked = walker.isAnchored(); };
    ScriptedBehavior behavior = scriptedBy(script);

    behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}));

    REQUIRE(anchoredWhenAsked == true);
    REQUIRE(behavior.getCurrentNodeId() == 1);
    REQUIRE_FALSE(behavior.getTargetNodeId().has_value());
}
