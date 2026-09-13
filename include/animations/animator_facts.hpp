#pragma once

#include <span>
#include "conditions/fact_rows.hpp"

struct ActorFacts;

std::span<const FactRow<ActorFacts>> animatorRows();
