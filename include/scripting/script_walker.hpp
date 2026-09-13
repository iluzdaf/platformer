#pragma once

#include <optional>
#include <vector>
#include "navigation/navigation_place.hpp"
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
    void routeTo(int node, std::optional<glm::vec2> stopShortAt = std::nullopt);
    InputIntentions follow(float deltaTime);
    std::optional<int> currentNode() const;
    std::optional<int> targetNode() const;
    glm::vec2 feetOf(int node) const;
    std::optional<int> furthestRefugeFrom(int from, glm::vec2 threat, float away) const;
    std::optional<PlaceOnThePath> placeOnThePath(glm::vec2 point) const;
    int endOfThePathBeyond(const PlaceOnThePath &place, glm::vec2 comingFrom) const;
    std::vector<int> walkableFrom(int node) const;
    bool standsAt(glm::vec2 point) const;

private:
    RouteWalker &walker;
    const ActorFacts &facts;
};
