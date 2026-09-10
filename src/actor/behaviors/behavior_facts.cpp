#include <array>
#include <span>
#include "actor/behaviors/behavior_facts.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/decided.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    using Row = FactRow<ActorBehaviorContext>;

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
                bool charging = context.decided && context.decided->charge.active;
                return std::get<bool>(asked) == charging;
            },
            "charge"},
    };
}

std::span<const FactRow<ActorBehaviorContext>> behaviorRows()
{
    return Rows;
}
