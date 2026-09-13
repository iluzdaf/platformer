#pragma once

#include <optional>
#include <string_view>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_contact_state.hpp"
#include "conditions/facts.hpp"

class NavigationGraph;
struct AbilityStates;
struct SensesData;
struct PatrolData;

struct ActorFacts
{
    const NavigationGraph &navigationGraph;
    glm::vec2 feet;
    glm::vec2 colliderSize;
    std::optional<glm::vec2> threatFeet;

    ActorContactState contacts;
    const FactsData *facts = nullptr;
    const AbilityStates *abilityStates = nullptr;
    const SensesData *senses = nullptr;
    const PatrolData *beat = nullptr;
    glm::vec2 velocity = glm::vec2(0.0f);
    bool alive = true;

    std::string_view inState = {};
    bool finished = false;
};
