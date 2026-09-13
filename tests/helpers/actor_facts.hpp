#pragma once

#include <optional>
#include <string_view>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/actor_facts.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/observed.hpp"
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

inline ActorFacts standingAt(
    const NavigationGraph &navigationGraph,
    glm::vec2 worldPosition,
    std::optional<glm::vec2> threatFeet = std::nullopt)
{
    return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), threatFeet, standing(), {}};
}

inline ActorFacts airborneAt(const NavigationGraph &navigationGraph, glm::vec2 worldPosition)
{
    return {
        navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), std::nullopt, ActorContactState{}};
}

inline ActorFacts factsOf(
    const AbilityStates &states,
    const Observed &observed,
    std::string_view inState = {})
{
    static const NavigationGraph nowhere;
    ActorFacts facts{nowhere, glm::vec2(0.0f), glm::vec2(0.0f), std::nullopt, observed.contacts};
    facts.abilityStates = &states;
    facts.velocity = observed.velocity;
    facts.alive = observed.alive;
    facts.inState = inState;
    return facts;
}
