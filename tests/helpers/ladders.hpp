#pragma once

#include <string>
#include <utility>
#include "animations/animator_data.hpp"
#include "conditions/asked.hpp"
#include "actor/actor_animation_data.hpp"

inline AnimationTransitionData fromAnyTo(std::string to, AnimationWhenData when)
{
    return AnimationTransitionData{std::string(), std::move(to), when};
}

inline AnimatorData everyPictureLadder()
{
    AnimationWhenData dead;
    dead["alive"] = false;
    AnimationWhenData pushed;
    pushed["knockback"] = true;
    AnimationWhenData swinging;
    swinging["swinging"] = true;
    AnimationWhenData dashing;
    dashing["dashing"] = true;
    AnimationWhenData climbing;
    climbing["onGround"] = false;
    climbing["climbing"] = true;
    AnimationWhenData onWall;
    onWall["onGround"] = false;
    onWall["onWall"] = true;
    AnimationWhenData rising;
    rising["onGround"] = false;
    rising["rising"] = true;
    AnimationWhenData falling;
    falling["onGround"] = false;
    falling["falling"] = true;
    AnimationWhenData walking;
    walking["onGround"] = true;
    walking["moving"] = true;
    AnimationWhenData standing;
    standing["onGround"] = true;

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
