#pragma once
#include <unordered_map>
#include "game/player/player_animation_state.hpp"
#include "animations/sprite_animation.hpp"
class PlayerState;

class AnimationManager
{
public:
    void update(float deltaTime, const PlayerState &playerState);

    SpriteAnimation &getCurrentAnimation();
    void addAnimation(PlayerAnimationState state, const SpriteAnimation &anim);
    PlayerAnimationState getCurrentState() const;
    void reset();

private:
    PlayerAnimationState currentState = PlayerAnimationState::Idle;
    std::unordered_map<PlayerAnimationState, SpriteAnimation> animations;

    void transitionTo(PlayerAnimationState newState);
};