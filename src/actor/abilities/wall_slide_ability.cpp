#include <stdexcept>
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/wall_slide_ability.hpp"
#include "input/input_intentions.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &data) : data(data)
{
    if (data.slideSpeed <= 0)
        throw std::runtime_error("slideSpeed must be positive");
}

void WallSlideAbility::applyMovement(float, const InputIntentions &, ActorMotionState &state)
{
    state.wallSlide.emit = false;
    state.wallSlide.velocity = glm::vec2(0.0f);
    state.wallSlide.active = false;

    if (state.contacts.onGround || !state.contacts.grippableWall() || state.velocity.y <= 0.0f)
        return;

    state.wallSlide.velocity.y = data.slideSpeed;
    state.wallSlide.active = true;
    state.wallSlide.emit = true;
}