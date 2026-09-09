#include <optional>
#include <string>
#include <utility>
#include "animations/animator_data.hpp"
#include "animations/animation_parameters.hpp"

namespace
{
    bool agrees(std::optional<bool> asked, bool actual)
    {
        return !asked || *asked == actual;
    }

    AnimationTransitionData fromAny(std::string to, AnimationWhen when)
    {
        return AnimationTransitionData{std::string(), std::move(to), when};
    }
}

bool holds(const AnimationWhen &when, const AnimationParameters &parameters)
{
    return agrees(when.alive, parameters.alive) && agrees(when.knockback, parameters.knockback) &&
           agrees(when.swinging, parameters.swinging) && agrees(when.dashing, parameters.dashing) &&
           agrees(when.onGround, parameters.onGround) &&
           agrees(when.climbing, parameters.climbing) && agrees(when.onWall, parameters.onWall) &&
           agrees(when.rising, parameters.rising) && agrees(when.falling, parameters.falling) &&
           agrees(when.moving, parameters.moving) && agrees(when.finished, parameters.finished);
}

AnimatorData theUsualLadder()
{
    AnimationWhen dead;
    dead.alive = false;
    AnimationWhen pushed;
    pushed.knockback = true;
    AnimationWhen swinging;
    swinging.swinging = true;
    AnimationWhen dashing;
    dashing.dashing = true;
    AnimationWhen climbing;
    climbing.onGround = false;
    climbing.climbing = true;
    AnimationWhen onWall;
    onWall.onGround = false;
    onWall.onWall = true;
    AnimationWhen rising;
    rising.onGround = false;
    rising.rising = true;
    AnimationWhen falling;
    falling.onGround = false;
    falling.falling = true;
    AnimationWhen walking;
    walking.onGround = true;
    walking.moving = true;
    AnimationWhen standing;
    standing.onGround = true;

    return AnimatorData{
        {fromAny("dead", dead),
         fromAny("knockback", pushed),
         fromAny("attack", swinging),
         fromAny("dash", dashing),
         fromAny("climb", climbing),
         fromAny("wallSlide", onWall),
         fromAny("jump", rising),
         fromAny("fall", falling),
         fromAny("walk", walking),
         fromAny("idle", standing)}};
}
