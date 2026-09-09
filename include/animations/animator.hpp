#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "actor/actor_animation_state.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"

struct Decided;
struct Observed;

class Animator
{
public:
    Animator();
    explicit Animator(const AnimatorData &ladder);

    void animate(float deltaTime, const Decided &decided, const Observed &observed);
    const FrameAnimation &playing() const;
    void add(ActorAnimationState state, const FrameAnimation &anim);
    ActorAnimationState state() const;
    std::vector<std::string> takeCues();
    bool finished() const;
    const AnimatorData &ladder() const;

private:
    struct Rung
    {
        ActorAnimationState from;
        bool fromAny;
        ActorAnimationState to;
        AnimationWhen when;
    };

    ActorAnimationState wanted(const Decided &decided, const Observed &observed) const;

    AnimatorData data;
    std::vector<Rung> rungs;
    ActorAnimationState currentState = ActorAnimationState::Idle;
    std::unordered_map<ActorAnimationState, FrameAnimation> animations;
};
