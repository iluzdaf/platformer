#include <stdexcept>
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_slide_ability.hpp"
#include "input/input_intentions.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &data) : data(data)
{
    if (data.slideSpeed <= 0)
        throw std::runtime_error("slideSpeed must be positive");
}

void WallSlideAbility::decide(
    float,
    const InputIntentions &,
    const Observed &observed,
    Decided &decided)
{
    decided.wallSlide.emit = false;
    decided.wallSlide.velocity = glm::vec2(0.0f);
    decided.wallSlide.active = false;

    if (observed.contacts.onGround || !observed.contacts.grippableWall() ||
        observed.velocity.y <= 0.0f)
        return;

    decided.wallSlide.velocity.y = data.slideSpeed;
    decided.wallSlide.active = true;
    decided.wallSlide.emit = true;
}