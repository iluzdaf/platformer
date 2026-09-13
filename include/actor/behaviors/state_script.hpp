#pragma once

#include <string>
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

class RouteWalker;

class StateScript
{
public:
    virtual ~StateScript() = default;
    virtual void enter(const std::string &call) = 0;
    virtual InputIntentions decide(
        const std::string &call,
        RouteWalker &walker,
        const ActorFacts &facts,
        float deltaTime) = 0;
    virtual void exit(const std::string &call) = 0;
};
