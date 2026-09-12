#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/bite_ability_state.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/hurting_from.hpp"
#include "combat/hurting.hpp"
#include "physics/aabb.hpp"

std::optional<Hurting> hurtingFrom(const AbilityStates &states, const AABB &body)
{
    const SwingAbilityState &swing = states.swing;
    if (swing.striking())
    {
        float x = swing.direction < 0.0f ? body.left() - swing.reach.x : body.right();
        AABB reach{glm::vec2(x, body.center().y - swing.reach.y * 0.5f), swing.reach};
        return Hurting{reach, swing.damage, swing.direction};
    }

    const PounceAbilityState &pounce = states.pounce;
    if (pounce.active)
        return Hurting{body, pounce.damage, pounce.direction};

    const ChargeAbilityState &charge = states.charge;
    if (charge.active)
        return Hurting{body, charge.damage, charge.direction};

    const BiteAbilityState &bite = states.bite;
    if (bite.active)
        return Hurting{body, bite.damage, 0.0f};

    return std::nullopt;
}
