#pragma once

#include <string>
#include <utility>
#include "conditions/asked.hpp"
#include "animations/animation_ladder_data.hpp"

inline AnimationTransitionData fromAnyTo(std::string to, AnimationWhenData when)
{
    return AnimationTransitionData{std::string(), std::move(to), when};
}

inline AnimationTransitionData deadTransition()
{
    AnimationWhenData dead;
    dead["alive"] = false;
    return fromAnyTo("dead", dead);
}

inline AnimationTransitionData knockbackTransition()
{
    AnimationWhenData pushed;
    pushed["knockback"] = true;
    return fromAnyTo("knockback", pushed);
}

inline AnimationTransitionData swingTransition()
{
    AnimationWhenData swinging;
    swinging["swinging"] = true;
    return fromAnyTo("attack", swinging);
}

inline AnimationTransitionData dashTransition()
{
    AnimationWhenData dashing;
    dashing["dashing"] = true;
    return fromAnyTo("dash", dashing);
}

inline AnimationTransitionData climbTransition()
{
    AnimationWhenData climbing;
    climbing["onGround"] = false;
    climbing["climbing"] = true;
    return fromAnyTo("climb", climbing);
}

inline AnimationTransitionData wallSlideTransition()
{
    AnimationWhenData onWall;
    onWall["onGround"] = false;
    onWall["onWall"] = true;
    return fromAnyTo("wallSlide", onWall);
}

inline AnimationTransitionData jumpTransition()
{
    AnimationWhenData rising;
    rising["onGround"] = false;
    rising["rising"] = true;
    return fromAnyTo("jump", rising);
}

inline AnimationTransitionData fallTransition()
{
    AnimationWhenData falling;
    falling["onGround"] = false;
    falling["falling"] = true;
    return fromAnyTo("fall", falling);
}

inline AnimationTransitionData walkTransition()
{
    AnimationWhenData moving;
    moving["onGround"] = true;
    moving["moving"] = true;
    return fromAnyTo("walk", moving);
}

inline AnimationTransitionData idleTransition()
{
    AnimationWhenData standing;
    standing["onGround"] = true;
    return fromAnyTo("idle", standing);
}