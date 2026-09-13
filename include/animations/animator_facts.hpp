#pragma once

#include <span>
#include "conditions/fact_rows.hpp"

struct ActorFacts;

// Everything that can be asked about the actor, then two things about the picture:
// whether its clip has finished, and which state the actor is in.
std::span<const FactRow<ActorFacts>> animatorRows();
