#include <array>
#include <cmath>
#include <span>
#include <string>
#include "animations/animator_facts.hpp"
#include "actor/abilities/dash_ability_state.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/abilities/wall_climb_ability_state.hpp"
#include "actor/abilities/wall_hang_ability_state.hpp"
#include "actor/abilities/wall_slide_ability_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    constexpr float StandingStill = 0.1f;

    bool yes(const Asked &asked)
    {
        return std::get<bool>(asked);
    }

    using Row = FactRow<AnimatorFacts>;

    constexpr std::array Rows{
        Row{"alive",
            AskedKind::YesOrNo,
            "alive",
            "dead",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.observed.alive; },
            ""},
        Row{"knockback",
            AskedKind::YesOrNo,
            "knocked back",
            "not knocked back",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.decided.knockback.active; },
            "knockback"},
        Row{"swinging",
            AskedKind::YesOrNo,
            "swinging",
            "not swinging",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.decided.swing.swinging(); },
            ""},
        Row{"dashing",
            AskedKind::YesOrNo,
            "dashing",
            "not dashing",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.decided.dash.active; },
            "dash"},
        Row{"pouncing",
            AskedKind::YesOrNo,
            "pouncing",
            "not pouncing",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.decided.pounce.active; },
            "pounce"},
        Row{"onGround",
            AskedKind::YesOrNo,
            "on ground",
            "in the air",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.observed.contacts.onGround; },
            ""},
        Row{"climbing",
            AskedKind::YesOrNo,
            "climbing",
            "not climbing",
            [](const Asked &asked, const AnimatorFacts &facts)
            {
                bool climbing =
                    facts.decided.wallHang.active && facts.decided.wallClimb.velocity.y != 0.0f;
                return yes(asked) == climbing;
            },
            "wallHang"},
        Row{"onWall",
            AskedKind::YesOrNo,
            "on a wall",
            "off the wall",
            [](const Asked &asked, const AnimatorFacts &facts)
            {
                bool onWall = facts.decided.wallSlide.active || facts.decided.wallHang.active;
                return yes(asked) == onWall;
            },
            "wallSlide"},
        Row{"rising",
            AskedKind::YesOrNo,
            "rising",
            "not rising",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == (facts.observed.velocity.y < 0.0f); },
            ""},
        Row{"falling",
            AskedKind::YesOrNo,
            "falling",
            "not falling",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == (facts.observed.velocity.y > 0.0f); },
            ""},
        Row{"moving",
            AskedKind::YesOrNo,
            "moving",
            "still",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == (std::abs(facts.observed.velocity.x) > StandingStill); },
            ""},
        Row{"finished",
            AskedKind::YesOrNo,
            "clip finished",
            "clip playing",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return yes(asked) == facts.finished; },
            ""},
        Row{"inState",
            AskedKind::Name,
            "in state",
            "",
            [](const Asked &asked, const AnimatorFacts &facts)
            { return std::get<std::string>(asked) == facts.inState; },
            ""},
    };
}

std::span<const FactRow<AnimatorFacts>> animatorRows()
{
    return Rows;
}
