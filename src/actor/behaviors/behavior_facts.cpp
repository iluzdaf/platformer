#include <array>
#include <span>
#include "actor/behaviors/behavior_facts.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/actor_contact_state.hpp"
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
    };
}

std::span<const FactRow<ActorBehaviorContext>> behaviorRows()
{
    return Rows;
}
