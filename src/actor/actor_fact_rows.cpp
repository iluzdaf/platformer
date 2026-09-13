#include <array>
#include <cmath>
#include <span>
#include <optional>
#include <string>
#include <glm/geometric.hpp>
#include "actor/actor_fact_rows.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "navigation/navigation_place.hpp"
#include "actor/actor_facts.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/dash_ability_state.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/abilities/wall_climb_ability_state.hpp"
#include "actor/abilities/wall_hang_ability_state.hpp"
#include "actor/abilities/wall_slide_ability_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    constexpr float StandingStill = 0.1f;

    using Row = FactRow<ActorFacts>;

    bool yes(const Asked &asked)
    {
        return std::get<bool>(asked);
    }

    const AbilityStates &statesOf(const ActorFacts &facts)
    {
        static const AbilityStates none;
        return facts.abilityStates ? *facts.abilityStates : none;
    }

    bool threatWithin(const ActorFacts &facts, std::optional<float> distance)
    {
        return facts.threatFeet && distance &&
               glm::distance(facts.feet, *facts.threatFeet) <= *distance;
    }

    constexpr std::array Rows{
        Row{"alive",
            AskedKind::YesOrNo,
            "alive",
            "dead",
            [](const Asked &asked, const ActorFacts &facts) { return yes(asked) == facts.alive; },
            ""},
        Row{"knockback",
            AskedKind::YesOrNo,
            "knocked back",
            "not knocked back",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == statesOf(facts).knockback.active; },
            "knockback"},
        Row{"swinging",
            AskedKind::YesOrNo,
            "swinging",
            "not swinging",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == statesOf(facts).swing.swinging(); },
            ""},
        Row{"dashing",
            AskedKind::YesOrNo,
            "dashing",
            "not dashing",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == statesOf(facts).dash.active; },
            "dash"},
        Row{"pouncing",
            AskedKind::YesOrNo,
            "pouncing",
            "not pouncing",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == statesOf(facts).pounce.active; },
            "pounce"},
        Row{"charging",
            AskedKind::YesOrNo,
            "charging",
            "not charging",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == statesOf(facts).charge.active; },
            "charge"},
        Row{"onGround",
            AskedKind::YesOrNo,
            "on ground",
            "in the air",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == facts.contacts.onGround; },
            ""},
        Row{"climbing",
            AskedKind::YesOrNo,
            "climbing",
            "not climbing",
            [](const Asked &asked, const ActorFacts &facts)
            {
                const AbilityStates &states = statesOf(facts);
                bool climbing = states.wallHang.active && states.wallClimb.velocity.y != 0.0f;
                return yes(asked) == climbing;
            },
            "wallHang"},
        Row{"onWall",
            AskedKind::YesOrNo,
            "on a wall",
            "off the wall",
            [](const Asked &asked, const ActorFacts &facts)
            {
                const AbilityStates &states = statesOf(facts);
                return yes(asked) == (states.wallSlide.active || states.wallHang.active);
            },
            "wallSlide"},
        Row{"rising",
            AskedKind::YesOrNo,
            "rising",
            "not rising",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == (facts.velocity.y < 0.0f); },
            ""},
        Row{"falling",
            AskedKind::YesOrNo,
            "falling",
            "not falling",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == (facts.velocity.y > 0.0f); },
            ""},
        Row{"moving",
            AskedKind::YesOrNo,
            "moving",
            "still",
            [](const Asked &asked, const ActorFacts &facts)
            { return yes(asked) == (std::abs(facts.velocity.x) > StandingStill); },
            ""},
        Row{"threatOnMySurface",
            AskedKind::YesOrNo,
            "threat on my surface",
            "threat not on my surface",
            [](const Asked &asked, const ActorFacts &facts)
            {
                bool onIt = facts.threatFeet &&
                            onTheSameRun(facts.navigationGraph, facts.feet, *facts.threatFeet);
                return yes(asked) == onIt;
            },
            ""},
        Row{"cornered",
            AskedKind::YesOrNo,
            "cornered",
            "not cornered",
            [](const Asked &asked, const ActorFacts &facts)
            {
                bool cornered =
                    facts.threatFeet &&
                    corneredBy(
                        facts.navigationGraph, facts.feet, *facts.threatFeet, facts.colliderSize.x);
                return yes(asked) == cornered;
            },
            ""},
        Row{"threatClose",
            AskedKind::YesOrNo,
            "threat close",
            "threat not close",
            [](const Asked &asked, const ActorFacts &facts)
            {
                return yes(asked) ==
                       threatWithin(facts, facts.senses ? facts.senses->close : std::nullopt);
            },
            ""},
        Row{"threatInReach",
            AskedKind::YesOrNo,
            "threat in reach",
            "threat out of reach",
            [](const Asked &asked, const ActorFacts &facts)
            {
                return yes(asked) ==
                       threatWithin(facts, facts.senses ? facts.senses->reach : std::nullopt);
            },
            ""},
    };
}

std::span<const FactRow<ActorFacts>> actorRows()
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
