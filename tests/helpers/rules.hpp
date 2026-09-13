#pragma once

#include "animations/animation_rule_data.hpp"
#include "conditions/when_data.hpp"

inline AnimationRuleData deadRule()
{
    WhenData dead;
    dead["alive"] = false;
    return AnimationRuleData{"dead", dead};
}

inline AnimationRuleData knockbackRule()
{
    WhenData pushed;
    pushed["knockback"] = true;
    return AnimationRuleData{"knockback", pushed};
}

inline AnimationRuleData swingRule()
{
    WhenData swinging;
    swinging["swinging"] = true;
    return AnimationRuleData{"attack", swinging};
}

inline AnimationRuleData dashRule()
{
    WhenData dashing;
    dashing["dashing"] = true;
    return AnimationRuleData{"dash", dashing};
}

inline AnimationRuleData climbRule()
{
    WhenData climbing;
    climbing["onGround"] = false;
    climbing["climbing"] = true;
    return AnimationRuleData{"climb", climbing};
}

inline AnimationRuleData wallSlideRule()
{
    WhenData onWall;
    onWall["onGround"] = false;
    onWall["onWall"] = true;
    return AnimationRuleData{"wallSlide", onWall};
}

inline AnimationRuleData jumpRule()
{
    WhenData rising;
    rising["onGround"] = false;
    rising["rising"] = true;
    return AnimationRuleData{"jump", rising};
}

inline AnimationRuleData fallRule()
{
    WhenData falling;
    falling["onGround"] = false;
    falling["falling"] = true;
    return AnimationRuleData{"fall", falling};
}

inline AnimationRuleData walkRule()
{
    WhenData moving;
    moving["onGround"] = true;
    moving["moving"] = true;
    return AnimationRuleData{"walk", moving};
}

inline AnimationRuleData idleRule()
{
    WhenData standing;
    standing["onGround"] = true;
    return AnimationRuleData{"idle", standing};
}