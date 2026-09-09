#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "actor/actor_animation_state.hpp"
#include "animations/frame_animation.hpp"

struct Decided;
struct Observed;

class Animator
{
public:
    void animate(float deltaTime, const Decided &decided, const Observed &observed);
    const FrameAnimation &playing() const;
    void add(ActorAnimationState state, const FrameAnimation &anim);
    ActorAnimationState state() const;
    std::vector<std::string> takeCues();
    bool finished() const;

private:
    ActorAnimationState currentState = ActorAnimationState::Idle;
    std::unordered_map<ActorAnimationState, FrameAnimation> animations;
};
