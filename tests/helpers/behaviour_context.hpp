#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_behavior_context.hpp"
#include "actor/actor_contact_state.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"

inline NavigationGraph aWalkRun(int nodeCount = 5, float spacing = 96.0f)
{
    NavigationGraph navigationGraph;
    for (int id = 0; id < nodeCount; ++id)
        navigationGraph.addNode(id, {id * spacing, 192.0f});

    for (int id = 1; id < nodeCount; ++id)
    {
        navigationGraph.addEdge(id - 1, id, EdgeType::Walk);
        navigationGraph.addEdge(id, id - 1, EdgeType::Walk);
    }
    return navigationGraph;
}

inline ActorContactState standing()
{
    ActorContactState contacts;
    contacts.onGround = true;
    return contacts;
}

inline ActorBehaviorContext standingAt(
    const NavigationGraph &navigationGraph,
    glm::vec2 worldPosition,
    std::optional<glm::vec2> threatFeet = std::nullopt)
{
    return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), threatFeet, standing(), {}};
}

inline ActorBehaviorContext airborneAt(
    const NavigationGraph &navigationGraph,
    glm::vec2 worldPosition)
{
    return {
        navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), std::nullopt, ActorContactState{}};
}
