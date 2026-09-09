#include <array>
#include <span>
#include <glm/geometric.hpp>
#include "actor/behaviors/behavior_facts.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/actor_contact_state.hpp"
#include "conditions/asked.hpp"
#include "game/noise.hpp"
#include "conditions/fact_rows.hpp"
#include "navigation/navigation_place.hpp"

namespace
{
    bool yes(const Asked &asked)
    {
        return std::get<bool>(asked);
    }

    float number(const Asked &asked)
    {
        return std::get<float>(asked);
    }

    using Row = FactRow<ActorBehaviorContext>;

    constexpr std::array Rows{
        Row{"threatWithin",
            AskedKind::Number,
            "threat within",
            "",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                return context.threatFeet.has_value() &&
                       glm::distance(context.feet, *context.threatFeet) <= number(asked);
            },
            ""},
        Row{"threatBeyond",
            AskedKind::Number,
            "threat beyond",
            "",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                return !context.threatFeet.has_value() ||
                       glm::distance(context.feet, *context.threatFeet) > number(asked);
            },
            ""},
        Row{"threatOnMySurface",
            AskedKind::YesOrNo,
            "on my surface",
            "off my surface",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                bool sharing =
                    context.threatFeet.has_value() &&
                    onTheSameRun(context.navigationGraph, context.feet, *context.threatFeet);
                return yes(asked) == sharing;
            },
            ""},
        Row{"cornered",
            AskedKind::YesOrNo,
            "cornered",
            "not cornered",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                bool cornered = context.threatFeet.has_value() && corneredBy(
                                                                      context.navigationGraph,
                                                                      context.feet,
                                                                      *context.threatFeet,
                                                                      context.colliderSize.x);
                return yes(asked) == cornered;
            },
            ""},
        Row{"landingWithin",
            AskedKind::Number,
            "landing within",
            "",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                for (const Noise &noise : context.noises)
                    if (noise.kind == LandingNoise &&
                        glm::distance(context.feet, noise.at) <= number(asked))
                        return true;

                return false;
            },
            ""},
        Row{"landingOnMySurface",
            AskedKind::YesOrNo,
            "landing on my surface",
            "no landing on my surface",
            [](const Asked &asked, const ActorBehaviorContext &context)
            {
                bool heard = false;
                for (const Noise &noise : context.noises)
                    heard =
                        heard || (noise.kind == LandingNoise &&
                                  onTheSameRun(context.navigationGraph, context.feet, noise.at));

                return yes(asked) == heard;
            },
            ""},
        Row{"onGround",
            AskedKind::YesOrNo,
            "on ground",
            "in the air",
            [](const Asked &asked, const ActorBehaviorContext &context)
            { return yes(asked) == context.contacts.onGround; },
            ""},
    };
}

std::span<const FactRow<ActorBehaviorContext>> behaviorRows()
{
    return Rows;
}
