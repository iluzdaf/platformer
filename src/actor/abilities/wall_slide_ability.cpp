#include <stdexcept>
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_slide_ability.hpp"
#include "input/input_intentions.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &data) : data(data)
{
    if (data.slideSpeed <= 0)
        throw std::runtime_error("A slide needs a speed above 0");
}

void WallSlideAbility::decide(
    float,
    const InputIntentions &,
    const Observed &observed,
    AbilityStates &states)
{
    states.wallSlide.emit = false;
    states.wallSlide.velocity = glm::vec2(0.0f);
    states.wallSlide.active = false;

    if (observed.contacts.onGround || !observed.contacts.grippableWall() ||
        observed.velocity.y <= 0.0f)
        return;

    states.wallSlide.velocity.y = data.slideSpeed;
    states.wallSlide.active = true;
    states.wallSlide.emit = true;
}