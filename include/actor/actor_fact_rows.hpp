#pragma once

#include <optional>
#include <string>
#include <span>
#include "conditions/fact_rows.hpp"

struct ActorFacts;
struct SensesData;

std::span<const FactRow<ActorFacts>> actorRows();

std::optional<std::string> whyNotSensed(const std::string &name, const SensesData &senses);
