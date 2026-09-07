#pragma once

#include <vector>
#include "actor/actor_contact_state.hpp"
#include "actor/hit.hpp"

struct Observed
{
    ActorContactState contacts;
    std::vector<Hit> hits;
};
