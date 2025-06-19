#pragma once

enum class PlayerAnimationState
{
    Idle,
    Walk,
    Dash,
    Jump,
    Fall,
    WallSlide
};

static const char *toString(PlayerAnimationState state)
{
    switch (state)
    {
    case PlayerAnimationState::Idle:
        return "Idle";
    case PlayerAnimationState::Walk:
        return "Walk";
    case PlayerAnimationState::Jump:
        return "Jump";
    case PlayerAnimationState::Fall:
        return "Fall";
    case PlayerAnimationState::WallSlide:
        return "WallSlide";
    case PlayerAnimationState::Dash:
        return "Dash";
    default:
        return "Unknown";
    }
}
