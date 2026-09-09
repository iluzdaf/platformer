#pragma once

#include <string>
#include <utility>
#include "animations/animator_data.hpp"
#include "actor/actor_animation_data.hpp"

inline AnimationTransitionData fromAnyTo(std::string to, AnimationWhen when)
{
    return AnimationTransitionData{std::string(), std::move(to), when};
}

inline AnimatorData everyPictureLadder()
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
        {fromAnyTo("dead", dead),
         fromAnyTo("knockback", pushed),
         fromAnyTo("attack", swinging),
         fromAnyTo("dash", dashing),
         fromAnyTo("climb", climbing),
         fromAnyTo("wallSlide", onWall),
         fromAnyTo("jump", rising),
         fromAnyTo("fall", falling),
         fromAnyTo("walk", walking),
         fromAnyTo("idle", standing)}};
}

inline AnimatorData ladderOfWhatItHas(const ActorAnimationData &animations)
{
    AnimatorData trimmed;
    for (const AnimationTransitionData &rung : everyPictureLadder().transitions)
        if (animations.clips.contains(rung.to))
            trimmed.transitions.push_back(rung);

    return trimmed;
}
