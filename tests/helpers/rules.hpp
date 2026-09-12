#pragma once

#include <string>
#include "conditions/asked.hpp"
#include "animations/animation_rule_data.hpp"

inline AnimationRuleData deadRule()
{
    AnimationWhenData dead;
    dead["alive"] = false;
    return AnimationRuleData{"dead", dead};
}

inline AnimationRuleData knockbackRule()
{
    AnimationWhenData pushed;
    pushed["knockback"] = true;
    return AnimationRuleData{"knockback", pushed};
}

inline AnimationRuleData swingRule()
{
    AnimationWhenData swinging;
    swinging["swinging"] = true;
    return AnimationRuleData{"attack", swinging};
}

inline AnimationRuleData dashRule()
{
    AnimationWhenData dashing;
    dashing["dashing"] = true;
    return AnimationRuleData{"dash", dashing};
}

inline AnimationRuleData climbRule()
{
    AnimationWhenData climbing;
    climbing["onGround"] = false;
    climbing["climbing"] = true;
    return AnimationRuleData{"climb", climbing};
}

inline AnimationRuleData wallSlideRule()
{
    AnimationWhenData onWall;
    onWall["onGround"] = false;
    onWall["onWall"] = true;
    return AnimationRuleData{"wallSlide", onWall};
}

inline AnimationRuleData jumpRule()
{
    AnimationWhenData rising;
    rising["onGround"] = false;
    rising["rising"] = true;
    return AnimationRuleData{"jump", rising};
}

inline AnimationRuleData fallRule()
{
    AnimationWhenData falling;
    falling["onGround"] = false;
    falling["falling"] = true;
    return AnimationRuleData{"fall", falling};
}

inline AnimationRuleData walkRule()
{
    AnimationWhenData moving;
    moving["onGround"] = true;
    moving["moving"] = true;
    return AnimationRuleData{"walk", moving};
}

inline AnimationRuleData idleRule()
{
    AnimationWhenData standing;
    standing["onGround"] = true;
    return AnimationRuleData{"idle", standing};
}