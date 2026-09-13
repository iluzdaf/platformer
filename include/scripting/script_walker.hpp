#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_facts.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "input/input_intentions.hpp"

class ScriptWalker
{
public:
    ScriptWalker(RouteWalker &walker, const ActorFacts &facts);
    bool anchored() const;
    bool finished() const;
    void routeTo(int node);
    InputIntentions follow(float deltaTime);
    std::optional<int> currentNode() const;
    std::optional<int> targetNode() const;
    glm::vec2 feetOf(int node) const;
    std::optional<int> furthestRefugeFrom(int from, glm::vec2 threat, float away) const;

private:
    RouteWalker &walker;
    const ActorFacts &facts;
};
