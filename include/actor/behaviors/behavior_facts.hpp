#pragma once

#include <span>
#include "conditions/fact_rows.hpp"

struct ActorBehaviorContext;

std::span<const FactRow<ActorBehaviorContext>> behaviorRows();
