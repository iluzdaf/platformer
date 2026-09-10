#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_contact_state.hpp"
#include "conditions/facts.hpp"

class NavigationGraph;
struct Decided;

struct ActorBehaviorContext
{
    const NavigationGraph &navigationGraph;
    glm::vec2 feet;
    glm::vec2 colliderSize;
    std::optional<glm::vec2> threatFeet;

    ActorContactState contacts;
    const FactsData *facts = nullptr;
    const Decided *decided = nullptr;
};
