#pragma once

#include <optional>
#include <string>
#include <span>
#include "conditions/fact_rows.hpp"

struct ActorBehaviorContext;
struct SensesData;

std::span<const FactRow<ActorBehaviorContext>> behaviorRows();

std::optional<std::string> whyNotSensed(const std::string &name, const SensesData &senses);
