#pragma once

enum class ActorAnimationState
{
    Idle,
    Walk,
    Dash,
    Jump,
    Fall,
    WallSlide,
    Attack,
    Dead
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
    case ActorAnimationState::Attack:
        return "Attack";
    case ActorAnimationState::Dead:
        return "Dead";
    default:
        return "Unknown";
    }
}
