#include <array>
#include <span>
#include <optional>
#include <string>
#include <glm/geometric.hpp>
#include "actor/behaviors/behavior_facts.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "navigation/navigation_place.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    using Row = FactRow<ActorBehaviorContext>;

    bool threatWithin(const ActorBehaviorContext &context, std::optional<float> distance)
    {
        return context.threatFeet && distance &&
               glm::distance(context.feet, *context.threatFeet) <= *distance;
    }

    constexpr std::array Rows{
        Row{"onGround",
            AskedKind::YesOrNo,
            "on ground",
            "in the air",
            [](const Asked &asked, const ActorBehaviorContext &context)
            { return std::get<bool>(asked) == context.contacts.onGround; },
            ""},
        Row{"charging",
            AskedKind::YesOrNo,
            "charging",
            "not charging",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                bool charging = context.abilityStates && context.abilityStates->charge.active;
                return std::get<bool>(asked) == charging;
            },
            "charge"},
        Row{"threatOnMySurface",
            AskedKind::YesOrNo,
            "threat on my surface",
            "threat not on my surface",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                bool onIt =
                    context.threatFeet &&
                    onTheSameRun(context.navigationGraph, context.feet, *context.threatFeet);
                return std::get<bool>(asked) == onIt;
            },
            ""},
        Row{"cornered",
            AskedKind::YesOrNo,
            "cornered",
            "not cornered",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                bool cornered = context.threatFeet && corneredBy(
                                                          context.navigationGraph,
                                                          context.feet,
                                                          *context.threatFeet,
                                                          context.colliderSize.x);
                return std::get<bool>(asked) == cornered;
            },
            ""},
        Row{"threatClose",
            AskedKind::YesOrNo,
            "threat close",
            "threat not close",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                return std::get<bool>(asked) ==
                       threatWithin(context, context.senses ? context.senses->close : std::nullopt);
            },
            ""},
        Row{"threatInReach",
            AskedKind::YesOrNo,
            "threat in reach",
            "threat out of reach",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                return std::get<bool>(asked) ==
                       threatWithin(context, context.senses ? context.senses->reach : std::nullopt);
            },
            ""},
    };
}

std::span<const FactRow<ActorBehaviorContext>> behaviorRows()
{
    return Rows;
}

std::optional<std::string> whyNotSensed(const std::string &name, const SensesData &senses)
{
    if (name == "threatClose" && !senses.close)
        return "asks about \"threatClose\", and its senses say nothing of close";

    if (name == "threatInReach" && !senses.reach)
        return "asks about \"threatInReach\", and its senses say nothing of reach";

    return std::nullopt;
}
