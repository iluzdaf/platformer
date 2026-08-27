#pragma once

enum class ActorAnimationState
{
    Idle,
    Walk,
    Dash,
    Jump,
    Fall,
    WallSlide
};

inline const char *toString(ActorAnimationState state)
{
    switch (state)
    {
    case ActorAnimationState::Idle:
        return "Idle";
    case ActorAnimationState::Walk:
        return "Walk";
    case ActorAnimationState::Jump:
        return "Jump";
    case ActorAnimationState::Fall:
        return "Fall";
    case ActorAnimationState::WallSlide:
        return "WallSlide";
    case ActorAnimationState::Dash:
        return "Dash";
    default:
        return "Unknown";
    }
}
